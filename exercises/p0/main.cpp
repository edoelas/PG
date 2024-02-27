#include <PGUPV.h>

using namespace PGUPV;

/* 
Rellena las funciones setup y render tal y como se explica en el enunciado de la práctica.
¡Cuidado! NO uses las llamadas de OpenGL directamente (glGenVertexArrays, glBindBuffer, etc.).
Usa las clases Model y Mesh de PGUPV.
*/

class MyRender : public Renderer {
public:
	void setup(void);
	void render(void);
	void reshape(uint w, uint h);
private:
	std::shared_ptr<Model> model;
};

void MyRender::setup() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	auto mesh = std::make_shared<Mesh>();
	mesh->addVertices({
		{ -0.5f, -0.5f, 0.0f },
		{ 0.5f, -0.5f, 0.0f },
		{ 0.0f, 0.5f, 0.0f }
	});
	mesh->setColor(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
	mesh->addDrawCommand(new DrawArrays(GL_LINE_LOOP, 0, 3));

	model = std::make_shared<Model>();
	model->addMesh(mesh);

	ConstantIllumProgramMVP::use();
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT);
	model->render();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	/*
	Sistema de coordenadas canónico: [-1, 1]x[-1, 1]x[-1, 1]
	*/
	ConstantIllumProgramMVP::setMVP(glm::mat4{ 1.0f });
}

int main(int argc, char *argv[]) {
	App &myApp = App::getInstance();
	myApp.setInitWindowSize(800, 800);
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}