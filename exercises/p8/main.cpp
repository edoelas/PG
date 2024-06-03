

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

Implementando las técnicas del bump mapping y el parallax mapping.

*/

class MyRender : public Renderer {
public:
	MyRender()
		: ci(vec4(0.0, 0.0, 1.0, 1.0), vec4(1.5, 0.0, 1.0, 1.0),
		vec3(0.0, 0.0, 1.0), 4),
		lightBulb(.05f, 5, 5, vec4(1.0f, 1.0f, 0.0f, 1.0f)) {}
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;

private:
	std::shared_ptr<GLMatrices> mats;
	std::shared_ptr<UBOLightSources> lights;
	std::shared_ptr<Program> ashader;
	Axes axes;
	Rect plane;
	Rect plane2;
	Cylinder cylinder;
	std::shared_ptr<CheckBoxWidget> showAxis;
	CircularInterpolator ci;
	Sphere lightBulb;
	void buildGUI();
};

void MyRender::setup() {
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	/* Tendrás que implementar el bump mapping y el parallax mapping en este
	 * shader */
	ashader = std::make_shared<Program>();
	ashader->addAttributeLocation(Mesh::VERTICES, "position");
	ashader->addAttributeLocation(Mesh::NORMALS, "normal");
	ashader->addAttributeLocation(Mesh::TEX_COORD0, "texCoord");
	ashader->addAttributeLocation(Mesh::TANGENTS, "tangent");

	mats = GLMatrices::build();
	ashader->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
	lights = UBOLightSources::build();
	ashader->connectUniformBlock(lights, UBO_LIGHTS_BINDING_INDEX);

	LightSourceParameters lsp(glm::vec4(.2, .2, .2, 1.0),
		vec4(0.8, 0.7, 0.7, 1.0), vec4(1.0, 1.0, 1.0, 1.0),
		vec4(0.0, 0.0, 1.0, 0.0));
	lights->setLightSource(0, lsp);

	ashader->loadFiles(App::exercisesDir() + "p8/p8");
	ashader->compile();

	// Cargamos las texturas desde fichero
	auto tcolor = std::make_shared<Texture2D>();
	tcolor->loadImage(App::assetsDir() + "images/rocas-color.png");
	auto tbrillo = std::make_shared<Texture2D>();
	tbrillo->loadImage(App::assetsDir() + "images/rocas-gloss.png");
	auto tnormales = std::make_shared<Texture2D>();
	tnormales->loadImage(App::assetsDir() + "images/rocas-normales.png");
	auto talturas = std::make_shared<Texture2D>();
	talturas->loadImage(App::assetsDir() + "images/rocas-mapa-alturas.png");

	// Asociamos las texturas a un material
	auto material = std::make_shared<Material>("bump-mapping");
	material->setDiffuseTexture(tcolor);
	material->setSpecularTexture(tbrillo);
	material->setNormalMapTexture(tnormales);
	material->setHeightMapTexture(talturas);



	// Asignamos el material a todas las mallas
	plane.accept([material](Mesh &m) {
		m.setMaterial(material);
		m.setTangent(glm::vec3(1.0, 0.0, 0.0));
		});

	cylinder.accept([material](Mesh& m) {
		m.setMaterial(material);
		auto tangents = std::vector<glm::vec3>();
		for (auto i = 0; i < m.getVertices().size(); i++) {
			auto vertex = m.getVertices()[i];
			auto normal = m.getNormals()[i];
			// calculate tangent
			auto tangent = glm::normalize(glm::cross(normal, vec3(0, -1, 0)));

			tangents.push_back(tangent);
		}
		m.addTangents(tangents);
		});

	// suelo 
	auto tcolor2 = std::make_shared<Texture2D>();
	tcolor2->loadImage(App::assetsDir() + "images/beige-stonework_albedo.png");
	auto tbrillo2 = std::make_shared<Texture2D>();
	tbrillo2->loadImage(App::assetsDir() + "images/beige-stonework_roughness.png");
	auto tnormales2 = std::make_shared<Texture2D>();
	tnormales2->loadImage(App::assetsDir() + "images/beige-stonework_normal-ogl.png");
	auto talturas2 = std::make_shared<Texture2D>();
	talturas2->loadImage(App::assetsDir() + "images/beige-stonework_height.png");

	// Asociamos las texturas a un material
	auto material2 = std::make_shared<Material>("bump-mapping");
	// repeat textures
	material2->setDiffuseTexture(tcolor2);
	material2->setSpecularTexture(tbrillo2);
	material2->setNormalMapTexture(tnormales2);
	material2->setHeightMapTexture(talturas2);

	plane2.accept([material2](Mesh& m) {
		m.setMaterial(material2);
		m.setTangent(glm::vec3(1.0, 0.0, 0.0));
		});

	buildGUI();
	App::getInstance().getWindow().showGUI();

	setCameraHandler(std::make_shared<OrbitCameraHandler>(1.5f));
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 viewMatrix = getCamera().getViewMatrix();
	mats->setMatrix(GLMatrices::VIEW_MATRIX, viewMatrix);

	// Activar el shader que aplica el bump mapping
	ashader->use();

	LightSourceParameters light = lights->getLightSource(0);
	// Posición de la luz (coord. mundo)
	light.positionWorld = ci.interpolate(App::getInstance().getAppTime());
	// Posición de la luz (coord. cámara)
	light.positionEye = viewMatrix * light.positionWorld;
	lights->setLightSource(0, light);

	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->translate(GLMatrices::MODEL_MATRIX, -1, 0, 0);
	cylinder.render();
	mats->translate(GLMatrices::MODEL_MATRIX, 0.5, 0, 0);
	plane.render();
	mats->translate(GLMatrices::MODEL_MATRIX, 1.0, 0, 0);
	//mats->rotate(GLMatrices::MODEL_MATRIX, 1*3.14, vec3(0, 0, 1));
	plane.render();
	mats->translate(GLMatrices::MODEL_MATRIX, 0.5, 0, 0);
	cylinder.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	// use plane2 as floor
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	//mats->scale(GLMatrices::MODEL_MATRIX, 10, 10, 0);
	mats->rotate(GLMatrices::MODEL_MATRIX, -1.57f, vec3(1, 0, 0));
	mats->translate(GLMatrices::MODEL_MATRIX, -1, 0, -0.5);
	plane2.render();
	mats->translate(GLMatrices::MODEL_MATRIX, 1, 0, 0.0);
	plane2.render();
	mats->translate(GLMatrices::MODEL_MATRIX, 1, 0, 0.0);
	plane2.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);
	
	ConstantIllumProgram::use();

	if (showAxis->get()) {
		// Dibujamos los ejes de coordenadas
		axes.render();
	}

	// Dibujamos la posición de la luz
	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->setMatrix(GLMatrices::MODEL_MATRIX,
		glm::translate(glm::mat4(1.0f), vec3(light.positionWorld)));
	lightBulb.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
    mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
}

void MyRender::buildGUI() {
	auto panel = addPanel("Bump mapping");
	panel->setPosition(580, 10);
	panel->setSize(200, 80);

	showAxis = std::make_shared<CheckBoxWidget>("Mostrar ejes", false);
	panel->addWidget(showAxis);
	panel->addWidget(std::make_shared<CheckBoxWidget>("Parallax", false, ashader, "useParallax"));
	panel->addWidget(std::make_shared<FloatSliderWidget>("Bias", 0.01f, 0.00f, 0.1f, ashader, "parallaxBias"));
	panel->addWidget(std::make_shared<FloatSliderWidget>("Scale", 0.01f, 0.00f, 0.1f, ashader, "parallaxScale"));

	// Normals
	panel->addWidget(std::make_shared<Separator>());
	panel->addWidget(std::make_shared<Label>("Normals"));
	panel->addWidget(std::make_shared<CheckBoxWidget>("Fake normals", false, ashader, "useFakeNormals"));
	panel->addWidget(std::make_shared<CheckBoxWidget>("Show normal map", false, ashader, "showNormalMap"));
	panel->addWidget(std::make_shared<FloatSliderWidget>("Size NM", 2.3f, 0.00f, 10.0f, ashader, "size"));
	// panel->addWidget(std::make_shared<FloatSliderWidget>("Offset", 2.0f, 0.0f, 5.0f, ashader, "offset"));
	panel->addWidget(std::make_shared<FloatSliderWidget>("Scale NM", 5.0f, 0.0f, 10.0f, ashader, "scale"));

	// Material
	panel->addWidget(std::make_shared<Separator>());
	panel->addWidget(std::make_shared<Label>("Material properties"));

	panel->addWidget(std::make_shared<FloatSliderWidget>("Ambient", 1.5f, 0.0f, 10.0f, ashader, "matAmbient"));
	panel->addWidget(std::make_shared<FloatSliderWidget>("Difuse", 0.8f, 0.0f, 10.0f, ashader, "matDiffuse"));
	panel->addWidget(std::make_shared<FloatSliderWidget>("Specular", 0.6f, 0.0f, 10.0f, ashader, "matSpecular"));
	panel->addWidget(std::make_shared<FloatSliderWidget>("Shininess", 5.0f, 0.0f, 10.0f, ashader, "matShininess"));
}


int main(int argc, char *argv[]) {
	App &myApp = App::getInstance();
	myApp.setInitWindowSize(800, 600);
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
		PGUPV::MULTISAMPLE);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
