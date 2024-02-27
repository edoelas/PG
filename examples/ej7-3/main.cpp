
#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"
#include "GUI3.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

#define EQUAL_SPACING_TESE_SHADER App::examplesDir() + "ej7-3/shader-es.tese"
#define FRACTIONAL_ODD_SPACING_TESE_SHADER App::examplesDir() + "ej7-3/shader-fos.tese"
#define FRACTIONAL_EVEN_SPACING_TESE_SHADER App::examplesDir() + "ej7-3/shader-fes.tese"

#define OUTER_LEVEL_UNIF "levelOuter"
#define INNER_LEVEL_UNIF "levelInner"

#define SIDE_LENGTH 1.5f

/*

Este ejemplo muestra los diferentes tipos de espaciado en la teselación

*/

class MyRender : public Renderer {
public:
  MyRender()
    : outerLevelLoc(-1), innerLevelLoc(-1),
    outerLevel(glm::vec4(2.0, 2.0, 2.0, 2.0)), innerLevel(glm::vec2(3.0, 3.0)) {};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  enum Spacing { EQUAL, FRACTIONAL_EVEN, FRACTIONAL_ODD };
  static const std::vector<std::string> spacingNames;
  void setSpacing(Spacing s);
  void refreshUniforms(void);
  void buildGUI();
  std::shared_ptr<Program> program;
  Model model;
  GLint outerLevelLoc, innerLevelLoc;
  glm::vec4 outerLevel;
  glm::vec2 innerLevel;
};

const std::vector<std::string> MyRender::spacingNames{ "EQUAL", "FRACTIONAL_EVEN", "FRACTIONAL_ODD" };

void MyRender::refreshUniforms(void) {
  if (outerLevelLoc == -1 || innerLevelLoc == -1) {
    outerLevelLoc = program->getUniformLocation(OUTER_LEVEL_UNIF);
    innerLevelLoc = program->getUniformLocation(INNER_LEVEL_UNIF);
    if (outerLevelLoc == -1 || innerLevelLoc == -1) {
      ERRT("No se pueden encontrar los uniforms del shader");
    }
  }
  glUniform4f(outerLevelLoc, outerLevel.x, outerLevel.y, outerLevel.z, outerLevel.w);
  glUniform2f(innerLevelLoc, innerLevel.x, innerLevel.y);
}

/*
Esta función sustituye el shader de evaluación de teselación del programa,
dependiendo del tipo de espaciado a utilizar
*/
void MyRender::setSpacing(Spacing s) {
  program->removeShader(Shader::ShaderType::TESS_EVALUATION_SHADER);
  switch (s) {
  case EQUAL:
    program->addShader(Shader::loadFromFile(EQUAL_SPACING_TESE_SHADER));
    break;
  case FRACTIONAL_EVEN:
    program->addShader(
      Shader::loadFromFile(FRACTIONAL_EVEN_SPACING_TESE_SHADER));
    break;
  case FRACTIONAL_ODD:
    program->addShader(
      Shader::loadFromFile(FRACTIONAL_ODD_SPACING_TESE_SHADER));
    break;
  default:
    break;
  }
  program->use();
  // El programa ha cambiado. Los uniforms pueden haber cambiado de ubicación:
  // invalidar
  outerLevelLoc = innerLevelLoc = -1;
  refreshUniforms();
}

void MyRender::setup() {
  glClearColor(1.0f, 1.f, 1.0f, 1.0f);

  auto m = std::make_shared<Mesh>();
  std::vector<glm::vec3> vs;
  vs.push_back(glm::vec3(-0.5f, -0.5f, 0.f) * SIDE_LENGTH);
  vs.push_back(glm::vec3(0.5f, -0.5f, 0.f) * SIDE_LENGTH);
  vs.push_back(glm::vec3(0.5f, 0.5f, 0.f) * SIDE_LENGTH);
  vs.push_back(glm::vec3(-0.5f, 0.5f, 0.f) * SIDE_LENGTH);
  m->addVertices(vs);

  DrawArrays *dc = new DrawArrays(GL_PATCHES, 0, 4);
  dc->setVerticesPerPatch(4);
  m->addDrawCommand(dc);

  model.addMesh(m);

  program = std::shared_ptr<Program>(new Program());
  program->addAttributeLocation(Mesh::VERTICES, "position");
  program->loadFiles(App::examplesDir() + "ej7-3/shader");
  program->addShader(
    std::shared_ptr<Shader>(Shader::loadFromFile(EQUAL_SPACING_TESE_SHADER)));
  program->compile();

  buildGUI();
  App::getInstance().getWindow().showGUI(true);

  program->use();
  refreshUniforms();

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  model.render();
  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  uint min = MIN(w, h);
  glViewport(0, 0, min, min);
}



void MyRender::buildGUI() {
  // Construyendo una GUI para controlar los niveles de teselación
  auto panel = addPanel("Teselación");
  panel->setPosition(460, 10);
  panel->setSize(325, 140);

  auto it = std::make_shared<ListBoxWidget<>>("Espaciado", spacingNames);
  it->getValue().addListener([this](const int &i) { setSpacing(static_cast<Spacing>(i)); });
  panel->addWidget(it);

  auto ol = std::make_shared<Vec4SliderWidget>("Outer level", glm::vec4(2.0, 3.0, 2.0, 4.0),
    0.f, 10.f);
  ol->getValue().addListener([this](const glm::vec4 &v) { outerLevel = v; refreshUniforms(); });
  panel->addWidget(ol);

  auto il = std::make_shared<Vec2SliderWidget>("Inner level", glm::vec2(3.0f, 4.0f), 0.0f, 10.f);
  il->getValue().addListener([this](const glm::vec2 &v) { innerLevel = v; refreshUniforms(); });
  panel->addWidget(il);
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.setInitWindowSize(800, 600);
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
