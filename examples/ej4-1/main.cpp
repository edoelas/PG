
#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

 Usando un uniform para definir el color de las primitivas a dibujar.

 */

class MyRender : public Renderer {
public:
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
  void update(uint64_t ms) override;

private:
  std::shared_ptr<GLMatrices> mats;
  Program ashader;
  std::shared_ptr<Scene> teapot;          // Modelo de una tetera
  float teapotSpin;          // Ángulo de rotación actual de la tetera
  GLint primitive_color_loc; // Localización del uniform que define el color de
                             // la tetera
};

void MyRender::setup() {
  glClearColor(.1f, .1f, .1f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  teapotSpin = 0.0f;

  // Cargando un modelo desde un fichero
  teapot = FileLoader::load(App::assetsDir() + "models/teapot.3ds");

  // De todos los atributos del modelo teapot, sólo usaremos la posición de los
  // vértices
  ashader.addAttributeLocation(Mesh::VERTICES, "position");

  mats = GLMatrices::build();
  ashader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  ashader.loadFiles(App::assetsDir() + "shaders/constantshadinguniform");
  ashader.compile();

  // Obtenemos la localización del uniform que determinará el color de la
  // primitiva
  primitive_color_loc = ashader.getUniformLocation("primitive_color");

  // La cámara no se mueve
  mats->setMatrix(GLMatrices::VIEW_MATRIX,
                  glm::lookAt(vec3(0.0f, 0.0f, 2.5f), vec3(0.0f, 0.0f, 0.0f),
                              vec3(0.0f, 1.0f, 0.0f)));

  // Sólo tenemos un shader, así que lo activamos aquí
  ashader.use();
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Tetera roja
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, -0.5, 0.5, 0.0);
  mats->rotate(GLMatrices::MODEL_MATRIX, teapotSpin, vec3(1.0f, 1.0f, 0.0f));
  mats->scale(GLMatrices::MODEL_MATRIX, vec3(1.3f / teapot->maxDimension()));
  glUniform4f(primitive_color_loc, 1.0, 0.0, 0.0, 1.0);
  teapot->render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  // Tetera azul
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, 0.5, -0.5, 0.0);
  mats->rotate(GLMatrices::MODEL_MATRIX, teapotSpin, vec3(1.0f, 1.0f, 0.0f));
  mats->scale(GLMatrices::MODEL_MATRIX, vec3(1.3f / teapot->maxDimension()));
  glUniform4f(primitive_color_loc, 0.0, 0.0, 1.0, 1.0);
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

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
