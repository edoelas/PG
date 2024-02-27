

#ifndef _SCENE_FILE_H
#define _SCENE_FILE_H 2011

#include <string>
#include <map>
#include <vector>
#include <memory>


#include "fileFormats.h"


namespace PGUPV {
  class Material;
  class PBRMaterial;
  class UBOMaterial;
  class BindableTexture;
  class Scene;
  class Node;
  class Mesh;
  class Skeleton;

  class AssimpWrapper {
  public:
    AssimpWrapper();
    ~AssimpWrapper();
    /**
      Opciones de postprocesado de una escena:
      NONE: no se aplica ningún postprocesado a la escena
      FAST: triangula la escena, genera normales y tangentes...
      MEDIUM: igual que el anterior, pero intentando reducir memoria
          reutilizando materiales
      HIGHEST_QUALITY: igual que el anterior, pero buscando instancias
    */
    enum class LoadOptions {
      NONE, FAST, MEDIUM, HIGHEST_QUALITY
    };

    /**
    Carga una escena desde el fichero indicado
    \param filename ruta del fichero a cargar
    \param options Postprocesado a realizar sobre la escena (por defecto,
      LoadOptions::MEDIUM)
    */
    std::shared_ptr<Scene> load(const std::string &filename, LoadOptions options = LoadOptions::MEDIUM);

	std::vector<ExportFileFormat> listSupportedExportFormat();

	/**
	Carga una escena desde el fichero indicado
	\param filename ruta del fichero a cargar
	\param options Postprocesado a realizar sobre la escena (por defecto,
	  LoadOptions::MEDIUM)
	*/
	bool save(const std::string &path, const std::string &id, std::shared_ptr<Scene> scene);

  private:
	class AssimpWrapperImpl;
	std::unique_ptr<AssimpWrapperImpl> impl;
  };
};

#endif
