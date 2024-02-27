// Que es mejor: volver a pasar sobre la misma linea o usar mas vertices y draw_lines?
// Aspect ratio?
// Ejemplos?
#include <PGUPV.h>
#include <iostream>
#include <cmath>


using namespace PGUPV;
using namespace std;

#define PI 3.14159265358979323846
#define NVERTS 90

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


	// Cuadrado
	auto squareMesh = std::make_shared<Mesh>();
	squareMesh->addVertices({
		{ -0.5f, -0.5f, 0.0f },
		{ 0.5f, -0.5f, 0.0f },
		{ 0.5f, 0.5f, 0.0f },
		{ -0.5f, 0.5f, 0.0f},
		{ 0.0f, 0.5f, 0.0f},
		{ 0.0f, -0.5f, 0.0f},
		{ 0.5f, 0.0f, 0.0f},
		{ -0.5f, 0.0f, 0.0f}
		});

	GLubyte indices[] = {
		0,1,
		1,2,
		2,3,
		3,0,
		4,5,
		6,7
	};

	squareMesh->addIndices(indices, sizeof(indices));
	squareMesh->setColor(glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f });
	squareMesh->addDrawCommand(new DrawElements(GL_LINES, sizeof(indices), GL_UNSIGNED_BYTE, 0));
	
	// Circulo
	auto circleMesh = std::make_shared<Mesh>();
	glm::vec3 circleVertices[NVERTS];
	for (int i = 0; i < NVERTS; i++) {
		circleVertices[i][0] = 0.5f * float(cos(i * 4 * PI / 180));
		circleVertices[i][1] = 0.5f * float(sin(i * 4 * PI / 180));
		circleVertices[i][2] = 0.0f;
	}
	circleMesh->addVertices(circleVertices, NVERTS);

	circleMesh->setColor(glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });
	circleMesh->addDrawCommand(new DrawArrays(GL_LINE_LOOP, 0, NVERTS));

	model = std::make_shared<Model>();
	model->addMesh(squareMesh);
	model->addMesh(circleMesh);

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

int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.setInitWindowSize(800, 800);
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}