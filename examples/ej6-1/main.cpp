

#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"

using namespace PGUPV;

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using std::vector;

/*

 Ejemplo de shader de geometría identidad.
 Este proyecto usa el shader de sombreado de Gouraud con un shader de geometría
 que sólo propaga los triángulos que le llegan.

 */

#define CONE_HEIGHT 0.15f
#define CONE_ANGULAR_RADIUS 20.0f

class MyRender : public Renderer {
public:
  MyRender()
      : cone(0.5f, 0.00001f),
        luz((float)(2.0 * tan(CONE_ANGULAR_RADIUS * M_PI / 180.0) *
                    CONE_HEIGHT),
            0.00001f, CONE_HEIGHT, 5, 10, vec4(1.0, 1.0, 1.0, 1.0)),
        sph(0.5), box(0.2f, 1.0f, 0.5f), suelo(4, 4){};
  void setup(void) override;
  void render(void) override;
  void reshape(uint w, uint h) override;

private:
  std::shared_ptr<GLMatrices> mats;
  std::shared_ptr<Material> plastico_rojo, plastico_verde, goma_azul, perla;
  std::shared_ptr<UBOLightSources> luces;

  Program gshader;
  Cylinder cone, luz;
  Sphere sph;
  Box box;
  Rect suelo;
  LightSourceParameters lp;
};

void MyRender::setup() {
  glClearColor(.7f, .7f, .7f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  /* Este shader se encarga de calcular la iluminación, usando
  el algoritmo de Gouraud*/
  gshader.addAttributeLocation(Mesh::VERTICES, "position");
  gshader.addAttributeLocation(Mesh::NORMALS, "normal");

  plastico_rojo = PGUPV::getMaterial(PredefinedMaterial::RED_PLASTIC);
  plastico_verde = getMaterial(PredefinedMaterial::GREEN_PLASTIC);
  goma_azul = PGUPV::getMaterial(PredefinedMaterial::CYAN_RUBBER);
  perla = PGUPV::getMaterial(PredefinedMaterial::PEARL);

  mats = GLMatrices::build();
  gshader.connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);

  luces = UBOLightSources::build();
  gshader.connectUniformBlock(luces, UBO_LIGHTS_BINDING_INDEX);
  gshader.replaceString("$" + UBOMaterial::blockName, UBOMaterial::definition);
  gshader.loadFiles(App::examplesDir() + "ej6-1/ej6-1");
  gshader.compile();


  // Definición de los parámetros de la fuente
  lp.ambient = vec4(.3, .3, .3, 1.0);
  lp.diffuse = vec4(0.5, 0.5, 0.5, 1.0);
  lp.specular = vec4(0.8, 0.8, 0.8, 1.0);
  // .x: spotExponent, .y: spotCutoff (degrees), .z: spotCosCutoff
  lp.spotExponent = 50;
  lp.spotCutoff = CONE_ANGULAR_RADIUS;
  lp.spotCosCutoff = (float)cos(M_PI * CONE_ANGULAR_RADIUS / 180.0);
  // .x: constant, .y: linear, .z: quadratic
  lp.attenuation = vec3(1.0, 0.0, 0.0);
  lp.positionWorld = vec4(1.0, 1.0, 1.0, 1.0);

  luces->setLightSource(0, lp);

  setCameraHandler(std::make_shared<OrbitCameraHandler>());
}

mat4 buildR(vec3 t) {
  float h = 1.0f / (1.0f + t.y);

  glm::mat4 m(t.y + h * t.z * t.z, -t.x, -h * t.z * t.x, 0.0f, t.x, t.y, t.z,
              0.0f, -h * t.z * t.x, -t.z, t.y + h * t.x * t.x, 0.0f, 0.0f, 0.0f,
              0.0f, 1.0f);
  return m;
}

void MyRender::render() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  mat4 viewMatrix = getCamera().getViewMatrix();
  mats->setMatrix(GLMatrices::VIEW_MATRIX, viewMatrix);

  // Objetos con iluminación y textura
  gshader.use();

  if (lp.directional)
    lp.positionEye = viewMatrix * vec4(vec3(lp.positionWorld), 0.0);
  else
    lp.positionEye = viewMatrix * lp.positionWorld;
  // El foco apunta al origen
  lp.spotDirectionEye = -vec3(lp.positionEye);
  luces->setLightSource(0, lp);

  plastico_rojo->use();

  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, 1.0, 0.0, -0.5);
  cone.render();

  plastico_verde->use();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);
  sph.render();

  goma_azul->use();
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, -0.75f, 0.0f, 0.4f);
  mats->rotate(GLMatrices::MODEL_MATRIX, glm::radians(150.0f), 0.0, 1.0, 0.0);
  box.render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  // Suelo
  perla->use();
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, 0.0, -0.5001f, 0.0);
  mats->rotate(GLMatrices::MODEL_MATRIX, glm::radians(-90.0f), 1.0, 0.0, 0.0);
  suelo.render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  // Luz
  ConstantIllumProgram::use();
  mats->pushMatrix(GLMatrices::MODEL_MATRIX);
  mats->translate(GLMatrices::MODEL_MATRIX, vec3(lp.positionWorld));
  mats->multMatrix(GLMatrices::MODEL_MATRIX,
                   buildR(glm::normalize(vec3(lp.positionWorld))));
  mats->translate(GLMatrices::MODEL_MATRIX, vec3(0.0, -CONE_HEIGHT / 2, 0.0));
  luz.render();
  mats->popMatrix(GLMatrices::MODEL_MATRIX);

  CHECK_GL();
}

void MyRender::reshape(uint w, uint h) {
  glViewport(0, 0, w, h);
  mats->setMatrix(GLMatrices::PROJ_MATRIX, getCamera().getProjMatrix());
  mats->loadIdentity(GLMatrices::MODEL_MATRIX);
}

int main(int argc, char *argv[]) {
  App &myApp = App::getInstance();
  myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER |
                                PGUPV::MULTISAMPLE);
  myApp.getWindow().setRenderer(std::make_shared<MyRender>());
  return myApp.run();
}
