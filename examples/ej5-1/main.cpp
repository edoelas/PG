

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
Este ejemplo permite cambiar el tipo de interpolación que se aplica a las
coordenadas de textura definidas en los vértices.
*/

class MyRender : public Renderer {
public:
	MyRender() : program(std::shared_ptr<Program>(new Program())){};
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;

private:
	void buildGUI();
	std::shared_ptr<GLMatrices> mats;
	std::shared_ptr<Program> program;
	Rect plane;
  Material material;
	std::vector<std::shared_ptr<Texture2D>> text;

};

void MyRender::setup() {
	glClearColor(.7f, .7f, .7f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	/* Este shader aplica la textura normalmente, pero la coordenada de textura
	definida por los vértices se puede cambiar entre los diferentes tipos de
	interpolación soportados por OpenGL. */
	program->addAttributeLocation(Mesh::VERTICES, "position");
	program->addAttributeLocation(Mesh::TEX_COORD0, "texCoord");

	mats = GLMatrices::build();
	program->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

	program->loadFiles(App::examplesDir() + "ej5-1/textureReplace2");
	program->compile();

	// Localización del uniform con la unidad de textura
	GLint texUnitLoc = program->getUniformLocation("texUnit");

	// Establecemos la unidad de textura
	program->use();
	glUniform1i(texUnitLoc, 0);

	// Cargamos la nueva textura desde un fichero
	text.push_back(std::make_shared<Texture2D>(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
	text[0]->loadImage(App::assetsDir() + "images/checker2.png");

	text.push_back(std::make_shared<Texture2D>(GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE));
	text[1]->loadImage(App::assetsDir() + "images/checker3.png");

	// Creamos un material con la textura
  material.setDiffuseTexture(text[0]);

	setCameraHandler(std::make_shared<OrbitCameraHandler>(2.0f));

	buildGUI();

	App::getInstance().getWindow().showGUI(true);
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());
  material.use();
	plane.render();

	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	uint m = MIN(w, h);
	glViewport(0, 0, m, m);

  // Queremos un volumen de cámara fijo
	mats->setMatrix(GLMatrices::PROJ_MATRIX,
		glm::perspective(glm::radians(60.0f), 1.0f, .1f, 100.0f));
}

void MyRender::buildGUI() {
	auto panel = addPanel("Interpolación");
	panel->setPosition(490, 10);
	panel->setSize(290, 200);

	auto lb = std::make_shared<ListBoxWidget<>>("Textura", std::vector<std::string> {"B/N", "Color"});
	lb->getValue().addListener([this](const int &i) {material.setDiffuseTexture(text[i]); });
	panel->addWidget(lb);

	auto it = std::make_shared<ListBoxWidget<>>("Interpolación",
		std::vector<std::string> {"Smooth", "Flat", "No perspective"}, 0,
		program, "which");
	panel->addWidget(it);

	auto pv = std::make_shared<ListBoxWidget<>>("Provok. vtx",
		std::vector<std::string> {"GL_LAST_VERTEX_CONVENTION", "GL_FIRST_VERTEX_CONVENTION"});
	pv->getValue().addListener([](const int &i) {
		glProvokingVertex(i == 0 ? GL_LAST_VERTEX_CONVENTION : GL_FIRST_VERTEX_CONVENTION);
	});
	panel->addWidget(pv);
	
}

int main(int argc, char *argv[]) {
	App &myApp = App::getInstance();
	myApp.setInitWindowSize(800, 600);
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
		PGUPV::MULTISAMPLE);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
