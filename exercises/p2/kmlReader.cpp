
#include <pugixml.hpp>
#include <sstream>
#include <map>
#include <algorithm>
#include <glm/common.hpp>

#include "kmlReader.h"
#include "UTM.h"

KMLFile readNeighborhood(const std::string& fname)
{
	KMLFile result;

	pugi::xml_document doc;
	if (!doc.load_file(fname.c_str())) return result;

	pugi::xml_node root = doc.child("kml");
	auto document = root.child("Document");
	std::string coordinateSystem;

	for (pugi::xml_node schema : document.children("Schema")) {
		for (pugi::xml_node simpleField : schema.children("SimpleField")) {
			KMLAttributeDefinition def;
			auto id = simpleField.attribute("name").value();
			auto type = simpleField.attribute("type").value();
			if (type == std::string("string")) {
				def.type = KMLValueType::STRING;
			}
			else if (type == std::string("double")) {
				def.type = KMLValueType::DOUBLE;
			}
			else {
				assert(0);
			}
			def.title = simpleField.children("displayName").begin()->child_value();
			result.attributes[id] = def;
		}
	}

	for (pugi::xml_node placemark : document.children("Placemark")) {
		KMLPlacemark pm;
		pm.id = placemark.child("name").child_value();
		for (pugi::xml_node data : placemark.children("ExtendedData")) {
			for (pugi::xml_node schema : data.children("SchemaData")) {
				for (pugi::xml_node dat : schema.children("SimpleData")) {
					auto id = dat.attribute("name").value();
					auto value = dat.child_value();
					auto it = result.attributes.find(id);
					if (it != result.attributes.end()) {
						KMLValue attr;
						if (it->second.type == KMLValueType::STRING) {
							attr = std::string(value);
						}
						else if (it->second.type == KMLValueType::DOUBLE) {
							attr = std::stod(value);
						}
						pm.attributes[id] = attr;
					}
				}
			}
		}
		for (pugi::xml_node point : placemark.children("Polygon")) {
			KMLPolygon poly;
			for (pugi::xml_node outer : point.children("outerBoundaryIs")) {
				for (pugi::xml_node lr : outer.children("LinearRing")) {
					for (pugi::xml_node pList : lr.children("coordinates")) {
						auto coords = pList.child_value();
						std::stringstream ss(coords);
						double x, y;
						char comma;
						while (ss >> x >> comma >> y) {
							double utmx, utmy;
							LatLonToUTMXY(y, x, 30, utmx, utmy);
							poly.outerBoundary.push_back(glm::dvec2{ utmx, utmy });
						}
					}
				}
			}
			pm.geometry.push_back(poly);
		}
		result.placemarks.push_back(pm);
	}
	std::sort(result.placemarks.begin(), result.placemarks.end(), [&result](const auto& a, const auto& b) {
		std::string aName, bName;
		auto it = a.attributes.find("nombre");
		if (it != a.attributes.end()) {
			aName = std::get<std::string>(it->second);
		}
		else {
			aName = a.id;
		}
			
		it = b.attributes.find("nombre");
		if (it != b.attributes.end()) {
			bName = std::get<std::string>(it->second);
		}
		else {
			bName = b.id;
		}
		return aName < bName;
		});
	return result;
}
