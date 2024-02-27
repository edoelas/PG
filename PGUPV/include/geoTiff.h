#pragma once
#include "boundingVolumes.h"
#include "optional"
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace PGUPV {

	/**
	 * @brief Sólo para imágenes en formato GeoTIFF
	 * https://docs.ogc.org/is/19-008r4/19-008r4.html
	*/
	struct GeoTiffMetadata {
		enum class ProjectionType {
			Unknown,
			ModelTypeProjected,   /* Projection Coordinate System         */
			ModelTypeGeographic,   /* Geographic latitude-longitude System */
			ModelTypeGeocentric,   /* Geocentric (X,Y,Z) Coordinate System */
		};
		ProjectionType modelType{ ProjectionType::Unknown };
		glm::dvec3 modelPixelScale{ 1.0 };
		bool rasterPixelIsArea{ false }; // false = rasterPixelIsPoint, true = rasterPixelIsArea
		std::string citation;
		enum class GeodeticCRSGeoKey { // A geographic CS is a datum (origin, scale and orientation) and a prime meridian
			Unknown,	// There are a lot. See http://geotiff.maptools.org/spec/geotiff6.html#6.3.2.1
			GCS_EUREF89 // European Terrestrial Reference System 1989
		};
		GeodeticCRSGeoKey  geodeticCRSGeoKey{ GeodeticCRSGeoKey::Unknown };
		struct TiePoint {
			glm::vec3 imagePoint;
			glm::vec3 modelPoint;
		};
		enum class GeoAngularUnitsGeoKey {
			Unknown,		// http://geotiff.maptools.org/spec/geotiff6.html#6.3.1.4
			Angular_Degree,
			Angular_Radian
		};
		GeoAngularUnitsGeoKey geoAngularUnitsGeoKey{ GeoAngularUnitsGeoKey::Unknown };

		enum class GeogEllipsoidGeoKey {
			Unknown,		// http://geotiff.maptools.org/spec/geotiff6.html#6.3.2.3
			Ellipse_GRS_1980
		};
		GeogEllipsoidGeoKey geogEllipsoidGeoKey{ GeogEllipsoidGeoKey::Unknown };
		double semiMajorAxis{ 0.0 }, semiMinorAxis{ 0.0 }, invFlattening{ 0.0 };
		enum class ProjectedCSTypeGeoKey {
			Unknown,   // http://geotiff.maptools.org/spec/geotiff6.html#6.3.3.1
			ETRS89_UTM_30N // https://spatialreference.org/ref/epsg
		};
		ProjectedCSTypeGeoKey projectedCSTypeGeoKey{ ProjectedCSTypeGeoKey::Unknown };
		std::string projectedCSTypeGeoCitation;

		enum class LinearUnits {
			Unknown,	// http://geotiff.maptools.org/spec/geotiff6.html#6.3.1.3
			Linear_Meter
		};
		LinearUnits projectedCSLinearUnitsGeoKey{ LinearUnits::Unknown };
		LinearUnits verticalUnitsGeoKey{ LinearUnits::Unknown };
		std::vector<TiePoint> modelTiePoint;


		/**
		* @brief Returns the transformation matrix from raster to world coordinates
		* @return
		*/
		glm::mat4 rasterToModel() const;

		/**
		 * @brief Returns the corners of the image in world coordinates (it can be drawn
		 * with a triangle fan)
		 * @return 
		*/
		std::vector<glm::vec3> corners() const;
		/**
		 * @brief Returns the bounding box of the image in world coordinates
		 * @return
		*/
		BoundingBox getBB() const;
		uint32_t widthPixels{ 0 }, heightPixels{ 0 };
	};


	struct GeoKeyDirectory {
		struct Header {
			uint16_t dirVersion, // should be 1
				keyRevision, // should be 1
				minorRevision, // should be 0 or 1
				numKeys; // number of keys in directory
		} header;
		struct Key {
			uint16_t keyID, // see below
				tagLocation, // if 0, then the value is SHORT, contained in the "Value_Offset" entry.
				// Otherwise, the type is implied by the TIFF - Type of the tag
				// containing the value
				count, // number of values in this key
				index; // if tagLocation != 0, index into the TagArray indicated by tagLocation
			//  if tagLocation == 0, then contains the actual (SHORT) value of the Key, and
			// count = 1 is implied
		};
		std::vector<Key> keys;
	};

	/**
	 * @brief Given the geokey directory, the double and ascii parameters, fills the GeoTiffMetadata
	*/
	void processGeoKeyDirectory(
		const GeoKeyDirectory& gkd, 
		const uint16_t *geoShortParams, const double* geoDoubleParams,
		const char* asciiParams, GeoTiffMetadata& md);
}