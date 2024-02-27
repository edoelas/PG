

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

 Demo de un paisaje implementado mediante una textura con un mapa cúbico.
 También se incluye
 una escena con objetos que reflejan el entorno.

 */

class MyRender : public Renderer {
public:
  MyRender()
      : cone(0.5f, 0.00001f), sph(0.5, 50, 50), box(0.2f, 1.0f, 0.5f),
        sky(100.f, 100.f, 100.f) {};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  std::shared_ptr<GLMatrices> mats;
  Axes axes;
  std::shared_ptr<Program> reflectProgram, skyboxProgram;
  Cylinder cone;
  Sphere sph;
  Box box, sky;
  GLint envmapLoc, skyboxLevoLoc, reflectLevoLoc;
  TextureCubeMap envmap;
  GLenum cubeMapLoc;
  std::shared_ptr<CheckBoxWidget> showAxis, showScene;
  void buildGUI();
    
};

void MyRender::setup() {
  glClearColor(.7f, .7f, .7f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  /* Este shader se encarga de aplicar el color calculado como un
  reflejo de los objetos sobre el mapa de entorno. */
  reflectProgram = std::make_shared<Program>();
  reflectProgram->addAttributeLocation(Mesh::VERTICES, "position");
  reflectProgram->addAttributeLocation(Mesh::NORMALS, "normal");

  mats = GLMatrices::build();
  reflectProgram->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  reflectProgram->loadFiles(App::examplesDir() + "ej8-2/reflect");
  reflectProgram->compile();

  envmapLoc = reflectProgram->getUniformLocation("envmap");
  reflectLevoLoc = reflectProgram->getUniformLocation("levo");

  /* Este shader dibuja el cielo */
  skyboxProgram = std::make_shared<Program>();
  skyboxProgram->addAttributeLocation(Mesh::VERTICES, "position");
  skyboxProgram->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  skyboxProgram->loadFiles(App::assetsDir() + "shaders/skybox");
  skyboxProgram->compile();

  cubeMapLoc = skyboxProgram->getUniformLocation("cubeMap");
  skyboxLevoLoc = skyboxProgram->getUniformLocation("levo");

  // Cargamos las imágenes que forman la textura del mapa cúbico
  envmap.loadImages(App::assetsDir() + "images/GoldenGateBridge2/.jpg", true);
  envmap.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);
  envmap.generateMipmap();
    
  envmap.bind(GL_TEXTURE0);
  reflectProgram->use();
  glUniform1i(envmapLoc, 0);
  skyboxProgram->use();
  glUniform1i(cubeMapLoc, 0);

  setCameraHandler(std::make_shared<OrbitCameraHandler>());

  buildGUI();
  App::getInstance().getWindow().showGUI(true);
}

void MyRender::render() {
  // No hace falta borrar el buffer de color porque el mapa cúbico ocupa toda la
  // ventana
  glClear(GL_DEPTH_BUFFER_BIT);
  mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

  // Entorno
  skyboxProgram->use();
  sky.render();

  // Objetos con material de espejo
  if (showScene->get()) {
    reflectProgram->use();

    mats->pushMatrix(GLMatrices::MODEL_MATRIX);
    mats->translate(GLMatrices::MODEL_MATRIX, 1.0, 0.0, -0.5);
    cone.render();
    mats->popMatrix(GLMatrices::MODEL_MATRIX);

    sph.render();

    mats->pushMatrix(GLMatrices::MODEL_MATRIX);
    mats->translate(GLMatrices::MODEL_MATRIX, -0.75f, 0.0f, 0.4f);
    mats->rotate(GLMatrices::MODEL_MATRIX, glm::radians(150.0f), 0.0, 1.0, 0.0);
    box.render();
    mats->popMatrix(GLMatrices::MODEL_MATRIX);
  }

  // Ejes
  if (showAxis->get()) {
    ConstantIllumProgram::use();
    axes.render();
  }
  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
}

void MyRender::buildGUI() {

    auto panel = addPanel("Environment mapping");
    panel->setPosition(300, 10);
    panel->setSize(200, 100);
    
    showAxis = std::make_shared<CheckBoxWidget>("Mostrar ejes", true);
    panel->addWidget(showAxis);
    showScene = std::make_shared<CheckBoxWidget>("Mostrar escena", true);
    panel->addWidget(showScene);
    
    auto cbLevo = std::make_shared<CheckBoxWidget>("Levógiro", true);
    cbLevo->getValue().addListener( [this](bool levo) {
        skyboxProgram->use();
        glUniform1i(skyboxLevoLoc, levo);
        reflectProgram->use();
        glUniform1i(reflectLevoLoc, levo);
    });
    panel->addWidget(cbLevo);
    
    
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
                                PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
