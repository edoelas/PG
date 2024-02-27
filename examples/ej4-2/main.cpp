
#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"
// GUI: Para añadir widgets a tu aplicación, incluye esta cabecera:
#include "GUI3.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

 Usando un uniform para definir el color de las primitivas a dibujar.
 Ahora definimos el color de una de las teteras mediante un widget.

 Los widgets de PGUPV permiten escribir directamente sobre el uniform de un
 programa cada vez que el usuario interacciona con ellos.
 La mayoría de widgets tienen un constructor que recibe una referencia a
 un programa, y el nombre de un uniform. De esta forma, el widget actualizará
 el valor del uniform en el programa cada vez que cambie su estado.
 En este ejemplo actualizamos un vec4 que representa un color mediante el widget
 RGBAColorWidget pero, por ejemplo, un FloatSliderWidget escribirá en un uniform de
 tipo float cada vez que el usuario use el widget correspondiente.
 */

class MyRender : public Renderer {
public:
  MyRender() : program(std::shared_ptr<Program>(new Program())){};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
  void update(uint64_t ms) override;

private:
  std::shared_ptr<GLMatrices> mats;
  std::shared_ptr<Program> program;
  std::shared_ptr<Scene> teapot;          // Modelo de una tetera
  float teapotSpin;          // Ángulo de rotación actual de la tetera
                             // la tetera

  // GUI: No es necesario, pero siempre queda más limpio crear la interfaz en
  // una función aparte
  void buildGUI();

};

void MyRender::setup() {
  glClearColor(.1f, .1f, .1f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  teapotSpin = 0.0f;

  // Cargando un modelo desde un fichero
  teapot = FileLoader::load(App::assetsDir() + "models/teapot.3ds");

  // De todos los atributos del modelo teapot, sólo usaremos la posición de los
  // vértices
  program->addAttributeLocation(Mesh::VERTICES, "position");

  mats = GLMatrices::build();
  program->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  program->loadFiles(App::assetsDir() + "shaders/constantshadinguniform");
  program->compile();

  // La cámara no se mueve
  mats->setMatrix(GLMatrices::VIEW_MATRIX,
    glm::lookAt(vec3(0.0f, 0.0f, 2.5f), vec3(0.0f, 0.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f)));

  // Sólo tenemos un shader, así que lo activamos aquí
  program->use();

  // GUI: construyendo la interfaz de usuario
  buildGUI();

  // GUI: Por defecto, los widgets son invisibles. Para mostrar la interfaz se
  // puede pulsar Ctrl+H, o hacerlos visibles así:
  App::getInstance().getWindow().showGUI(true);
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Tetera
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->rotate(GLMatrices::MODEL_MATRIX, teapotSpin, vec3(1.0f, 1.0f, 0.0f));
  mats->scale(GLMatrices::MODEL_MATRIX, vec3(1.7f / teapot->maxDimension()));
  teapot->render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  if (h == 0)
    h = 1;
  float ar = (float)w / h;
  mats->setMatrix(GLMatrices::PROJ_MATRIX,
    glm::perspective(glm::radians(60.0f), ar, .1f, 100.0f));
}

// Radianes por segundo a los que gira la tetera
#define SPINSPEED glm::radians(180.0f)

void MyRender::update(uint64_t ms) {
  teapotSpin += SPINSPEED*ms / 1000.0f;
  if (teapotSpin > TWOPIf) teapotSpin -= TWOPIf;
}

// GUI: Aquí construímos el interfaz
void MyRender::buildGUI() {
  // Un Panel representa un conjunto de widgets agrupados en una ventana, con un título
  auto panel = addPanel("Color");

  // Podemos darle un tamaño y una posición, aunque no es necesario
  panel->setPosition(5, 5);
  panel->setSize(215, 70);

  // Ahora añadimos un widget para definir el valor de un uniform de tipo vec4 que 
  // representa un color
  auto color = std::make_shared<RGBAColorWidget>(
    "Color", // Etiqueta
    vec4(0.7, 0.7, 0.7, 1.0), // Valor inicial (gris)
    program,           // El programa que contiene el uniform
    "primitive_color"); // El nombre del uniform en el shader
  // Insertamos el widget en el grupo
  panel->addWidget(color);

  // Y ya está!
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
