
#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"
#include "GUI3.h"

using namespace PGUPV;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

Este ejemplo muestra los distintos tipos de teselación: quads, triangles e
isolines

*/

class MyRender : public Renderer {
public:
  MyRender()
    : what(QUADS), outerLevel(2.0, 3.0, 2.0, 4.0), innerLevel(3.0, 4.0) {}
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;
  void refreshUniforms(void);

private:
  Program quadsShader, trianglesShader, isolinesShader, isolinesShaderPoints;
  GLint qSOuterLevelLoc, qSInnerLevelLoc, tSOuterLevelLoc, tSInnerLevelLoc;
  GLint iSOuterLevelLoc, iSPOuterLevelLoc;
  Model rectangulo, triangulo;

  int what;
  vec4 outerLevel;
  vec2 innerLevel;
  void buildGUI();

  enum { QUADS, TRIANGLES, ISOLINES };
  static const std::vector<std::string> intTypesNames;
};

const std::vector<std::string> MyRender::intTypesNames{ "QUADS", "TRIANGLES", "ISOLINES" };


void MyRender::setup() {
  glClearColor(1.0f, 1.f, 1.0f, 1.0f);

  // Creamos un parche con cuatro vértices dentro de un Model
  auto m = std::make_shared<Mesh>();
  std::vector<glm::vec3> vs;
  vs.push_back(glm::vec3(-0.75, -0.75, 0.0));
  vs.push_back(glm::vec3(0.75, -0.75, 0.0));
  vs.push_back(glm::vec3(0.75, 0.75, 0.0));
  vs.push_back(glm::vec3(-0.75, 0.75, 0.0));
  m->addVertices(vs);

  DrawArrays *dc = new DrawArrays(GL_PATCHES, 0, 4);
  dc->setVerticesPerPatch(4);
  m->addDrawCommand(dc);

  rectangulo.addMesh(m);

  // Creamos un parche con tres vértices dentro de un Model
  m = std::make_shared<Mesh>();
  vs.clear();
  vs.push_back(glm::vec3(0.75, -0.75, 0.0));
  vs.push_back(glm::vec3(0.0, 0.75, 0.0));
  vs.push_back(glm::vec3(-0.75, -0.75, 0.0));
  m->addVertices(vs);

  dc = new DrawArrays(GL_PATCHES, 0, 3);
  dc->setVerticesPerPatch(3);
  m->addDrawCommand(dc);

  triangulo.addMesh(m);

  // En vez de cargar todos los shaders de golpe, los cargamos por partes porque
  // los cuatro programas comparten varios shaders
  std::shared_ptr<Shader> vShader =
    Shader::loadFromFile(App::examplesDir() + "ej7-2/shader.vert");
  std::shared_ptr<Shader> fShader =
    Shader::loadFromFile(App::examplesDir() + "ej7-2/shader.frag");

  // Programa para teselar quads
  quadsShader.addAttributeLocation(Mesh::VERTICES, "position");
  quadsShader.addShader(vShader);
  quadsShader.addShader(fShader);
  quadsShader.addShader(Shader::loadFromFile(App::examplesDir() + "ej7-2/quads.tesc"));
  quadsShader.addShader(Shader::loadFromFile(App::examplesDir() + "ej7-2/quads.tese"));
  quadsShader.compile();

  // Localizando sus uniforms para controlar el nivel de teselación
  qSOuterLevelLoc = quadsShader.getUniformLocation("outerLevel");
  qSInnerLevelLoc = quadsShader.getUniformLocation("innerLevel");

  // Shader de control de teselación para isolineas
  std::shared_ptr<Shader> tcShader =
    Shader::loadFromFile(App::examplesDir() + "ej7-2/isolines.tesc");

  // Programa para teselar isolíneas
  isolinesShader.addAttributeLocation(Mesh::VERTICES, "position");
  isolinesShader.addShader(vShader);
  isolinesShader.addShader(fShader);
  isolinesShader.addShader(tcShader);
  isolinesShader.addShader(Shader::loadFromFile(App::examplesDir() + "ej7-2/isolines.tese"));
  isolinesShader.compile();

  iSOuterLevelLoc = isolinesShader.getUniformLocation("outerLevel");

  // Programa para teselar isolíneas en modo de puntos
  isolinesShaderPoints.addAttributeLocation(Mesh::VERTICES, "position");
  isolinesShaderPoints.addShader(vShader);
  isolinesShaderPoints.addShader(fShader);
  isolinesShaderPoints.addShader(tcShader);
  isolinesShaderPoints.addShader(
    Shader::loadFromFile(App::examplesDir() + "ej7-2/isolines-points.tese"));
  isolinesShaderPoints.compile();

  iSPOuterLevelLoc = isolinesShaderPoints.getUniformLocation("outerLevel");

  // Programa para teselar triángulos
  trianglesShader.addAttributeLocation(Mesh::VERTICES, "position");
  trianglesShader.addShader(vShader);
  trianglesShader.addShader(fShader);
  trianglesShader.addShader(Shader::loadFromFile(App::examplesDir() + "ej7-2/triangles.tesc"));
  trianglesShader.addShader(Shader::loadFromFile(App::examplesDir() + "ej7-2/triangles.tese"));
  trianglesShader.compile();

  tSOuterLevelLoc = trianglesShader.getUniformLocation("outerLevel");
  tSInnerLevelLoc = trianglesShader.getUniformLocation("innerLevel");

  buildGUI();
  App::getInstance().getWindow().showGUI(true);

  glPointSize(10);

  what = QUADS;
  refreshUniforms();

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void MyRender::refreshUniforms(void) {
  switch (what) {
  case TRIANGLES:
    trianglesShader.use();
    glUniform2f(tSInnerLevelLoc, innerLevel.x, innerLevel.y);
    glUniform4fv(tSOuterLevelLoc, 1, &outerLevel.x);
    break;
  case QUADS:
    quadsShader.use();
    glUniform2f(qSInnerLevelLoc, innerLevel.x, innerLevel.y);
    glUniform4fv(qSOuterLevelLoc, 1, &outerLevel.x);
    break;
  case ISOLINES:
    isolinesShader.use();
    glUniform4fv(iSOuterLevelLoc, 1, &outerLevel.x);
    isolinesShaderPoints.use();
    glUniform4fv(iSPOuterLevelLoc, 1, &outerLevel.x);
    break;
  }
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT);

  switch (what) {
  case TRIANGLES:
    trianglesShader.use();
    triangulo.render();
    break;
  case QUADS:
    quadsShader.use();
    rectangulo.render();
    break;
  case ISOLINES:
    // Para las isolíneas, dibujamos dos veces:
    // 1. Primero las líneas
    isolinesShader.use();
    rectangulo.render();
    // 2. Luego los vértices
    isolinesShaderPoints.use();
    rectangulo.render();
    break;
  }
  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  uint m = MIN(w, h);
  glViewport(0, 0, m, m);
}


void MyRender::buildGUI() {
  // Construyendo una GUI para controlar los niveles de teselación
  auto panel = addPanel("Teselación");
  panel->setPosition(450, 10);
  panel->setSize(330, 140);

  auto it = std::make_shared<ListBoxWidget<>>("Interpolación", intTypesNames);
  it->getValue().addListener([this](const int &i) { what = i; refreshUniforms(); });
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
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}