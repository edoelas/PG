
#include <PGUPV.h>
#include <GUI3.h>
#include <iomanip>

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
	std::shared_ptr<GLMatrices> mats;
	std::unique_ptr<PGUPV::Mesh> boundary;
	std::shared_ptr<Label> cursorPos;
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
	panel->setPosition(530, 10);

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

	std::ostringstream os;
	os << std::fixed << std::setprecision(2) << me.x << " " << me.y;
	cursorPos->setText(os.str());
	return false;
}

void MyRender::setup() {
	glClearColor(0.6f, 0.6f, 0.9f, 1.0f);
	mats = GLMatrices::build();

	buildGUI();

	// Creamos un cuadrado para que la escena no esté vacía. Lo puedes usar para mostrar
	// los límites de la ciudad.
	boundary = std::make_unique<PGUPV::Mesh>();
	auto ll = glm::vec2{ -50.f, -50.0f};
	auto ur = glm::vec2{ 50.f, 50.0f};
	boundary->addVertices(
		{ ll,
		glm::vec2{ur.x, ll.y}, 
		ur,	
		glm::vec2{ll.x, ur.y}});
	boundary->addDrawCommand(new PGUPV::DrawArrays(GL_LINE_LOOP, 0, 4));

	setCameraHandler(std::make_shared<XYPanZoomCamera>(
		1000.0f, glm::vec3{ 0.0f}));
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT);

	mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
	mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

	ConstantUniformColorProgram::use();
	ConstantUniformColorProgram::setColor(glm::vec4{ 0.8f, 0.1f, 0.1f, 1.0f });
	boundary->render();
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
