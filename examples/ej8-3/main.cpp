

#include <sstream>

#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"
#include "GUI3.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

Ejemplo de iluminación basada en imagen

*/

class MyRender : public Renderer {
public:
  MyRender() : sph(0.5, 50, 50), sky(100.f, 100.f, 100.f) {};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  std::shared_ptr<GLMatrices> mats;
  std::shared_ptr<Program> reflectProgram;
  Axes axes;
  Program skyboxProgram;
  Sphere sph;
  Box sky;
  TextureCubeMap envmap, diffusemap, specularmap;
  GLint baseColorLoc;
  vec4 baseColor;
  std::shared_ptr<CheckBoxWidget> showAxes;
  void buildGUI();

};

void MyRender::setup() {
  glClearColor(.7f, .7f, .7f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  baseColor = vec4(0.4, 0.4, 0.4, 1.0);

  reflectProgram = std::make_shared<Program>();
  reflectProgram->addAttributeLocation(Mesh::VERTICES, "position");
  reflectProgram->addAttributeLocation(Mesh::NORMALS, "normal");

  mats = GLMatrices::build();
  reflectProgram->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

  reflectProgram->loadFiles(App::examplesDir() + "ej8-3/reflect2");
  reflectProgram->compile();

  GLint diffusemapLoc = reflectProgram->getUniformLocation("diffuseMap");
  GLint specularmapLoc = reflectProgram->getUniformLocation("specularMap");
  baseColorLoc = reflectProgram->getUniformLocation("baseColor");

  /* Este shader dibuja el cielo */
  skyboxProgram.addAttributeLocation(Mesh::VERTICES, "position");

  skyboxProgram.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  skyboxProgram.loadFiles(App::assetsDir() + "shaders/skybox");
  skyboxProgram.compile();
  GLint envmapLoc = skyboxProgram.getUniformLocation("cubeMap");

  if (!envmap.loadDDS(App::assetsDir() + "images/uffizi.dds"))
    throw std::runtime_error(
      "No se han podido cargar el mapa cúbico de uffizi.dds");

  if (!diffusemap.loadDDS(App::assetsDir() + "images/uffizi-diffuse.dds"))
    throw std::runtime_error(
      "No se han podido cargar el mapa cúbico de uffizi-diffuse.dds");

  if (!specularmap.loadDDS(App::assetsDir() + "images/uffizi-phong-100.dds"))
    throw std::runtime_error(
      "No se han podido cargar el mapa cúbico de uffizi-phong-100.dds");

  envmap.bind(GL_TEXTURE0);
  diffusemap.bind(GL_TEXTURE1);
  specularmap.bind(GL_TEXTURE2);

  reflectProgram->use();
  glUniform1i(diffusemapLoc, 1);
  glUniform1i(specularmapLoc, 2);
  glUniform4fv(baseColorLoc, 1, &baseColor.r);

  skyboxProgram.use();
  glUniform1i(envmapLoc, 0);

  setCameraHandler(std::make_shared<OrbitCameraHandler>());

  buildGUI();
  App::getInstance().getWindow().showGUI(true);
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

  reflectProgram->use();
  sph.render();

  // Entorno
  skyboxProgram.use();
  sky.render();

  // Ejes
  if (showAxes->get()) {
    ConstantIllumProgram::use();
    axes.render();
  }
  CHECK_GL();
}

void MyRender::buildGUI() {
  auto panel = addPanel("Image-based lighting");
  panel->setPosition(500, 10);
  panel->setSize(295, 100);

  panel->addWidget(std::make_shared<FloatSliderWidget>("% especular", 0.5f, 0.0f, 1.0f, reflectProgram, "specularPercent"));

  panel->addWidget(std::make_shared<FloatSliderWidget>("% difuso", 0.5f, 0.0f, 1.0f, reflectProgram, "diffusePercent"));

  showAxes = std::make_shared<CheckBoxWidget>("Mostrar ejes", false);
  panel->addWidget(showAxes);


}

void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.setInitWindowSize(800, 600);
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
    PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
