

#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"
#include "GUI3.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

// Color del cielo y del suelo
static const glm::vec4 SKY_COLOR(51.0 / 255, 204. / 255, 204. / 255, 1.0);
static const glm::vec4 GROUND_COLOR(0.5f, 0.0f, 0.0f, 1.0f);

/*

Ejemplo de iluminación semiesférica.

*/

class MyRender : public Renderer {
public:
  MyRender() :
    lightSource(.05f, 6, 6, glm::vec4(0.0f)),
    floor(3.0f, 3.0f, GROUND_COLOR),
    teapotSpin(0.0f)
  {};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
  void update(uint64_t ms) override;
private:
  std::shared_ptr<GLMatrices> mats;
  Program cshader, ashader;
  std::shared_ptr<Program> hemiShader;
  GLint lightPosLoc;
  vec3 lightPosWCS;
  Sphere lightSource;
  std::shared_ptr<Scene> teapot;
  Rect floor;
  mat4 floorXform;
  float teapotSpin;
  void buildGUI();
};


// Radianes por segundo a los que gira la tetera
#define SPINSPEED glm::radians(180.0f)

void MyRender::update(uint64_t ms) {
  teapotSpin += SPINSPEED*ms / 1000.0f;
  if (teapotSpin > TWOPIf) teapotSpin -= TWOPIf;
}

void MyRender::setup() {
  glClearColor(SKY_COLOR.r, SKY_COLOR.g, SKY_COLOR.b, 1.0f);
  glEnable(GL_DEPTH_TEST);

  // Inicialización de variables de estado
  // Posición de la luz en el S.C. del mundo
  lightPosWCS = vec3(0.0, 3.0, 0.0);

  teapot = FileLoader::load(App::assetsDir() + "models/teapot.3ds");

  /* Este shader se encarga de calcular la iluminación semiesférica*/
  hemiShader = std::make_shared<Program>();
  hemiShader->addAttributeLocation(Mesh::VERTICES, "position");
  hemiShader->addAttributeLocation(Mesh::NORMALS, "normal");

  mats = GLMatrices::build();
  hemiShader->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

  hemiShader->loadFiles(App::examplesDir() + "ej8-1/shader");
  hemiShader->compile();

  // Posición de las variables uniform
  lightPosLoc = hemiShader->getUniformLocation("lightpos");

  // Transformación del suelo
  floorXform = glm::translate(glm::mat4(1.0f), vec3(0.0f, -0.5f, 0.0f)) *
    glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));


  /* Este shader se encarga de dibujar los objetos usando los colores
  proporcionados por los vértices del modelo */
  cshader.addAttributeLocation(Mesh::VERTICES, "position");
  cshader.addAttributeLocation(Mesh::COLORS, "vertcolor");
  cshader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  cshader.loadFiles(App::assetsDir() + "shaders/constantshading");
  cshader.compile();

  setCameraHandler(std::make_shared<OrbitCameraHandler>());

  buildGUI();
  App::getInstance().getWindow().showGUI(true);
}


void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glm::mat4 viewMatrix = getCamera().getViewMatrix();
  mats->setMatrix(GLMatrices::VIEW_MATRIX, viewMatrix);

  hemiShader->use();
  // Posición de la fuente en el S.C. de la cámara
  vec4 lp = viewMatrix * glm::vec4(lightPosWCS, 1.0);
  glUniform3f(lightPosLoc, lp.x, lp.y, lp.z);

  // Tetera que gira sobre sí misma
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->rotate(GLMatrices::MODEL_MATRIX, teapotSpin, vec3(1.0f, 1.0f, 0.0f));
  mats->scale(GLMatrices::MODEL_MATRIX, vec3(1.0f / teapot->maxDimension()));
  teapot->render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  // Un shader para dibujar el color definido por cada vértice
  cshader.use();

  // Dibujando el suelo
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->setMatrix(GLMatrices::MODEL_MATRIX, floorXform);
  floor.render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  // Dibujamos la fuente
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, vec3(lightPosWCS));
  lightSource.render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  int m = MIN(w, h);
  glViewport(0, 0, m, m);
  mats->setMatrix(GLMatrices::PROJ_MATRIX, glm::perspective(glm::radians(60.0f), 1.0f, .1f, 100.0f));
  mats->loadIdentity(GLMatrices::MODEL_MATRIX);
}

void MyRender::buildGUI() {
  auto panel = addPanel("Iluminación semiesférica");
  panel->setPosition(500, 10);
  panel->setSize(300, 100);

  auto sc = std::make_shared<RGBAColorWidget>("Cielo", SKY_COLOR, hemiShader, "skyColor");
  sc->getValue().addListener([](const glm::vec4 &nc) { glClearColor(nc.r, nc.g, nc.b, nc.a); });
  panel->addWidget(sc);

  auto gc = std::make_shared<RGBAColorWidget>("Suelo", GROUND_COLOR, hemiShader, "groundColor");
  gc->getValue().addListener([this](const glm::vec4 &nc) { floor.getMesh(0).setColor(nc); });
  panel->addWidget(gc);


}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.setInitWindowSize(800, 600);
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER | PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}

