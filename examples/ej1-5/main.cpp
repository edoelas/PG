#include <PGUPV.h>
#include <GUI3.h>

using namespace PGUPV;

using glm::vec3;
using std::vector;

/**
 * @brief Ejemplo de un visualizador de modelos 3D
*/

class MyRender : public Renderer {
public:
	MyRender() = default;
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;
	void update(uint64_t) override;
private:
	void buildModels();
	std::shared_ptr<GLMatrices> mats;
	Axes axes;
	std::vector<std::shared_ptr<Model>> models;
	
	void buildGUI();
	std::shared_ptr<IntSliderWidget> modelSelector;
};

/**
 Puedes añadir más modelos al vector models
*/
void MyRender::buildModels()
{

	auto caja1 = std::make_shared<Box>(0.8f, 0.8f, 0.8f, glm::vec4(0.8f, 0.2f, 0.2f, 1.0f));
	models.push_back(caja1);
	auto caja2 = std::make_shared<Box>(0.9f, 0.9f, 0.9f, glm::vec4(0.2f, 0.8f, 0.2f, 1.0f));
	models.push_back(caja2);
	auto caja3 = std::make_shared<Box>(0.7f, 0.7f, 0.7f, glm::vec4(0.2f, 0.2f, 0.8f, 1.0f));
	models.push_back(caja3);
}


void MyRender::setup() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	// Habilitamos el z-buffer
	glEnable(GL_DEPTH_TEST);
	// Habilitamos el back face culling. ¡Cuidado! Si al dibujar un objeto hay 
	// caras que desaparecen o el modelo se ve raro, puede ser que estés 
	// definiendo los vértices del polígono del revés (en sentido horario)
	// Si ese es el caso, invierte el orden de los vértices.
	// Puedes activar/desactivar el back face culling en cualquier aplicación 
	// PGUPV pulsando las teclas CTRL+B
	glEnable(GL_CULL_FACE);

	mats = PGUPV::GLMatrices::build();

	// Activamos un shader que dibuja cada vértice con su atributo color
	ConstantIllumProgram::use();

	buildModels();

	// Establecemos una cámara que nos permite explorar el objeto desde cualquier
	// punto
	setCameraHandler(std::make_shared<OrbitCameraHandler>());

	// Construimos la interfaz de usuario
	buildGUI();
}

void MyRender::render() {
	// Borramos el buffer de color y el zbuffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Le pedimos a la cámara que nos de la matriz de la vista, que codifica la
	// posición y orientación actuales de la cámara.
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());


	// Dibujamos los ejes
	axes.render();

	// Dibujamos los objetos
	if (!models.empty()) {
		models[modelSelector->get()]->render();
	}

	// Si la siguiente llamada falla, quiere decir que OpenGL se encuentra en
	// estado erróneo porque alguna de las operaciones que ha ejecutado
	// recientemente (después de la última llamada a CHECK_GL) ha tenido algún
	// problema. Revisa tu código.
	CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
}

// Este método se ejecuta una vez por frame, antes de llamada a render. Recibe el 
// número de milisegundos que han pasado desde la última vez que se llamó, y se suele
// usar para hacer animaciones o comprobar el estado de los dispositivos de entrada
void MyRender::update(uint64_t) {
	// Si el usuario ha pulsado el espacio, ponemos la cámara en su posición inicial
	if (App::isKeyUp(PGUPV::KeyCode::Space)) {
		getCameraHandler()->resetView();
	}
}


/**
En éste método construimos los widgets que definen la interfaz de usuario. En esta
práctica no tienes que modificar esta función.
*/
void MyRender::buildGUI() {
	auto panel = addPanel("Modelos");
	modelSelector = std::make_shared<IntSliderWidget>("Model", 0, 0, static_cast<int>(models.size()-1));

	if (models.size() <= 1) {
		panel->addWidget(std::make_shared<Label>("Introduce algún modelo más en el vector models..."));
	}
	else {
		panel->addWidget(modelSelector);
	}
	App::getInstance().getWindow().showGUI();
}

int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
		PGUPV::MULTISAMPLE);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
