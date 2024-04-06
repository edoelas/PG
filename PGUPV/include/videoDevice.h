#pragma once

#include <vector>
#include <string>
#include <map>

#include "media.h"

namespace media {
	class VideoDevice : public Media {
	public:
		// Abre la cámara con la configuración indicada
		VideoDevice(unsigned int camId, unsigned int optsId = 0);
		~VideoDevice();

		struct VideoFormat {
			uint32_t width;     ///< width in pixels
			uint32_t height;    ///< height in pixels
			std::string fourcc;    ///< fourcc code (platform dependent)
			uint32_t fps;       ///< frames per second
			uint32_t bpp;       ///< bits per pixel
		};

		struct CameraInfo {
			std::string name;
			std::string devicePath;
			std::vector<VideoFormat> formats;
		};

		/**
			Devuelve una lista con el nombre de las cámaras disponibles
		*/
		static std::vector<CameraInfo> getAvailableCameras();

	private:
		static bool libavInitialized;
		static void initializeLibAv();

		void init(const CameraInfo &ci, size_t optionId);
		void openDevice(const CameraInfo& ci, size_t optionId);
	};
};
