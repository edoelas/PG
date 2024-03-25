#pragma once

#include <vector>
#include <string>
#include <map>

#include "media.h"

namespace media {
	class VideoDevice : public Media {
	public:
		// Abre la c�mara con la configuraci�n indicada
		VideoDevice(unsigned int camId, unsigned int optsId = 0, float fps = MAX_FPS);
		~VideoDevice();

		// Constante que indica usar la velocidad mayor
		static const float MAX_FPS;

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
			Devuelve una lista con el nombre de las c�maras disponibles
		*/
		static std::vector<CameraInfo> getAvailableCameras();

	private:
		static bool libavInitialized;
		static void initializeLibAv();

		void init(const CameraInfo &ci, size_t optionId);
		void openDevice(const CameraInfo& ci, size_t optionId);
	};
};
