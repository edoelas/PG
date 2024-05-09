
#include <PGUPV.h>
#include <GUI3.h>
#include <iomanip>
#include <tuple>
#include "gmlReader.h"

using namespace PGUPV;


class MyRender : public Renderer {
public:
	MyRender() : axes(500.0f) {};
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;
	bool mouse_move(const MouseMotionEvent&) override;
private:
	void buildGUI();
	glm::vec3 reduce_point(double x, double y);
	std::tuple<double, double> augment_point(glm::vec2 v);	
	std::shared_ptr<GLMatrices> mats;
	std::unique_ptr<PGUPV::Mesh> boundary;
	std::unique_ptr<PGUPV::Mesh> exteriorMesh;
	std::unique_ptr<PGUPV::Mesh> interiorMesh;
	std::shared_ptr<Label> cursorPos;
	glm::uvec2 windowSize{ 0 };
	City city;

	Axes axes;
};


glm::vec3 MyRender::reduce_point(double x, double y) {
	return glm::vec3{ static_cast<float>(x - city.min.x), static_cast<float>(y - city.min.y), 0.0f };
}

std::tuple<double, double> MyRender::augment_point(glm::vec2 v) {
	return { static_cast<double>(v.x + city.min.x), static_cast<double>(v.y + city.min.y) };
}

/**
 * @brief Aquí se define el panel de control de la aplicación
*/
void MyRender::buildGUI() {
	// Creamos un panel nuevo donde introducir nuestros controles, llamado Configuracion
	auto panel = addPanel("Location");
	// Tamaño del panel y posición del panel
	panel->setSize(260, 100);
	panel->setPosition(0, 0);

	panel->addWidget(std::make_shared<Label>("Cursor pos: "));
	cursorPos = std::make_shared<Label>("");
	panel->addWidget(cursorPos);

	App::getInstance().getWindow().showGUI(true);
}

/**
 * @brief Este método se llama cada vez que el ratón se mueve
 * @param me información sobre el evento de movimiento del ratón
 * @return devolver true si se quiere capturar el evento
*/
bool MyRender::mouse_move(const MouseMotionEvent& me) {
	auto cam = std::static_pointer_cast<XYPanZoomCamera>(getCameraHandler());
	auto center = cam->getCenter();
	auto viewportSize = glm::vec2{ cam->getWidth(), cam->getHeight() };
	auto relPos = glm::vec2{me.x / float(windowSize.x) -0.5, -(me.y / float(windowSize.y) - 0.5) };
	auto mousePos = glm::vec2{ center.x + viewportSize.x * relPos.x, center.y + viewportSize.y * relPos.y };
	// 728136, 4373696
	std::ostringstream os;
	auto [mx, my] = augment_point(mousePos);
	os << std::fixed << std::setprecision(2) << mx << " " << my << "/n";
	cursorPos->setText(os.str());
	return false;
}



void MyRender::setup() {    
	glClearColor(0.6f, 0.6f, 0.9f, 1.0f);
	mats = GLMatrices::build();

	buildGUI();
	std::string fname = App::assetsDir() + "/data/A.ES.SDGC.BU.46900.buildingpart.gml";
	city = readBuildings(fname, true);


	exteriorMesh = std::make_unique<Mesh>();
	interiorMesh = std::make_unique<Mesh>();

	std::vector<glm::vec3> exteriorVertices;
	std::vector<GLint> exteriorFirst;
	std::vector<GLsizei> exteriorCount;
	int exteriorCounter = 0;

	std::vector<glm::vec3> interiorVertices;
	std::vector<GLint> interiorFirst;
	std::vector<GLsizei> interiorCount;
	int interiorCounter = 0;

	for (const Building& building : city.buildings) {
		for (const BuildingPart& part : building.parts) {
			// exterior
			exteriorFirst.push_back(exteriorCounter);
			for (const glm::dvec2& exterior : part.exterior) {
				exteriorVertices.push_back(reduce_point(exterior.x, exterior.y));
				exteriorCounter++;

			}
			exteriorCount.push_back(exteriorCounter - exteriorFirst.back());
			
			// interior
			for (const auto interior : part.interior) {
			interiorFirst.push_back(interiorCounter);
				for (const glm::dvec2& hole : interior) {
					interiorVertices.push_back(reduce_point(hole.x, hole.y));
					interiorCounter++;
				}
			interiorCount.push_back(interiorCounter - interiorFirst.back());
			}
		}
	}

	exteriorMesh->addVertices(exteriorVertices); 
	exteriorMesh->addDrawCommand(new MultiDrawArrays(GL_LINE_LOOP, exteriorFirst.data(), exteriorCount.data(), exteriorFirst.size()));

	interiorMesh->addVertices(interiorVertices);
	interiorMesh->addDrawCommand(new MultiDrawArrays(GL_LINE_LOOP, interiorFirst.data(), interiorCount.data(), interiorFirst.size()));


	// Limites
	auto ll = reduce_point(city.min.x, city.min.y);
	auto ur = reduce_point(city.max.x, city.max.y);
	boundary = std::make_unique<PGUPV::Mesh>();
	boundary->addVertices({ ll, glm::vec2{ur.x, ll.y}, ur, glm::vec2{ll.x, ur.y}});
	boundary->setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });
	boundary->addDrawCommand(new PGUPV::DrawArrays(GL_LINE_LOOP, 0, 4));

	auto center = glm::vec3{ (ll.x + ur.x) / 2, (ll.y + ur.y) / 2, 0.0f };
	setCameraHandler(std::make_shared<XYPanZoomCamera>(1000.0f, center));
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT);
	auto cam = std::static_pointer_cast<XYPanZoomCamera>(getCameraHandler());

	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

	ConstantUniformColorProgram::use();

	ConstantUniformColorProgram::setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });
	boundary->render();
	ConstantUniformColorProgram::setColor(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });
	exteriorMesh->render();
	if (cam->getWidth() < 5000) {
		ConstantUniformColorProgram::setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });
		interiorMesh->render();
	}

}


void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	windowSize = glm::uvec2{ w, h };
	// El manejador de cámara define una cámara perspectiva con la misma razón de aspecto que la ventana
}

int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
