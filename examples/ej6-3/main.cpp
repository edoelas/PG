

#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

Ejemplo de un shader de geometría que, por cada triángulo de entrada, genera cuatro
nuevos triángulos: uno exactamente igual que el original, y otros tres como el
original, pero proyectados sobre los planos principales, mostrando
la planta, alzado y perfil del objeto.

*/

class MyRender : public Renderer {
public:
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
  void update(uint64_t ms) override;

private:
  std::shared_ptr<GLMatrices> mats;
  Axes axes;
  Program ashader;
  GLint lightPosLoc;
  vec4 lightPosWCS;
  std::shared_ptr<Scene> teapot;
  float teapotSpin;
};

void MyRender::setup() {
  glClearColor(1.f, 1.f, 1.f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  // Inicialización de variables de estado
  // Posición de la luz en el S.C. del mundo
  lightPosWCS = vec4(2, 2, 2, 1);

  teapotSpin = 0.0;

  teapot = FileLoader::load(App::assetsDir() + "models/teapot.3ds");

  /* Este shader se encarga de calcular la iluminación (sólo componente
   * difusa)*/
  ashader.addAttributeLocation(Mesh::VERTICES, "position");
  ashader.addAttributeLocation(Mesh::NORMALS, "normal");

  mats = GLMatrices::build();
  ashader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

  ashader.loadFiles(App::examplesDir() + "ej6-3/ej6-3");
  ashader.compile();

  // Posiciones de las variables uniform
  lightPosLoc = ashader.getUniformLocation("lightpos");

  setCameraHandler(std::make_shared<OrbitCameraHandler>(2.5f, glm::radians(45.0f), glm::radians(30.0f)));
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

  ashader.use();
  // Posición de la fuente en el S.C. de la cámara
  vec4 lp = getCamera().getViewMatrix() * lightPosWCS;
  glUniform3f(lightPosLoc, lp.x, lp.y, lp.z);

  // Dibujando la tetera
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, 0.5f, 0.5f, 0.5f);
  mats->rotate(GLMatrices::MODEL_MATRIX, teapotSpin, 0.0f, 1.0f, 0.0f);
  mats->scale(GLMatrices::MODEL_MATRIX, vec3(0.7f / teapot->maxDimension()));
  teapot->render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  // Un shader sencillo para dibujar el color definido por cada vértice
  ConstantIllumProgram::use();

  // Dibujamos los ejes de coordenadas
  axes.render();

  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  if (h == 0)
    h = 1;
  float ar = (float)w / h;
  mats->setMatrix(GLMatrices::PROJ_MATRIX,
                  glm::perspective(glm::radians(60.0f), ar, .1f, 100.0f));
  mats->loadIdentity(GLMatrices::MODEL_MATRIX);
}

// Radianes por segundo a los que gira la tetera
#define SPINSPEED glm::radians(180.0f)

void MyRender::update(uint64_t ms) {
  teapotSpin += SPINSPEED*ms / 1000.0f;
  if (teapotSpin > TWOPIf) teapotSpin -= TWOPIf;
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
                                PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
