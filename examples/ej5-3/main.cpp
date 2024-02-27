

#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

struct paddedfloat {
  GLfloat f;
  char _pad[sizeof(vec4) - sizeof(float)];
};
struct TransformBlock {
  GLfloat scale;
  char _pad1[sizeof(vec4) - sizeof(GLfloat)];
  glm::vec3 translation;
  char _pad2[sizeof(vec4) - sizeof(vec3)];
  paddedfloat rotation[3];
  glm::mat4 proj_matrix;
};

/*

Ejemplo de uso de los Uniform Buffer Objects.
Pulsa Ctrl+I durante la ejecución del programa para mostrar información sobre
los miembros del Uniform Buffer Object.

*/

class MyRender : public Renderer {
public:
  MyRender() : sph(0.5, 20, 20, vec4(1.0, 0.0, 0.0, 1.0)), bo(NULL){};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  Program dshader;
  Sphere sph;
  std::shared_ptr<BufferObject> bo;
  TransformBlock tb;
};

void MyRender::setup() {
  glClearColor(.7f, .7f, .7f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  // Creamos un buffer object (BO) del tamaño suficiente
  bo = BufferObject::build(sizeof(TransformBlock));

  // Preparamos el contenido de la estructura en memoria de CPU que luego
  // transferiremos al BO
  tb.scale = 0.5;
  tb.translation = vec3(0.0, 0.0, -1.0);
  tb.rotation[0].f = glm::radians(45.0f);
  tb.rotation[1].f = 0.0;
  tb.rotation[2].f = glm::radians(60.0f);
  tb.proj_matrix = glm::perspective(glm::radians(60.0f), 1.0f, .1f, 10.0f);

  // Vinculamos el BO al punto de vinculación GL_UNIFORM_BUFFER
  // Es equivalente a glBindBuffer(GL_UNIFORM_BUFFER, bo->getId());
  gl_uniform_buffer.bind(bo);
  // Escribimos en el BO vinculado a GL_UNIFORM_BUFFER el contenido completo de
  // tb
  gl_uniform_buffer.write(&tb);

  dshader.addAttributeLocation(Mesh::VERTICES, "position");
  dshader.addAttributeLocation(Mesh::COLORS, "vertcolor");

  dshader.loadFiles(App::examplesDir() + "ej5-3/ubo-dummy");
  dshader.compile();

  // 1. Calcular el índice del bloque
  GLuint bindex = glGetUniformBlockIndex(dshader.getId(), "TransformBlock");
  if (bindex == GL_INVALID_INDEX)
    ERRT("No se ha encontrado el bloque TransformBlock");
  CHECK_GL();

  // 2. Conecta el bloque de uniforms del shader a un índice del punto de
  // vinculación dado
  glUniformBlockBinding(dshader.getId(), bindex, 0);
  CHECK_GL();

  // 3. Vincular el BO al mismo índice del punto de vinculación
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, bo->getId());
  CHECK_GL();

  dshader.use();
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  sph.render();
  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) { glViewport(0, 0, w, h); }

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
                                PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
