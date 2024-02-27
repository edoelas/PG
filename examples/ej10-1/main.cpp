#include <glm/gtc/matrix_transform.hpp>

#include "PGUPV.h"
#include "GUI3.h"


/*

Ejemplo de Physically Based Rendering con luces puntuales y materiales.

*/

using namespace PGUPV;

struct MyPBRMaterialWidget {
	MyPBRMaterialWidget(std::shared_ptr<Panel> panel, const std::string& label, const glm::vec3& albedoColor,
		float metallicValue, float roughnessValue, float ambientOcclusionValue) {
		panel->addWidget(std::make_shared<Label>(label));

		albedo = std::make_shared<ColorWidget<glm::vec3>>("Albedo##" + label, albedoColor);
		panel->addWidget(albedo);

		metallic = std::make_shared<FloatSliderWidget>("Metallic##" + label, metallicValue, 0.0f, 1.0f);
		panel->addWidget(metallic);

		roughness = std::make_shared<FloatSliderWidget>("Roughness##" + label, roughnessValue, 0.0f, 1.0f);
		panel->addWidget(roughness);

		ao = std::make_shared<FloatSliderWidget>("AO##" + label, ambientOcclusionValue, 0.0f, 1.0f);
		panel->addWidget(ao);
	}

	std::shared_ptr<RGBColorWidget> albedo;
	std::shared_ptr<FloatSliderWidget> metallic, roughness, ao;
};

class MyRender : public Renderer {
public:
	MyRender() : axes(1.2f), sphere(0.5f, 20, 40), luminaire(.08f, 10, 10, glm::vec4(1.0, 1.0, 0.0, 1.0)) {};
	void setup(void) override;
	void render(void) override;
	void reshape(uint w, uint h) override;

private:
	void buildGUI();
	Axes axes;
	std::shared_ptr<Program> pbr_program;
	std::shared_ptr<GLMatrices> mats;
	std::shared_ptr<UBOPBRLightSources> lights;

	Sphere sphere, luminaire;
	std::shared_ptr<Scene> tetera;

	GLint albedoLoc{ -1 }, metallicLoc{ -1 }, roughnessLoc{ -1 }, AOLoc{ -1 };
	void setMaterialUniforms(const MyPBRMaterialWidget& widget);
	std::shared_ptr<MyPBRMaterialWidget> teapotMaterial, sphereMaterial;
};

void MyRender::setMaterialUniforms(const MyPBRMaterialWidget& widget) {
	auto color = glm::pow(widget.albedo->getColor(), glm::vec3(2.2f));
	glUniform3fv(albedoLoc, 1, &color.x);
	glUniform1f(roughnessLoc, widget.roughness->get());
	glUniform1f(metallicLoc, widget.metallic->get());
	glUniform1f(AOLoc, widget.ao->get());
}


void MyRender::setup() {
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glEnable(GL_DEPTH_TEST);

	// Construímos el objeto GLMatrices
	pbr_program = std::make_shared<Program>();
	pbr_program->addAttributeLocation(Mesh::VERTICES, "position");
	pbr_program->addAttributeLocation(Mesh::NORMALS, "normal");
	pbr_program->addAttributeLocation(Mesh::TEX_COORD0, "texCoords");

	lights = UBOPBRLightSources::build();
	pbr_program->connectUniformBlock(lights, UBO_PBR_LIGHTS_BINDING_INDEX);
	mats = GLMatrices::build();
	pbr_program->connectUniformBlock(mats, UBO_GL_MATRICES_BINDING_INDEX);
	pbr_program->loadFiles(App::examplesDir() + "ej10-1/pbr");
	pbr_program->compile();
	albedoLoc = pbr_program->getUniformLocation("albedo");
	metallicLoc = pbr_program->getUniformLocation("metallic");
	roughnessLoc = pbr_program->getUniformLocation("roughness");
	AOLoc = pbr_program->getUniformLocation("ao");

	tetera = FileLoader::load(App::assetsDir() + "models/teapot.3ds");

	for (int i = 0; i < 4; i++) {
		auto l = lights->getLightSource(i);
		l.color = glm::vec3(1.0f, 1.0f, 1.0f);
		l.intensity = 20.0f + i * 5;
		l.scaledColor = l.color * l.intensity;
		if (i == 0) l.positionWorld = glm::vec4(-10.0f, 10.0f, 10.0f, 1.0f);
		else if (i == 1) l.positionWorld = glm::vec4(10.0f, 10.0f, 10.0f, 1.0f);
		else if (i == 2) l.positionWorld = glm::vec4(-10.0f, -10.0f, 10.0f, 1.0f);
		else l.positionWorld = glm::vec4(10.0f, -10.0f, 10.0f, 1.0f);
		l.enabled = true;
		lights->setLightSource(i, l);
	}

	setCameraHandler(std::make_shared<OrbitCameraHandler>());
	buildGUI();
	App::getInstance().getWindow().showGUI(true);
}

void MyRender::render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	auto viewMatrix = getCameraHandler()->getCamera().getViewMatrix();
	mats->setMatrix(GLMatrices::VIEW_MATRIX, viewMatrix);
	lights->updateLightEyeSpacePosition(viewMatrix);

	pbr_program->use();

	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->scale(GLMatrices::MODEL_MATRIX, 0.5f);
	mats->translate(GLMatrices::MODEL_MATRIX, -tetera->getBB().getCenter() + glm::vec3(-2.0f, 0.0f, 0.0f));
	setMaterialUniforms(*teapotMaterial);
	tetera->render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);

	mats->pushMatrix(GLMatrices::MODEL_MATRIX);
	mats->translate(GLMatrices::MODEL_MATRIX, 1.0f, 0.0f, 0.0f);
	setMaterialUniforms(*sphereMaterial);
	sphere.render();
	mats->popMatrix(GLMatrices::MODEL_MATRIX);


	ConstantUniformColorProgram::use();
	for (int i = 0; i < NUM_LIGHT_SOURCES; i++) {
		if (lights->getLightSource(i).enabled == 0) continue;
		mats->loadIdentity(GLMatrices::MODEL_MATRIX);
		mats->translate(GLMatrices::MODEL_MATRIX, lights->getLightSource(i).positionWorld);
		ConstantUniformColorProgram::setColor(glm::vec4(lights->getLightSource(i).color, 1.0));
		luminaire.render();
		mats->loadIdentity(GLMatrices::MODEL_MATRIX);
	}
}

void MyRender::reshape(uint w, uint h) {
	glViewport(0, 0, w, h);
	if (h == 0)
		h = 1;
	float aspectRatio = (float)w / h;
	mats->setMatrix(GLMatrices::PROJ_MATRIX,
		glm::perspective(glm::radians(60.0f), aspectRatio, .1f, 30.0f));
}


void MyRender::buildGUI() {
	auto panel = addPanel("Luces");
	// Tamaño del panel y posición del panel
	panel->setSize(260, 300);
	panel->setPosition(530, 10);

	teapotMaterial = std::make_shared<MyPBRMaterialWidget>(panel, "Tetera", glm::vec3(0.7f, 0.0f, 0.0f), 0.7f, 0.2f, 0.2f);


	panel->addWidget(std::make_shared<Separator>());
	sphereMaterial = std::make_shared<MyPBRMaterialWidget>(panel, "Esfera", glm::vec3(0.2f, 0.7f, 0.1f), 0.2f, 0.7f, 0.2f);

	panel->addWidget(std::make_shared<Separator>());
	auto lightProp = std::make_shared<PBRLightSourceWidget>(lights, 20.0f);
	panel->addWidget(lightProp);

}

int main(int argc, char* argv[]) {
	App& myApp = App::getInstance();
	myApp.initApp(argc, argv, PGUPV::DOUBLE_BUFFER | PGUPV::DEPTH_BUFFER);
	myApp.getWindow().setRenderer(std::make_shared<MyRender>());
	return myApp.run();
}
