
#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"
#include "GUI3.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

Implementación básica del algoritmo de shadow mapping

*/

// Tamaños del shadow map seleccionables por el usuario
const unsigned int sizes[] = { 64, 128, 512, 1024, 2048 };
#define INITIAL_DEPTH_TEXTURE_SIZE_INDEX 3

// Profundidad del frustum del shadow map
#define FRUSTUM_DEPTH 7.5f

class MyRender : public Renderer {
public:
	MyRender()
		: cone(0.5f, 0.00001f, 1.0f, 10, 40), box(0.2f, 1.0f, 0.5f),
		suelo(4.f, 4.f, vec4(0.0f), 20, 20),
		currentTextureSize(sizes[INITIAL_DEPTH_TEXTURE_SIZE_INDEX]),
		luz1(.02f, 5, 5, vec4(1.0f, 1.0f, 1.0f, 1.0f)),
		luz2(.02f, 5, 5, vec4(1.0f, 1.0f, 1.0f, 1.0f)),
		zbuffer1(2, 2),
		zbuffer2(2, 2),
		ci(vec4(0.0, 2.0, 0.0, 1.0), vec4(3.0, 2.0, 0.0, 1.0),
			vec3(0.0, 1.0, 0.0), 3.6f) {};
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;
	void update(uint64_t ms) override;

private:
	// Uniform Buffer Objects
	std::shared_ptr<GLMatrices> mats, shadowMats1, shadowMats2;
	std::shared_ptr<UBOLightSources> luces;

	// Objetos de la escena
	std::shared_ptr<Scene> teapot;
	Cylinder cone;
	Box box;
	Rect suelo;

	// Framebuffer object y textura para generar el shadow map
	FBO fbo1, fbo2;
	std::shared_ptr<Texture2D> depthTexture1, depthTexture2;
	uint currentTextureSize;
	// Programas
	Program gshader, shadowShader;
	std::shared_ptr<Program> zshader;
	// Esfera usada para mostrar la posición de la fuente
	Sphere luz1, luz2;
	// Cuadrilátero para dibujar el shadow map en pantalla
	Rect zbuffer1, zbuffer2;
	// ¿Mostrar el shadow map en pantalla?
	std::shared_ptr<CheckBoxWidget> showDepthMap;
	// Posición de la fuente en el espacio del mundo
	vec4 lightPosition1, lightPosition2;
	// Interpolador para mover la fuente en círculos
	CircularInterpolator ci;
	// Localización del uniform con la matriz de sombra
	GLint shadowMatrixLoc1, shadowMatrixLoc2;
	// Tamaño actual de la ventana
	uint windowWidth, windowHeight;

	// Factor de escala para el shadow map
	std::shared_ptr<FloatSliderWidget> factorUI, unitsUI;

	void prepareFBO(uint width, uint height);
	void drawScene(std::shared_ptr<GLMatrices> mats, bool drawFloor = true);
	void buildGUI();

};

void MyRender::update(uint64_t) {
	// Cálculo de la posición de la luz
	lightPosition1 = ci.interpolate(App::getInstance().getAppTime()/5);
	lightPosition2 = vec4(-lightPosition1.z, lightPosition1.y, lightPosition1.x, 1.0);
}

void MyRender::prepareFBO(uint width, uint height) {
	// Esta textura contendrá el shadow map 1
	depthTexture1 = std::shared_ptr<Texture2D>(
		new Texture2D(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
	depthTexture1->allocate(width, height, GL_DEPTH_COMPONENT32);
	// Función de comparación
	depthTexture1->setCompareFunc(GL_LEQUAL);
	// Vinculamos la textura al punto de vinculación del depth buffer del FBO
	fbo1.attach(GL_DEPTH_ATTACHMENT, depthTexture1);
	// Comprobamos que el FBO esté completo
	if (!fbo1.isComplete())
		ERRT("FBO1 incompleto");

	// Esta textura contendrá el shadow map 2
	depthTexture2 = std::shared_ptr<Texture2D>(
		new Texture2D(GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
	depthTexture2->allocate(width, height, GL_DEPTH_COMPONENT32);
	// Función de comparación
	depthTexture2->setCompareFunc(GL_LEQUAL);
	// Vinculamos la textura al punto de vinculación del depth buffer del FBO
	fbo2.attach(GL_DEPTH_ATTACHMENT, depthTexture2);
	// Comprobamos que el FBO esté completo
	if (!fbo2.isComplete())
		ERRT("FBO2 incompleto");
}

void MyRender::setup() {
	glClearColor(.7f, .7f, .7f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	prepareFBO(sizes[INITIAL_DEPTH_TEXTURE_SIZE_INDEX], sizes[INITIAL_DEPTH_TEXTURE_SIZE_INDEX]);

	// Posición de la luz en el sistema de coordenadas del mundo
	update(0);

	/* Preparamos los materiales que usaremos en nuestra escena */
	auto plastico_rojo = PGUPV::getMaterial(PredefinedMaterial::RED_PLASTIC);
	auto plastico_verde = getMaterial(PredefinedMaterial::GREEN_PLASTIC);
	auto goma_azul = PGUPV::getMaterial(PredefinedMaterial::CYAN_RUBBER);
	auto perla = PGUPV::getMaterial(PredefinedMaterial::PEARL);

	// Asociamos los materiales con los modelos
	// Estos son modelos básicos
	cone.accept([plastico_rojo](Mesh& m) { m.setMaterial(plastico_rojo); });
	box.accept([goma_azul](Mesh& m) {m.setMaterial(goma_azul);  });
	suelo.accept([perla](Mesh& m) {m.setMaterial(perla); });

	// Cuando cargamos un modelo desde un fichero 3D, el resultado es una escena (una
	// jerarquía de nodos)
	teapot = FileLoader::load(App::assetsDir() + "models/teapot.3ds");
	// La función processMeshes ejecuta la lambda indicada sobre cada malla de la escena
	teapot->processMeshes([plastico_verde](Mesh& m) {m.setMaterial(plastico_verde); });

	// Este shader se encarga de dibujar la escena final, teniendo en cuenta el
	// shadow map calculado previamente.

	// Definimos la posición de los atributos
	gshader.addAttributeLocation(Mesh::VERTICES, "position");
	gshader.addAttributeLocation(Mesh::NORMALS, "normal");

	// mats contendrá las matrices correspondientes a la cámara
	mats = GLMatrices::build();
	gshader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

	// UBO con la definición de las fuentes
	luces = UBOLightSources::build();
	gshader.connectUniformBlock(luces, UBO_LIGHTS_BINDING_INDEX);

	gshader.replaceString("$" + UBOMaterial::blockName, UBOMaterial::definition);

	gshader.loadFiles(App::exercisesDir() + "p9/p9");
	gshader.compile();
	gshader.use();

	// Localización del uniform con la matriz de sombra
	shadowMatrixLoc1 = gshader.getUniformLocation("shadowMatrix1");
	GLint textureUnit1 = gshader.getUniformLocation("depthTexture1");
	glUniform1i(textureUnit1, 3);
	shadowMatrixLoc2 = gshader.getUniformLocation("shadowMatrix2");
	GLint textureUnit2 = gshader.getUniformLocation("depthTexture2");
	glUniform1i(textureUnit2, 4);

	// Definición de los parámetros que no cambian de la fuente
	LightSourceParameters lp1;
	lp1.ambient = vec4(.3, .3, .3, 1.0);
	lp1.diffuse = vec4(0.5, 0.5, 0.5, 1.0);
	lp1.specular = vec4(0.8, 0.8, 0.8, 1.0);
	luces->setLightSource(0, lp1);

	LightSourceParameters lp2;
	lp2.ambient = vec4(.3, .3, .3, 1.0);
	lp2.diffuse = vec4(0.5, 0.5, 0.5, 1.0);
	lp2.specular = vec4(0.8, 0.8, 0.8, 1.0);
	luces->setLightSource(1, lp2);

	// Este shader se encargará de calcular el shadow map

	// Esta es la shadow matrix, que usaremos para construir el shadow map desde
	// el punto de vista de la fuente
	shadowMats1 = GLMatrices::build();
	// Este es el frustum del shador map. Debería cubrir toda la escena, idealmente 
	// usando el menor volumen posible. Aquí lo definimos estáticamente.
	shadowMats1->setMatrix(
		GLMatrices::PROJ_MATRIX,
		glm::frustum(-0.7f, .7f, -.7f, .7f, 1.3f, FRUSTUM_DEPTH));
	shadowShader.connectUniformBlock(shadowMats1, UBO_GL_MATRICES_BINDING_INDEX);

	shadowMats2 = GLMatrices::build();
	// Este es el frustum del shador map. Debería cubrir toda la escena, idealmente 
	// usando el menor volumen posible. Aquí lo definimos estáticamente.
	shadowMats2->setMatrix(
		GLMatrices::PROJ_MATRIX,
		glm::frustum(-0.7f, .7f, -.7f, .7f, 1.3f, FRUSTUM_DEPTH));
	shadowShader.connectUniformBlock(shadowMats2, UBO_GL_MATRICES_BINDING_INDEX);

	shadowShader.addAttributeLocation(Mesh::VERTICES, "position");
	shadowShader.loadFiles(App::exercisesDir() + "p9/shadowMap");
	shadowShader.compile();

	// Este shader se usa para mostrar el shadow map en la esquina inferior
	// izquierda de la ventana
	zshader = std::make_shared<Program>();
	zshader->addAttributeLocation(Mesh::VERTICES, "position");
	zshader->addAttributeLocation(Mesh::TEX_COORD0, "texCoord");
	zshader->loadFiles(App::exercisesDir() + "p9/zbuffershow");
	zshader->compile();

	GLint textureLoc = zshader->getUniformLocation("texUnit");
	zshader->use();
	glUniform1i(textureLoc, 3);

	setCameraHandler(std::make_shared<OrbitCameraHandler>());

	buildGUI();
}

// Dibuja los objetos de la escena, y si drawFloor es true, el suelo también
void MyRender::drawScene(std::shared_ptr<GLMatrices> mats, bool drawFloor) {
	// Cono rojo
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->translate(GLMatrices::MODEL_MATRIX, 1.0, 0.0, -0.5);
	cone.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	// Tetera
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->translate(GLMatrices::MODEL_MATRIX, 0.0f, -0.3f, 0.0f);
	mats->scale(GLMatrices::MODEL_MATRIX, vec3(1.5f / teapot->maxDimension()));
	teapot->render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	// Caja azul
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->translate(GLMatrices::MODEL_MATRIX, -0.75f, 0.0f, 0.4f);
	mats->rotate(GLMatrices::MODEL_MATRIX, glm::radians(150.0f), 0.0, 1.0, 0.0);
	box.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	// Suelo
	if (drawFloor) {
		mats->pushMatrix(GLMatrices::MODEL_MATRIX);
		mats->translate(GLMatrices::MODEL_MATRIX, 0.0, -0.5001f, 0.0);
		mats->rotate(GLMatrices::MODEL_MATRIX, glm::radians(-90.0f), 1.0, 0.0, 0.0);
		suelo.render();
		mats->popMatrix(GLMatrices::MODEL_MATRIX);
	}
}

// Matriz de escalado y desplazamiento para el cálculo de la matriz de sombra
static const glm::mat4 scaleBiasMatrix(0.5, 0.0, 0.0, 0.0, 0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 0.5, 0.0, 0.5, 0.5, 0.5, 1.0);

void MyRender::render() {
	glPolygonOffset(factorUI->get(), unitsUI->get());


	// Creando el shadow map, desde el punto de vista de la fuente 1
	fbo1.bind(GL_DRAW_FRAMEBUFFER);
	glViewport(0, 0, currentTextureSize, currentTextureSize);
	glClear(GL_DEPTH_BUFFER_BIT);
	// Completamos la matriz de sombra usando la posición actual de la luz
	shadowMats1->setMatrix(
		GLMatrices::VIEW_MATRIX,
		glm::lookAt(vec3(lightPosition1), vec3(0.0), vec3(0.0, 1.0, 0.0)));
	shadowShader.use();
	glDrawBuffer(GL_NONE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	drawScene(shadowMats1, false);
	glDisable(GL_POLYGON_OFFSET_FILL);
	fbo1.unbind(GL_DRAW_FRAMEBUFFER);


	// Creando el shadow map, desde el punto de vista de la fuente 2
	fbo2.bind(GL_DRAW_FRAMEBUFFER);
	glViewport(0, 0, currentTextureSize, currentTextureSize);
	glClear(GL_DEPTH_BUFFER_BIT);
	// Completamos la matriz de sombra usando la posición actual de la luz
	shadowMats2->setMatrix(
		GLMatrices::VIEW_MATRIX,
		glm::lookAt(vec3(lightPosition2), vec3(0.0), vec3(0.0, 1.0, 0.0)));
	shadowShader.use();
	glDrawBuffer(GL_NONE);
	glEnable(GL_POLYGON_OFFSET_FILL);
	drawScene(shadowMats2, false);
	glDisable(GL_POLYGON_OFFSET_FILL);
	fbo2.unbind(GL_DRAW_FRAMEBUFFER);


	// Dibujamos la escena normalmente
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, windowWidth, windowHeight);

	glDrawBuffer(GL_BACK);

	mat4 viewMatrix = getCamera().getViewMatrix();
	mats->setMatrix(GLMatrices::VIEW_MATRIX, viewMatrix);

	// Actualizamos la posición de la luz en el UBO
	LightSourceParameters lp1 = luces->getLightSource(0);
	lp1.positionWorld = lightPosition1;
	lp1.positionEye = viewMatrix * lightPosition1;
	luces->setLightSource(0, lp1);
	// Calculando la matriz de sombra
	glm::mat4 shadowMatrix1 =
		scaleBiasMatrix * shadowMats1->getMatrix(GLMatrices::MODELVIEWPROJ_MATRIX);

	LightSourceParameters lp2 = luces->getLightSource(1);
	lp2.positionWorld = lightPosition2;
	lp2.positionEye = viewMatrix * lightPosition2;
	luces->setLightSource(1, lp2);

	glm::mat4 shadowMatrix2 =
		scaleBiasMatrix * shadowMats2->getMatrix(GLMatrices::MODELVIEWPROJ_MATRIX);

	// Finalmente, dibujamos la escena usando la textura de profundidad calculada
	// en el paso anterior
	gshader.use();
	glUniformMatrix4fv(shadowMatrixLoc1, 1, GL_FALSE, &shadowMatrix1[0][0]);
	depthTexture1->bind(GL_TEXTURE3);
	depthTexture1->setCompareMode(GL_COMPARE_REF_TO_TEXTURE);
	glUniformMatrix4fv(shadowMatrixLoc2, 1, GL_FALSE, &shadowMatrix2[0][0]);
	depthTexture2->bind(GL_TEXTURE4);
	depthTexture2->setCompareMode(GL_COMPARE_REF_TO_TEXTURE);

	drawScene(mats);
	
	depthTexture2->setCompareMode(GL_NONE);
	depthTexture1->setCompareMode(GL_NONE);


	// A partir de aquí se dibuja la fuente de luz y el shadow map
	// Dibujamos la fuente (sin iluminación)
	ConstantIllumProgram::use();
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->translate(GLMatrices::MODEL_MATRIX, vec3(lightPosition1));
	luz1.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->translate(GLMatrices::MODEL_MATRIX, vec3(lightPosition2));
	luz2.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	// Dibujamos el shadow map
	if (showDepthMap->get()) {
		glViewport(0, 0, 400, 400);
		glDisable(GL_DEPTH_TEST);
		zshader->use();
		zbuffer1.render();
		//zbuffer2.render();
		glEnable(GL_DEPTH_TEST);
	}

	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	if (h == 0)
		h = 1;
	float ar = (float)w / h;
	mats->setMatrix(GLMatrices::PROJ_MATRIX,
		glm::perspective(glm::radians(60.0f), ar, 0.3f, 20.0f));
	windowWidth = w;
	windowHeight = h;
}

void MyRender::buildGUI() {
	auto panel = addPanel("Shadow mapping");
	panel->setPosition(580, 10);
	showDepthMap = std::make_shared<CheckBoxWidget>("Mostrar shadowmap", true);
	panel->addWidget(showDepthMap);

	panel->addWidget(std::make_shared<CheckBoxWidget>("Grises", false, zshader, "grayScale"));



	std::vector<std::string> sizesNames;
	for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++)
		sizesNames.push_back(std::to_string(sizes[i]));

	auto ts = std::make_shared<ListBoxWidget<>>("Tamaño de textura", sizesNames, INITIAL_DEPTH_TEXTURE_SIZE_INDEX);
	ts->getValue().addListener([this](const GLint& i) {
		prepareFBO(sizes[i], sizes[i]);
		currentTextureSize = sizes[i];
		switch (sizes[i]) {
			case 64:
				factorUI->set(3.6);
				unitsUI->set(1.0);
				break;
			case 128:
				factorUI->set(3.2);
				unitsUI->set(1.0);
				break;
			case 512:
				factorUI->set(2.0);
				unitsUI->set(1.0);
				break;
			case 1024:
				factorUI->set(1.4);
				unitsUI->set(1.0);
				break;
			case 2048:
				factorUI->set(1.4);
				unitsUI->set(1.0);
				break;	
		}
	});
	panel->addWidget(ts);

	// Añadimos un selector de units y factor
	panel->addWidget(std::make_shared<Separator>());
	panel->addWidget(std::make_shared<Label>("glPolygonOffset"));
	factorUI = std::make_shared<FloatSliderWidget>("Factor", 1.4f, 0.0, 10);
	panel->addWidget(factorUI);
	unitsUI = std::make_shared<FloatSliderWidget>("Units", 1.0f, 0.0, 100);
	panel->addWidget(unitsUI);

}

int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.setInitWindowSize(800, 600);
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
		PGUPV::MULTISAMPLE);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}