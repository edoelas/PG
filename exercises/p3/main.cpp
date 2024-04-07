#include <PGUPV.h>
#include <GUI3.h>
#include <iomanip>
#include "gmlReader.h"
#include "kmlReader.h"
#include <filesystem>

using namespace PGUPV;


class MyRender : public Renderer {
public:
	MyRender() : axes(500.0f) {};
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;
	bool mouse_move(const MouseMotionEvent&) override;
private:
	// files
	City city_data;
	KMLFile neighbourhood_data;

	// OGL
	std::shared_ptr<GLMatrices> mats;
	std::unique_ptr<PGUPV::Mesh> boundary;
	std::unique_ptr<PGUPV::Mesh> exteriorMesh;
	std::unique_ptr<PGUPV::Mesh> interiorMesh;
	std::unique_ptr<PGUPV::Mesh> neighbourhoods;
	std::vector<std::unique_ptr<PGUPV::Mesh>> neighbourhoodMeshList;
	// textures
	std::vector<std::shared_ptr<Texture2D>> textures;
	std::vector<std::unique_ptr<Mesh>> textureMeshes;
	// UI
	std::shared_ptr<Label> cursorPos;
	std::shared_ptr<ListBoxWidget<>> neighbourhoodWidget;
	std::shared_ptr<RGBAColorWidget> colorWidget;
	std::shared_ptr<ListBoxWidget<>> tilesList;
	std::shared_ptr<Label> renderedTilesLbl;
	// other
	std::vector<std::string> neighbourhoodsNames;
	std::string tilesFolder;
	Axes axes;
	glm::uvec2 windowSize{ 0 };

	void buildGUI();
	void setupBuildings(std::string path);
	void setupNeighbourhoods(std::string path);
	void setupBounds();
	void setupGeoTiff(std::shared_ptr<Texture2D> texture, std::vector<glm::dvec3> corners);
	bool loadGeoTif(const std::string& filename);
	void refreshTilesList(const std::string& folder);
	glm::vec3 reduce_point(double x, double y);
	std::tuple<double, double> augment_point(glm::vec2 v);
};


glm::vec3 MyRender::reduce_point(double x, double y) {
	return glm::vec3{ static_cast<float>(x - city_data.min.x), static_cast<float>(y - city_data.min.y), 0.0f };
}

std::tuple<double, double> MyRender::augment_point(glm::vec2 v) {
	return { static_cast<double>(v.x + city_data.min.x), static_cast<double>(v.y + city_data.min.x) };
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

	// Mostramos la lista de barrios usando ListBoxWidget
	neighbourhoodWidget = std::make_shared<ListBoxWidget<>>("Barrios", neighbourhoodsNames);
	panel->addWidget(neighbourhoodWidget);

	// Creamos un widget para seleccionar el color
	colorWidget = std::make_shared<RGBAColorWidget>("Color", glm::vec4{ 0.8f, 1.f, 0.1f, 0.4f });
	panel->addWidget(colorWidget);


	// Añadimos un botón para seleccionar la carpeta de los tiles
	panel->addWidget(std::make_shared<Button>("Select tiles folder", [this]() {
		std::string folder;
		if (CommonDialogs::SelectFolder(tilesFolder, folder) == CommonDialogs::DialogResult::OK) {
			// Mostrar los ficheros de la carpeta seleccionada
			refreshTilesList(folder);
			// Guardar la carpeta seleccionada en las propiedades de la aplicación
			App::getInstance().setProperty("tilesFolder", folder);
		}
		}));


	// Cargamos la carpeta seleccionada en la ejecución anterior
	App::getInstance().getProperty("tilesFolder", tilesFolder);
	// Esta lista mostrará los ficheros de la carpeta seleccionada
	tilesList = std::make_shared<ListBoxWidget<>>("Tiles", std::vector<std::string>{});
	if (!tilesFolder.empty()) {
		refreshTilesList(tilesFolder);
	}
	// Inicialmente no hay selección
	tilesList->setEmptySelection();
	// Al seleccionar un fichero, lo cargamos
	tilesList->getValue().addListener([this](const int&) {
		if (tilesList->hasSelection())
			loadGeoTif(tilesFolder + "/" + tilesList->getSelectedElement());
		});
	panel->addWidget(tilesList);

	auto hbox = std::make_shared<HBox>();
	hbox->addChild(std::make_shared<Label>("Rendered tiles: "));
	renderedTilesLbl = std::make_shared<Label>("");
	hbox->addChild(renderedTilesLbl);
	panel->addWidget(hbox);

	App::getInstance().getWindow().showGUI(true);
}

bool MyRender::loadGeoTif(const std::string& filename)
{
	INFO("Loading " + filename);

	// El objeto Image nos permite acceder a los metadatos de un fichero GeoTiff
	Image geotif(filename);
	auto metadata = geotif.getGeoTiffMetadata();
	if (!metadata) {
		throw std::runtime_error("No metadata found in " + filename);
	}
	// Creamos una textura a partir de la imagen cargada en memoria
	auto texture = std::make_shared<Texture2D>();
	texture->loadImage(geotif);

	auto corners = metadata.value().corners();
	setupGeoTiff(texture, corners);
	return true;
}

void MyRender::setupGeoTiff(std::shared_ptr<Texture2D> texture, std::vector<glm::dvec3> corners)
{
	auto mesh = std::make_unique<Mesh>();
	std::vector<glm::vec3> vertices;
	vertices.reserve(4);
	for (const auto& corner : corners) {
		auto v = reduce_point(corner.x, corner.y);
		vertices.push_back(v);
	}
	mesh->addVertices(vertices);
	mesh->addTexCoord(0, { {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f} });
	mesh->addDrawCommand(new DrawArrays(GL_TRIANGLE_FAN, 0, 4));
	
	textureMeshes.push_back(std::move(mesh));
	textures.push_back(texture);
}

/**
 * @brief Carga en la lista los ficheros TIF de la carpeta indicada
 * @param folder
*/
void MyRender::refreshTilesList(const std::string& folder)
{
	tilesFolder = folder;
	tilesList->clear();
	std::vector<std::string> tiles;
	for (const auto& p : std::filesystem::directory_iterator(folder)) {
		if (p.path().extension() == ".tif") {
			tiles.push_back(p.path().filename().string());
		}
	}
	tilesList->setElements(tiles, false);
	tilesList->setEmptySelection();
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
	auto [mx, my] = augment_point(mousePos);
	os << std::fixed << std::setprecision(2) << mx << " " << my << "/n";
	cursorPos->setText(os.str());
	return false;
}

void MyRender::setupBuildings( std::string path) {
	city_data = readBuildings(path, true);

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

	for (const Building& building : city_data.buildings) {
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
	exteriorMesh->setColor(glm::vec4{ 1.0f, 1.0f, 1.0f, 1.0f });

	interiorMesh->addVertices(interiorVertices);
	interiorMesh->addDrawCommand(new MultiDrawArrays(GL_LINE_LOOP, interiorFirst.data(), interiorCount.data(), interiorFirst.size()));
	interiorMesh->setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });
}

void MyRender::setupBounds() {
	auto ll = reduce_point(city_data.min.x, city_data.min.y);
	auto ur = reduce_point(city_data.max.x, city_data.max.y);
	boundary = std::make_unique<PGUPV::Mesh>();
	boundary->addVertices({ ll, glm::vec2{ur.x, ll.y}, ur, glm::vec2{ll.x, ur.y} });
	boundary->setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });
	boundary->addDrawCommand(new PGUPV::DrawArrays(GL_LINE_LOOP, 0, 4));

	auto center = glm::vec3{ (ll.x + ur.x) / 2, (ll.y + ur.y) / 2, 0.0f };
	setCameraHandler(std::make_shared<XYPanZoomCamera>(1000.0f, center));
}

void MyRender::setupNeighbourhoods(std::string path) {
	neighbourhood_data = readNeighborhood(App::assetsDir() + "/data/barris-barrios.kml");

	neighbourhoods = std::make_unique<PGUPV::Mesh>();
	neighbourhoodsNames = std::vector<std::string>(neighbourhood_data.placemarks.size());
	std::vector<GLint> neighbourhoodsVFirst;
	std::vector<GLsizei> neighbourhoodsVCount;
	std::vector<glm::vec3> vertices;
	int counter = 0;

	for (size_t i = 0; i < neighbourhood_data.placemarks.size(); i++) {
		auto placemark = neighbourhood_data.placemarks[i];
		neighbourhoodsNames[i] = std::get<std::string>(placemark.attributes["nombre"]);
		neighbourhoodsVFirst.push_back(counter);
		for (const auto& point : placemark.geometry) {
			for (const auto& vertex : point.outerBoundary) {
				vertices.push_back(reduce_point(vertex.x, vertex.y));
				counter++;
			}
		}
		neighbourhoodsVCount.push_back(counter - neighbourhoodsVFirst.back());
	}

	neighbourhoods->addVertices(vertices);
	neighbourhoods->addDrawCommand(new MultiDrawArrays(GL_LINE_LOOP, neighbourhoodsVFirst.data(), neighbourhoodsVCount.data(), neighbourhoodsVFirst.size()));
	neighbourhoods->setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });

	// create a std::vector with the std::make_unique<PGUPV::Mesh> for each neighbourhood
	neighbourhoodMeshList = std::vector<std::unique_ptr<PGUPV::Mesh>>(neighbourhoodsVFirst.size());
	for (size_t i = 0; i < neighbourhoodsVFirst.size(); i++) {
		auto mesh = std::make_unique<PGUPV::Mesh>();
		mesh->addVertices(vertices);
		mesh->addDrawCommand(new DrawArrays(GL_TRIANGLE_FAN, neighbourhoodsVFirst[i], neighbourhoodsVCount[i]));
		mesh->setColor(glm::vec4{ 0.8f, 1.f, 0.1f, 0.4f });
		neighbourhoodMeshList[i] = std::move(mesh);
	}

}

void MyRender::setup() {
	glClearColor(0.6f, 0.6f, 0.9f, 1.0f);
	mats = GLMatrices::build();
	
	setupBuildings(App::assetsDir() + "/data/A.ES.SDGC.BU.46900.buildingpart.test.gml");
	setupNeighbourhoods(App::assetsDir() + "/data/barris-barrios.kml");
	setupBounds();

	buildGUI();
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT);
	auto cam = std::static_pointer_cast<XYPanZoomCamera>(getCameraHandler());

	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

	TextureReplaceProgram::use();

	for (int i = 0; i < textures.size(); i++) {
		textures[i]->bind(GL_TEXTURE0+i);
		TextureReplaceProgram::setTextureUnit(i);
		textureMeshes[i]->render();
	}

	renderedTilesLbl->setText(std::to_string(textures.size()));
	
	ConstantIllumProgram::use();

	exteriorMesh->render();
	boundary->render();
	if (cam->getWidth() < 5000) interiorMesh->render();

	// draw selected neighbourhood
	auto selected = neighbourhoodWidget->getSelected();
	if (selected >= 0) {

		glEnable(GL_STENCIL_TEST);
		glClear(GL_STENCIL_BUFFER_BIT);

		glStencilFunc(GL_ALWAYS, 0, 1);
		glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
		glColorMask(false, false, false, false);

		neighbourhoodMeshList[selected]->render();

		glStencilFunc(GL_EQUAL, 1, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
		glColorMask(true, true, true, true);

		glEnable(GL_BLEND);
		neighbourhoodMeshList[selected]->setColor(colorWidget->getColor());
		neighbourhoodMeshList[selected]->render();
		glDisable(GL_BLEND);


		glDisable(GL_STENCIL_TEST);
	}
	neighbourhoods->render();

}


void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	windowSize = glm::uvec2{ w, h };
}

int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::STENCIL_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
