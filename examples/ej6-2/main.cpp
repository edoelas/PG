

#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

Ejemplo de shader de geometría que descarta selectivamente primitivas.

*/

#define SPHERE_RADIUS 0.02f

class MyRender : public Renderer {
public:
  MyRender() : sphere(SPHERE_RADIUS, 10, 10, vec4(1.0, 0.0, 0.0, 1.0)){};
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
  GLint sphereLoc;
  float sphereRadius;
  vec4 spherePosWCS;
  Sphere sphere;
  std::shared_ptr<Scene> teapot;
  Rect plane;
  CircularInterpolator ci;
};

void MyRender::update(uint64_t ms) {
  static uint64_t total = 0;

  total += ms;
  spherePosWCS = ci.interpolate(total / 1000.0);
}

void MyRender::setup() {
  glClearColor(1.f, 1.f, 1.f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  // Inicialización de variables de estado
  // Posición de la luz en el S.C. del mundo
  lightPosWCS = vec4(2, 2, 2, 1);
  // Radio de la esfera
  sphereRadius = SPHERE_RADIUS;
  // Posición del centro de la esfera en el S.C. del mundo
  spherePosWCS = vec4(.8f, 0.f, 0.2f, 1.0f);

  // Este objeto interpola circularmente.
  // Se usa para calcular la posición de la esfera en cada momento.
  ci = CircularInterpolator(vec4(0.0f), spherePosWCS, vec3(0, .2, 1), 4);

  teapot = FileLoader::load(App::assetsDir() + "models/teapot.3ds");

  /* El shader se encarga de calcular la iluminación (sólo componente difusa)*/
  ashader.addAttributeLocation(Mesh::VERTICES, "position");
  ashader.addAttributeLocation(Mesh::NORMALS, "normal");

  mats = GLMatrices::build();
  ashader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

  ashader.loadFiles(App::examplesDir() + "ej6-2/ej6-2");
  ashader.compile();

  // Posiciones de las variables uniform
  lightPosLoc = ashader.getUniformLocation("lightpos");
  sphereLoc = ashader.getUniformLocation("sphere");

  setCameraHandler(std::make_shared<OrbitCameraHandler>());
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  mat4 viewMatrix = getCamera().getViewMatrix();
  mats->setMatrix(GLMatrices::VIEW_MATRIX, viewMatrix);

  ashader.use();
  // Posición de la fuente en el S.C. de la cámara
  vec4 lp = viewMatrix * lightPosWCS;
  glUniform3f(lightPosLoc, lp.x, lp.y, lp.z);

  // Posición del centro de la esfera en S.C. de la cámara
  vec4 sp = viewMatrix * spherePosWCS;
  sp.w = sphereRadius;
  glUniform4fv(sphereLoc, 1, &sp.x);

  // Dibujando la tetera
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->scale(GLMatrices::MODEL_MATRIX, vec3(1.0f / teapot->maxDimension()));
  teapot->render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  // Un shader sencillo para dibujar el color definido por cada vértice
  ConstantIllumProgram::use();

  // Dibujamos los ejes de coordenadas
  axes.render();

  // Dibujamos la esfera
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, vec3(spherePosWCS));
  sphere.render();
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
  mats->loadIdentity(GLMatrices::MODEL_MATRIX);
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
                                PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
