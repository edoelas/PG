
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

Este ejemplo muestra la forma más sencilla de usar el teselador para subdividir
un cuadrado en triángulos.

*/

class MyRender : public Renderer {
public:
  MyRender() {}
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  void buildGUI();
  std::shared_ptr<Program> program;
  Model model;
    
};

void MyRender::setup() {
  glClearColor(1.0f, 1.f, 1.0f, 1.0f);

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

  model.addMesh(m);

  program = std::shared_ptr<Program>(new Program());
  program->addAttributeLocation(Mesh::VERTICES, "position");
  program->loadFiles(App::examplesDir() + "ej7-1/shader");
  program->compile();
  program->use();

  buildGUI();
  App::getInstance().getWindow().showGUI(true);

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
    panel->setPosition(500, 10);
    panel->setSize(280, 80);
    
    panel->addWidget(std::make_shared<Vec4SliderWidget>("Outer level",
      glm::vec4(2.0, 3.0, 2.0, 4.0), 0.f, 10.f, program, "levelOuter"));
    panel->addWidget(std::make_shared<Vec2SliderWidget>("Inner level", 
      glm::vec2(3.0f, 4.0f), 0.0f, 10.f, program, "levelInner"));
    
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.setInitWindowSize(800, 600);
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}