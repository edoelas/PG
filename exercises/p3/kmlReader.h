#pragma once

#include <vector>
#include <string>
#include <glm/vec2.hpp>
#include <variant>
#include <map>


/**
 * @brief Clase de utilidad para leer la información sobre los barrios publicada por el Ayuntamiento de 
  Valencia con el formato KML: 
 
	https://valencia.opendatasoft.com/explore/
 .
*/

// KMLValue es un tipo que puede ser un string o un double
using KMLValue = std::variant<std::string, double>;
// Enumeración para indicar el tipo de valor que contiene un KMLValue
enum class KMLValueType { STRING, DOUBLE};

// Estructura que representa un atributo de un fichero KML
struct KMLAttributeDefinition {
	std::string title;
	enum KMLValueType type;
};

// Estructura que representa los vértices de un polígono de un fichero KML (en UTM 30)
struct KMLPolygon {
	std::vector<glm::dvec2> outerBoundary;
};

// Estructura que representa un lugar de un fichero KML (en este caso, un barrio)
struct KMLPlacemark {
	std::string id;  // id del barrio
	std::map<std::string, KMLValue> attributes; // atributos del barrio
	std::vector<KMLPolygon> geometry; // polígonos que representan el barrio
};

// Estructura que representa un fichero KML
struct KMLFile {
	std::map<std::string, KMLAttributeDefinition> attributes; // título y tipo de los atributos
	std::vector<KMLPlacemark> placemarks; // barrios
};

/**
 * @brief Lee un fichero KML con la información de los barrios de una ciudad.
 * @param fname nombre del fichero
 * @return la información de los barrios
 */
KMLFile readNeighborhood(const std::string& fname);

/**
 * @brief Lee un fichero KML con la información de los baños de una ciudad.
 * @param fname nombre del fichero
 * @return la información de los baños
 */
KMLFile readBathrooms(const std::string& fname);