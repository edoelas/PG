

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

Herramienta para mostrar las normales de cara y de vértice de los modelos PGUPV.

*/

class MyRender : public Renderer {
public:
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;

private:
	std::shared_ptr<GLMatrices> mats;
	Axes axes;
	Program ashader;
	std::shared_ptr<Program> nshader;
	GLint lightPosLoc;
	vec4 lightPosWCS;
	GLint vertNColorLoc, faceNColorLoc, tangentColorLoc;
	vec4 vertNColor, faceNColor, tangentColor;
	std::vector<std::shared_ptr<Model>> objetos;
	std::shared_ptr<IntSliderWidget> showObjectIdx;

	void buildGUI();
};

void MyRender::setup() {
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Inicialización de variables de estado
	// Posición de la luz en el S.C. del mundo
	lightPosWCS = vec4(2, 2, 2, 1);
	// Color de las normales de vértice
	vertNColor = vec4(0.0, 0.0, 1.0, 1.0);
	// Color de las normales de cara
	faceNColor = vec4(0.0, 1.0, 0.0, 1.0);
	// Color de las tangentes
	tangentColor = vec4(0.0, 1.0, 1.0, 1.0);

	objetos.push_back(std::make_shared<Box>(0.5f, 0.5f, 0.5f));
	objetos.push_back(std::make_shared<Torus>(12, 10, .3f, .8f));
	objetos.push_back(std::make_shared<Cylinder>());
	objetos.push_back(std::make_shared<Cylinder>(0.6f, 0.001f));
	objetos.push_back(std::make_shared<TrefoilKnot>(10, 20, 1.1f));

	/* Este shader se encarga de calcular la iluminación (sólo componente
	 * difusa)*/
	ashader.addAttributeLocation(Mesh::VERTICES, "position");
	ashader.addAttributeLocation(Mesh::NORMALS, "normal");

	mats = GLMatrices::build();
	ashader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

	ashader.loadFiles(App::assetsDir() + "shaders/simplyDiffuse");
	ashader.compile();

	// Posiciones de las variables uniform
	lightPosLoc = ashader.getUniformLocation("lightpos");
	GLint diffuseLoc = ashader.getUniformLocation("diffuseColor");

	ashader.use();
	// Color del objeto
	glUniform4f(diffuseLoc, 0.8f, 0.3f, 0.3f, 1.0f);

	/* Este shader se encarga de dibujar las normales */
	nshader = std::make_shared<Program>();
	nshader->addAttributeLocation(Mesh::VERTICES, "position");
	nshader->addAttributeLocation(Mesh::NORMALS, "normal");
	nshader->addAttributeLocation(Mesh::TANGENTS, "tangent");

	nshader->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

	nshader->loadFiles(App::examplesDir() + "ej6-5/ej6-5");
	nshader->compile();

	// Posiciones de las variables uniform
	vertNColorLoc = nshader->getUniformLocation("vertNColor");
	faceNColorLoc = nshader->getUniformLocation("faceNColor");
	tangentColorLoc = nshader->getUniformLocation("tangentColor");

	nshader->use();
	glUniform4fv(faceNColorLoc, 1, &faceNColor.r);
	glUniform4fv(vertNColorLoc, 1, &vertNColor.r);
	glUniform4fv(tangentColorLoc, 1, &tangentColor.r);

	setCameraHandler(std::make_shared<OrbitCameraHandler>(2.0f));

#ifndef __APPLE__
	glLineWidth(2.0);
#endif

	buildGUI();
	App::getInstance().getWindow().showGUI(true);
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

	ashader.use();
	// Posición de la fuente en el S.C. de la cámara
	vec4 lp = getCamera().getViewMatrix() * lightPosWCS;
	glUniform3f(lightPosLoc, lp.x, lp.y, lp.z);

	// Dibujando el modelo
	objetos[showObjectIdx->get()]->render();

	// Cambiamos el shader para dibujar las normales
	nshader->use();

	// Volvemos a dibujar el modelo
	objetos[showObjectIdx->get()]->render();

#ifndef __APPLE__
	glLineWidth(1.0);
#endif

	// Un shader sencillo para dibujar el color definido por cada vértice
	ConstantIllumProgram::use();

	// Dibujamos los ejes de coordenadas
	axes.render();

	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	uint m = MIN(w, h);
	glViewport(0, 0, m, m);
	mats->setMatrix(GLMatrices::PROJ_MATRIX,
		glm::perspective(glm::radians(60.0f), 1.0f, .1f, 100.0f));
}

// Añadimos un widget para controlar las luces de la escena
void MyRender::buildGUI() {
	auto panel = addPanel("Visor");;
	panel->setPosition(560, 20);
	panel->setSize(215, 200);

	showObjectIdx = std::make_shared<IntSliderWidget>("Objeto", 0, 0, static_cast<int>(objetos.size() - 1));
	panel->addWidget(showObjectIdx);

	panel->addWidget(std::make_shared<Separator>());
	panel->addWidget(std::make_shared<Label>("Mostrar normales a"));

	panel->addWidget(std::make_shared<CheckBoxWidget>("Cara", true, nshader, "showFaceNormal"));
	panel->addWidget(std::make_shared<CheckBoxWidget>("Vértice", false, nshader, "showVertexNormal"));

	panel->addWidget(std::make_shared<Separator>());

	panel->addWidget(std::make_shared<CheckBoxWidget>("Mostrar tangentes", false, nshader, "showTangent"));

	auto fc = std::make_shared<CheckBoxWidget>("Face culling", false);
	fc->getValue().addListener([](const bool& checked) {
		if (checked) {
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
		}
		else
			glDisable(GL_CULL_FACE);
		});
	panel->addWidget(fc);

	panel->addWidget(std::make_shared<FloatSliderWidget>("Long. vec", 0.06f, 0.01f, 0.5f, nshader, "normalLength"));
}

int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.setInitWindowSize(800, 600);
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
		PGUPV::MULTISAMPLE);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
