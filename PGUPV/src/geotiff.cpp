#include "geoTiff.h"

using PGUPV::GeoTiffMetadata;

glm::mat4 GeoTiffMetadata::rasterToModel() const {
	glm::mat4 m{ 1.0f };
	if (modelType == ProjectionType::ModelTypeProjected) {
		m[0][0] = static_cast<float>(modelPixelScale.x);
		m[1][1] = static_cast<float>(-modelPixelScale.y);
		m[2][2] = static_cast<float>(modelPixelScale.z);
		m[3][0] = static_cast<float>(modelTiePoint[0].modelPoint.x - modelTiePoint[0].imagePoint.x * modelPixelScale.x);
		m[3][1] = static_cast<float>(modelTiePoint[0].modelPoint.y + modelTiePoint[0].imagePoint.y * modelPixelScale.y);
		m[3][2] = static_cast<float>(modelTiePoint[0].modelPoint.z - modelTiePoint[0].imagePoint.z * modelPixelScale.z);
	}
	return  m;
}

std::vector<glm::vec3> GeoTiffMetadata::corners() const
{
	double imageWorldWidth, imageWorldHeight;
	if (rasterPixelIsArea) {
		imageWorldWidth = modelPixelScale.x * widthPixels;
		imageWorldHeight = modelPixelScale.y * heightPixels;
	}
	else {
		imageWorldWidth = modelPixelScale.x * (widthPixels - 1);
		imageWorldHeight = modelPixelScale.y * (heightPixels - 1);
	}
	glm::mat4 m = rasterToModel();
	std::vector<glm::vec3> cornerVtcs;
	cornerVtcs.push_back(m * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
	cornerVtcs.push_back(m * glm::vec4(0.0f, imageWorldHeight, 0.0f, 1.0f));
	cornerVtcs.push_back(m * glm::vec4(imageWorldWidth, imageWorldHeight, 0.0f, 1.0f));
	cornerVtcs.push_back(m * glm::vec4(imageWorldWidth, 0.0f, 0.0f, 1.0f));
	return cornerVtcs;
}

PGUPV::BoundingBox GeoTiffMetadata::getBB() const
{
	return PGUPV::computeBoundingBox(corners());
}


using PGUPV::GeoTiffMetadata;

void PGUPV::processGeoKeyDirectory(const PGUPV::GeoKeyDirectory& gkd, 
	const uint16_t* geoShortParams, const double* geoDoubleParams, const char* asciiParams, 
	GeoTiffMetadata& md) {

	for (const auto& key : gkd.keys) {
		switch (key.keyID) {
		case 1024: // GTModelTypeGeoKey
			if (key.index == 1) md.modelType = GeoTiffMetadata::ProjectionType::ModelTypeProjected;
			else if (key.index == 2) md.modelType = GeoTiffMetadata::ProjectionType::ModelTypeGeographic;
			else if (key.index == 3) md.modelType = GeoTiffMetadata::ProjectionType::ModelTypeGeocentric;
			break;
		case 1025: // GTRasterTypeGeoKey
			md.rasterPixelIsArea = key.index == 1;
			break;
		case 1026: // GTCitationGeoKey
			if (asciiParams != nullptr) {
				md.citation = std::string(asciiParams + key.index, asciiParams + key.index + key.count - 1);
			}
			break;
		case 2048: // GeographicTypeGeoKey
			switch (key.index) {
			case 4258:
				md.geodeticCRSGeoKey = GeoTiffMetadata::GeodeticCRSGeoKey::GCS_EUREF89; break;
			default:
				md.geodeticCRSGeoKey = GeoTiffMetadata::GeodeticCRSGeoKey::Unknown; break;
			}
			break;
		case 2049: // GeogCitationGeoKey
			break;
		case 2050: // GeogGeodeticDatumGeoKey
			break;
		case 2051: // GeogPrimeMeridianGeoKey
			break;
		case 2052: // GeogLinearUnitsGeoKey
			break;
		case 2053: // GeogLinearUnitSizeGeoKey
			break;
		case 2054: // GeogAngularUnitsGeoKey
			switch (key.index) {
			case 9101: md.geoAngularUnitsGeoKey = GeoTiffMetadata::GeoAngularUnitsGeoKey::Angular_Radian; break;
			case 9102: md.geoAngularUnitsGeoKey = GeoTiffMetadata::GeoAngularUnitsGeoKey::Angular_Degree; break;
			default: md.geoAngularUnitsGeoKey = GeoTiffMetadata::GeoAngularUnitsGeoKey::Unknown; break;
			}
			break;
		case 2055: // GeogAngularUnitSizeGeoKey
			break;
		case 2056: // GeogEllipsoidGeoKey
			switch (key.index) {
			case 7019: md.geogEllipsoidGeoKey = GeoTiffMetadata::GeogEllipsoidGeoKey::Ellipse_GRS_1980; break;
			default: md.geogEllipsoidGeoKey = GeoTiffMetadata::GeogEllipsoidGeoKey::Unknown; break;
			}
			break;
		case 2057: // GeogSemiMajorAxisGeoKey
			assert(key.tagLocation == 34736 && key.count == 1 && geoDoubleParams != nullptr);
			md.semiMajorAxis = geoDoubleParams[key.index];
			break;
		case 2058: // GeogSemiMinorAxisGeoKey
			assert(key.tagLocation == 34736 && key.count == 1 && geoDoubleParams != nullptr);
			md.semiMinorAxis = geoDoubleParams[key.index];
			break;
		case 2059: // GeogInvFlatteningGeoKey
			assert(key.tagLocation == 34736 && key.count == 1 && geoDoubleParams != nullptr);
			md.invFlattening = geoDoubleParams[key.index];
			break;
		case 2060: // GeogAzimuthUnitsGeoKey
			break;
		case 2061: // GeogPrimeMeridianLongGeoKey
			break;
		case 2062: // GeogTOWGS84GeoKey
			break;
		case 3072: // ProjectedCSTypeGeoKey
			switch (key.index) {
			case 25830: md.projectedCSTypeGeoKey = GeoTiffMetadata::ProjectedCSTypeGeoKey::ETRS89_UTM_30N; break;
			default: md.projectedCSTypeGeoKey = GeoTiffMetadata::ProjectedCSTypeGeoKey::Unknown; break;
			}
			break;
		case 3073: // PCSCitationGeoKey
			if (asciiParams != nullptr) {
				md.projectedCSTypeGeoCitation = std::string(asciiParams + key.index, asciiParams + key.index + key.count - 1);
			}
			break;
		case 3074: // ProjectionGeoKey
			break;
		case 3075: // ProjCoordTransGeoKey
			break;
		case 3076: // ProjLinearUnitsGeoKey
			switch (key.index) {
			case 9001: md.projectedCSLinearUnitsGeoKey = GeoTiffMetadata::LinearUnits::Linear_Meter; break;
			default: md.projectedCSLinearUnitsGeoKey = GeoTiffMetadata::LinearUnits::Unknown; break;
			}
			break;
		case 3077: // ProjLinearUnitSizeGeoKey
			break;
		case 3078: // ProjStdParallel1GeoKey
			break;
		case 3079: // ProjStdParallel2GeoKey
			break;
		case 3080: // ProjNatOriginLongGeoKey
			break;
		case 3081: // ProjNatOriginLatGeoKey
			break;
		case 3082: // ProjFalseEastingGeoKey
			break;
		case 3083: // ProjFalseNorthingGeoKey
			break;
		case 3084: // ProjFalseOriginLongGeoKey
			break;
		case 3085: // ProjFalseOriginLatGeoKey
			break;
		case 3086: // ProjFalseOriginEastingGeoKey
			break;
		case 3087: // ProjFalseOriginNorthingGeoKey
			break;
		case 3088: // ProjCenterLongGeoKey
			break;
		case 3089: // ProjCenterLatGeoKey
			break;
		case 3090: // ProjCenterEastingGeoKey
			break;
		case 3091: // ProjCenterNorthingGeoKey
			break;
		case 3092: // ProjScaleAtNatOriginGeoKey
			break;
		case 3093: // ProjScaleAtCenterGeoKey
			break;
		case 3094: // ProjAzimuthAngleGeoKey
			break;
		case 3095: // ProjStraightVertPoleLongGeoKey
			break;
		case 4096: // VerticalCSTypeGeoKey
			break;
		case 4097: // VerticalCitationGeoKey
			break;
		case 4098: // VerticalDatumGeoKey
			break;
		case 4099: // VerticalUnitsGeoKey
			switch (key.index) {
			case 9001: md.verticalUnitsGeoKey = GeoTiffMetadata::LinearUnits::Linear_Meter; break;
			default: md.verticalUnitsGeoKey = GeoTiffMetadata::LinearUnits::Unknown; break;
			}
			break;
		}
	}
}
