

#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

Ejemplo de dibujo instanciado.

*/

class MyRender : public Renderer {
public:
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  std::shared_ptr<GLMatrices> mats;
  Program ashader;
  GLint texUnitLoc;
  Model celda;
  Texture2DArray tiles;
};

void MyRender::setup() {
  glClearColor(1.f, 1.f, 1.f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  /* Este shader se encarga aplicar una textura a los objetos dibujados
    en cuenta una textura */
  // Posición de los vértices
  ashader.addAttributeLocation(Mesh::VERTICES, "position");
  // Coordenadas de textura
  ashader.addAttributeLocation(Mesh::TEX_COORD0, "texCoord");

  mats = GLMatrices::build();
  ashader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
  ashader.loadFiles(App::examplesDir() + "ej7-4/shader");
  ashader.compile();

  // Localización del uniform con la unidad de textura
  texUnitLoc = ashader.getUniformLocation("texUnit");

  // Le decimos al shader qué unidad de textura utilizar
  ashader.use();
  glUniform1i(texUnitLoc, 0);

  // La geometría consiste en un cuadrilátero de lado uno...
  auto mesh = std::make_shared<Mesh>();
  std::vector<vec3> v;
  v.push_back(vec3(-0.5, -0.5, 0.0));
  v.push_back(vec3(0.5, -0.5, 0.0));
  v.push_back(vec3(0.5, 0.5, 0.0));
  v.push_back(vec3(-0.5, 0.5, 0.0));

  // con las coordenadas de textura cubriendo toda la textura...
  std::vector<vec2> ct;
  ct.push_back(vec2(0.0, 0.0));
  ct.push_back(vec2(1.0, 0.0));
  ct.push_back(vec2(1.0, 1.0));
  ct.push_back(vec2(0.0, 1.0));

  mesh->addVertices(v);
  mesh->addTexCoord(0, ct);
  // y dibujando 64 copias cada vez que se dibuje (internamente se llama a
  // glDrawArraysInstanced)
  mesh->addDrawCommand(new DrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 8 * 8));

  celda.addMesh(mesh);

  // Cargamos las imágenes de la textura desde fichero
  Image t(App::assetsDir() + "images/tile1.jpg");
  tiles.allocate(t.getWidth(), t.getHeight(), 2, GL_RGBA);
  tiles.loadSlice(t, 0);
  tiles.loadSlice(Image(App::assetsDir() + "images/tile2.jpg"), 1);
  tiles.bind(GL_TEXTURE0);
    
  tiles.generateMipmap();
  tiles.setMinFilter(GL_LINEAR_MIPMAP_LINEAR);

  setCameraHandler(std::make_shared<OrbitCameraHandler>(2.0f));
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mats->setMatrix(GLMatrices::VIEW_MATRIX, getCamera().getViewMatrix());

  // Dibujamos el plano texturado
  ashader.use();
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->scale(GLMatrices::MODEL_MATRIX, 0.2f, 0.2f, 0.2f);
  celda.render();
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

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
                                PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
