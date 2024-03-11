
#include <pugixml.hpp>
#include <sstream>
#include <map>
#include <algorithm>
#include <glm/common.hpp>

#include "gmlReader.h"


std::vector<glm::dvec2> parseCoordinates(const char* vals) {
	std::vector<glm::dvec2> result;
	std::stringstream ss(vals);
	double x, y;
	while (ss >> x >> y) {
		result.emplace_back(glm::dvec2{ x, y });
	}
	return result;
}

void addBuildingPart(std::map<std::string, Building>& c, const std::string& buildingId, const BuildingPart& bp) {
	glm::dvec2 bmin, bmax;
	bmin = glm::vec2{ std::numeric_limits<float>::max() };
	bmax = glm::vec2{ std::numeric_limits<float>::lowest() };
	for (const auto& p : bp.exterior) {
		bmin = glm::min(bmin, p);
		bmax = glm::max(bmax, p);
	}

	if (c.find(buildingId) == c.end()) {
		Building b;
		b.id = buildingId;
		b.min = bmin;
		b.max = bmax;
		b.parts.push_back(bp);
		c[buildingId] = b;
	}
	else {
		auto& b = c[buildingId];
		b.min = glm::min(b.min, bmin);
		b.max = glm::max(b.max, bmax);
		b.parts.push_back(bp);
	}
}

City readBuildings(const std::string& fname, bool loadInteriors) {
	std::map<std::string, Building> buildings;

	pugi::xml_document doc;
	if (!doc.load_file(fname.c_str())) return City();

	pugi::xml_node root = doc.child("gml:FeatureCollection");
	std::string coordinateSystem;

	for (pugi::xml_node building : root.children("gml:featureMember")) {
		for (pugi::xml_node bpart : building.children("bu-ext2d:BuildingPart")) {
			BuildingPart buildingPart;
			std::string gid;
			gid = bpart.attribute("gml:id").value();
			auto lastDot = gid.find_last_of(".");
			auto unders = gid.find("_");
			auto buildingId = gid.substr(lastDot + 1, unders - lastDot - 1);
			buildingPart.id = gid.substr(unders + 1);

			for (pugi::xml_node geom : bpart.children("bu-ext2d:geometry")) {
				for (pugi::xml_node bg : geom.children("bu-core2d:BuildingGeometry")) {
					for (pugi::xml_node bcg : bg.children("bu-core2d:geometry")) {
						for (pugi::xml_node srf : bcg.children("gml:Surface")) {
							auto crs = srf.attribute("srsName").value();
							if (coordinateSystem.empty()) {
								coordinateSystem = crs;
							}
							else {
								assert(coordinateSystem == crs);
							}
							auto lastDot = gid.find_last_of(".");
							auto unders = gid.find("_");
							//std::cout << gid << "\n";
							auto buildingId = gid.substr(lastDot + 1, unders - lastDot - 1);

							for (pugi::xml_node patches : srf.children("gml:patches")) {
								for (pugi::xml_node polpatches : patches.children("gml:PolygonPatch")) {
									for (pugi::xml_node exterior : polpatches.children("gml:exterior")) {
										for (pugi::xml_node lr : exterior.children("gml:LinearRing")) {
											for (pugi::xml_node pList : lr.children("gml:posList")) {
												auto dim = std::stoi(pList.attribute("srsDimension").value());
												auto count = std::stoi(pList.attribute("count").value());
												if (dim != 2) {
													assert(0); // "dim != 2\n";
												}
												else {
													buildingPart.exterior = parseCoordinates(pList.child_value());
												}
											}
										}
									}
									if (loadInteriors) {
										for (pugi::xml_node interior : polpatches.children("gml:interior")) {
											for (pugi::xml_node lr : interior.children("gml:LinearRing")) {
												for (pugi::xml_node pList : lr.children("gml:posList")) {
													auto dim = std::stoi(pList.attribute("srsDimension").value());
													auto count = std::stoi(pList.attribute("count").value());

													if (dim != 2) {
														assert(0); // std::cerr << "dim != 2\n";
													}
													else {
														buildingPart.interior.push_back(parseCoordinates(pList.child_value()));
													}

												}
											}
										}
									}
								}
							}
						}
					}

				}
			}

			for (pugi::xml_node floors : bpart.children("bu-ext2d:numberOfFloorsAboveGround")) {
				buildingPart.floors = std::stoi(floors.child_value());
			}
			float undergroundHeight = 0.0f;
			for (pugi::xml_node heightBelow : bpart.children("bu-ext2d:heightBelowGround")) {
				undergroundHeight = std::stof(heightBelow.child_value());
			}
			if (undergroundHeight > 0.0f) {
				buildingPart.undergroundHeight = undergroundHeight;
			}
			else {
				int undergroundFloors = 0;
				for (pugi::xml_node floorsBelow : bpart.children("bu-ext2d:numberOfFloorsBelowGround")) {
					undergroundFloors = std::stoi(floorsBelow.child_value());
				}
				if (undergroundFloors > 0) {
					buildingPart.undergroundHeight = undergroundFloors * 3.0f;
				}
			}

			addBuildingPart(buildings, buildingId, buildingPart);
		}
	}

	City result;
	result.min = glm::vec2{ std::numeric_limits<float>::max() };
	result.max = glm::vec2{ std::numeric_limits<float>::lowest() };
	result.srs = coordinateSystem;
	for (const auto& kv : buildings) {
		result.min = glm::min(result.min, kv.second.min);
		result.max = glm::max(result.max, kv.second.max);
		result.buildings.push_back(kv.second);
	}
	return result;
}
