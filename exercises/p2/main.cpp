#include <PGUPV.h>
#include <GUI3.h>
#include <iomanip>
#include "gmlReader.h"
#include "kmlReader.h"

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
	void setupBuildings(std::string path);
	void setupNeighbourhoods(std::string path);
	void setupBounds();

	// files
	City city_file;
	KMLFile neighbourhood_file;
	// OGL
	std::shared_ptr<GLMatrices> mats;
	std::unique_ptr<PGUPV::Mesh> boundary;
	std::unique_ptr<PGUPV::Mesh> exteriorMesh;
	std::unique_ptr<PGUPV::Mesh> interiorMesh;
	std::unique_ptr<PGUPV::Mesh> neighbourhoods;
	std::vector<std::string> neighbourhoodsNames;
	// UI
	std::shared_ptr<Label> cursorPos;
	std::shared_ptr<ListBoxWidget> neighbourhoodWidget;
	glm::uvec2 windowSize{ 0 };

	Axes axes;
};


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

	// Mostramos la lista de barrios usando ListBoxWidget

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
	auto relPos = glm::vec2{ me.x / float(windowSize.x) - 0.5, -(me.y / float(windowSize.y) - 0.5) };
	auto mousePos = glm::vec2{ center.x + viewportSize.x * relPos.x, center.y + viewportSize.y * relPos.y };
	// 728136, 4373696
	std::ostringstream os;
	os << std::fixed << std::setprecision(2) << mousePos.x << " " << mousePos.y << "/n";
	cursorPos->setText(os.str());
	return false;
}

void MyRender::setupBuildings( std::string path) {
	city_file = readBuildings(path, true);

	exteriorMesh = std::make_unique<Mesh>();
	std::vector<glm::vec3> exteriorVertices;
	std::vector<GLint> exteriorFirst;
	std::vector<GLsizei> exteriorCount;
	int exteriorCounter = 0;

	interiorMesh = std::make_unique<Mesh>();
	std::vector<glm::vec3> interiorVertices;
	std::vector<GLint> interiorFirst;
	std::vector<GLsizei> interiorCount;
	int interiorCounter = 0;

	for (const Building& building : city_file.buildings) {
		for (const BuildingPart& part : building.parts) {
			// exterior
			exteriorFirst.push_back(exteriorCounter);
			for (const glm::dvec2& exterior : part.exterior) {
				float x = static_cast<float>(exterior.x);
				float y = static_cast<float>(exterior.y);
				exteriorVertices.push_back(glm::vec3(x, y, 0.0f));
				exteriorCounter++;

			}
			exteriorCount.push_back(exteriorCounter - exteriorFirst.back());

			// interior
			for (const auto interior : part.interior) {
				interiorFirst.push_back(interiorCounter);
				for (const glm::dvec2& hole : interior) {
					float x = static_cast<float>(hole.x);
					float y = static_cast<float>(hole.y);
					interiorVertices.push_back(glm::vec3(x, y, 0.0f));
					interiorCounter++;
				}
				interiorCount.push_back(interiorCounter - interiorFirst.back());
			}
		}
	}

	exteriorMesh->addVertices(exteriorVertices);
	exteriorMesh->addDrawCommand(new MultiDrawArrays(GL_LINE_LOOP, exteriorFirst.data(), exteriorCount.data(), exteriorFirst.size()));
	exteriorMesh->setColor(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });

	interiorMesh->addVertices(interiorVertices);
	interiorMesh->addDrawCommand(new MultiDrawArrays(GL_LINE_LOOP, interiorFirst.data(), interiorCount.data(), interiorFirst.size()));
	interiorMesh->setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });
}

void MyRender::setupBounds() {
	boundary = std::make_unique<PGUPV::Mesh>();
	boundary->addVertices({ city_file.min, glm::vec2{city_file.max.x, city_file.min.y}, city_file.max, glm::vec2{city_file.min.x, city_file.max.y} });
	boundary->setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });
	boundary->addDrawCommand(new PGUPV::DrawArrays(GL_LINE_LOOP, 0, 4));
}

void MyRender::setupNeighbourhoods(std::string path) {
	neighbourhood_file = readNeighborhood(App::assetsDir() + "/data/barris-barrios.kml");

	neighbourhoods = std::make_unique<PGUPV::Mesh>();
	std::vector<glm::vec3> vertices;
	std::vector<GLint> first;
	std::vector<GLsizei> count;
	int counter = 0;

	for (const auto& polygon : neighbourhood_file.placemarks) {
		first.push_back(counter);
		for (const auto& point : polygon.geometry) {
			for (const auto& vertex : point.outerBoundary) {
				vertices.push_back(glm::vec3{ vertex.x, vertex.y, 0.0f });
				counter++;
			}
		}
		count.push_back(counter - first.back());
	}

	neighbourhoods->addVertices(vertices);
	neighbourhoods->addDrawCommand(new MultiDrawArrays(GL_LINE_LOOP, first.data(), count.data(), first.size()));
	neighbourhoods->setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });

	neighbourhoodsNames = std::vector<std::string>(neighbourhood_file.placemarks.size());
	for (size_t i = 0; i < neighbourhood_file.placemarks.size(); i++) {
		neighbourhoodsNames[i] = std::get<std::string>(neighbourhood_file.placemarks[i].attributes["nombre"]);
	}
	
}

void MyRender::setup() {
	glClearColor(0.6f, 0.6f, 0.9f, 1.0f);
	mats = GLMatrices::build();
	buildGUI();
	
	setupBuildings(App::assetsDir() + "/data/A.ES.SDGC.BU.46900.buildingpart.test.gml");
	setupNeighbourhoods(App::assetsDir() + "/data/barris-barrios.kml");
	setupBounds();


	auto center = glm::vec3{ (city_file.min.x + city_file.max.x) / 2, (city_file.min.y + city_file.max.y) / 2, 0.0f };
	setCameraHandler(std::make_shared<XYPanZoomCamera>(1000.0f, center));
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT);
	auto cam = std::static_pointer_cast<XYPanZoomCamera>(getCameraHandler());

	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

	ConstantIllumProgram::use();
	
	boundary->render();
	exteriorMesh->render();
	neighbourhoods->render();
	if (cam->getWidth() < 5000) interiorMesh->render();
}


void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	windowSize = glm::uvec2{ w, h };
}

int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
