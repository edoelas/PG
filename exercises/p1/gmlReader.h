#pragma once

#include <vector>
#include <string>
#include <glm/vec2.hpp>


/**
 * @brief Clase de utilidad para leer la información de los edificios de un fichero GML, 
 con el formato del Catastro Español: 
 
 https://www.catastro.minhap.es/webinspire/documentos/Conjuntos%20de%20datos.pdf
 .
*/

/**
 * @brief Estructura que representa una parte de un edificio.
 */
struct BuildingPart {
	std::string id; // id de la parte del edificio
	std::vector<glm::dvec2> exterior; // polilínea exterior, en el sentido horario
	std::vector<std::vector<glm::dvec2>> interior; // polilíneas interiores, en el sentido antihorario. Son los huecos
	float undergroundHeight{ 0.0f }; // altura del edificio bajo el suelo
	uint8_t floors{ 0 }; // número de plantas sobre el suelo
};

/**
 * @brief Estructura que representa un edificio.
 */
struct Building {
	std::string id; // parcela catastral
	glm::dvec2 min{ 0.0f }, max{ 0.0f }; // rectángulo que contiene el edificio
	std::vector<BuildingPart> parts; // partes del edificio
};

/**
 * @brief Estructura que representa una ciudad.
 */
struct City {
	std::vector<Building> buildings;	// conjunto de edificios
	glm::dvec2 min{ 0.0f }, max{ 0.0f }; // rectángulo que contiene la ciudad
	std::string srs;					// sistema de referencia espacial
};

/**
 * @brief Lee un fichero GML con la información de los edificios de una ciudad.
 * @param fname nombre del fichero
 * @param loadInteriors si es true, se cargan los huecos de los edificios
 * @return la información de la ciudad
 */
City readBuildings(const std::string& fname, bool loadInteriors);
