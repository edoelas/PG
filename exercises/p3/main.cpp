
#include <PGUPV.h>
#include <GUI3.h>
#include <iomanip>
#include <filesystem>

using namespace PGUPV;

/**
 * @brief Ejemplo de cómo mostrar los ficheros de un directorio en un ListBoxWidget, y cargarlos
 * bajo demanda. Además, se guarda la última carpeta seleccionada en las propiedades de la aplicación.
 * Utiliza el código que te sea útil para tu solución.
 * 
*/

class MyRender : public Renderer {
public:
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;
private:
	void buildGUI();
	std::shared_ptr<GLMatrices> mats;
	Program shader;
	std::shared_ptr<Label> cursorPos, viewportSizeLbl;
	glm::uvec2 windowSize{ 0 };

	std::shared_ptr<ListBoxWidget<>> tilesList;
	std::shared_ptr<Label> renderedTilesLbl;

	bool loadGeoTif(const std::string& filename);
	void refreshTilesList(const std::string& folder);

	std::string tilesFolder;
	Axes axes{ 500.f };
};




void MyRender::buildGUI() {
	// Creamos un panel nuevo donde introducir nuestros controles
	auto panel = addPanel("Location");
	// Tamaño del panel y posición del panel
	panel->setSize(260, 520);
	panel->setPosition(530, 10);

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

// Carga el fichero indicado
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

	return true;
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

void MyRender::setup() {
	glClearColor(0.6f, 0.6f, 0.9f, 1.0f);
	mats = GLMatrices::build();

	buildGUI();
	setCameraHandler(std::make_shared<XYPanZoomCamera>(
		1.0f, glm::vec3{ 0.0f }));
}


void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

	uint32_t count = 0;
	renderedTilesLbl->setText(std::to_string(count));

	ConstantIllumProgram::use();
	axes.render();
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
