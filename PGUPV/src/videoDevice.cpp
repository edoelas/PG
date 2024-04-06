extern "C"
{
#ifdef _WIN32
#pragma warning( push)
#pragma warning( disable : 4244)
#endif
#include <libavcodec/avcodec.h>
#include <libavdevice/avdevice.h>
#include <libavutil/dict.h>
#ifdef _WIN32
#pragma warning(pop)
#endif
}

#include <openpnp-capture.h>

#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>



#include "app.h"
#include "videoDevice.h"
#include "log.h"
#include "utils.h"


using media::VideoDevice;
using PGUPV::App;
using namespace std::string_literals;

bool VideoDevice::libavInitialized = false;

VideoDevice::VideoDevice(unsigned int camId, unsigned int optsId) {
	
	auto cams = getAvailableCameras();
	if (camId >= cams.size())
		ERRT("La cámara no existe");
	if (optsId >= cams[camId].formats.size())
		ERRT("La opción de la cámara no existe");
	init(cams[camId], optsId);
	INFO("VideoDevice creado (cámara " + std::to_string(camId) + ") " + std::to_string(reinterpret_cast<std::uint64_t>(this)));
}

VideoDevice::~VideoDevice() {
	INFO("VideoDevice destruído " + std::to_string(reinterpret_cast<std::uint64_t>(this)));
}

void VideoDevice::init(const VideoDevice::CameraInfo &ci, size_t optionId) {
	if (!libavInitialized) {
		initializeLibAv();
	}
	openDevice(ci, optionId);
	if (!searchAudioVideoStreams())
		ERRT("No se ha encontrado el flujo de vídeo de la cámara");
	prepareForReading();
	INFO("Cámara preparada para capturar");
}

std::string FourCCToString(uint32_t fourcc)
{
	std::string v;
	for (uint32_t i = 0; i < 4; i++)
	{
		v += static_cast<char>(fourcc & 0xFF);
		fourcc >>= 8;
	}
	return v;
}

std::vector<VideoDevice::CameraInfo> VideoDevice::getAvailableCameras() {
	
	std::vector<VideoDevice::CameraInfo> result;

	//Cap_installCustomLogFunction(myCustomLogFunction);
	INFO( "OpenPNP Library Version: "s + Cap_getLibraryVersion());
	//Cap_setLogLevel(8);

	CapContext ctx = Cap_createContext();

	uint32_t deviceCount = Cap_getDeviceCount(ctx);
	INFO("Number of cameras: " + std::to_string(deviceCount));
	for (uint32_t i = 0; i < deviceCount; i++)
	{
		VideoDevice::CameraInfo info;
		info.name = Cap_getDeviceName(ctx, i);
		#ifdef _WIN32
		info.devicePath = Cap_getDeviceUniqueID(ctx, i);
		#elif __linux__
		info.devicePath = "/dev/video" + std::to_string(i);
		#else
		static_assert(false, "Unsupported");
		#endif
		

		// show all supported frame buffer formats
		int32_t nFormats = Cap_getNumFormats(ctx, i);
		for (int32_t j = 0; j < nFormats; j++)
		{
			CapFormatInfo finfo;
			Cap_getFormatInfo(ctx, i, j, &finfo);

			VideoDevice::VideoFormat vf;
			vf.width = finfo.width;
			vf.height = finfo.height;
			vf.fourcc = FourCCToString(finfo.fourcc);
			vf.bpp = finfo.bpp;
			vf.fps = finfo.fps;
			info.formats.push_back(vf);
		}
		result.push_back(info);
	}

	CapResult result1 = Cap_releaseContext(ctx);
	return result;
}


void VideoDevice::initializeLibAv() {
	if (App::getInstance().isIgnoreCamerasSet())
		return;
	avdevice_register_all();
	//findAvailableCameras(availableCameras);
	libavInitialized = true;
}



void VideoDevice::openDevice(const VideoDevice::CameraInfo& ci, size_t optsId) {
	const auto& opt = ci.formats[optsId];

	AVDictionary* options = nullptr;
	av_dict_set(&options, "framerate", std::to_string(opt.fps).c_str(), 0);
	auto size_req = std::to_string(opt.width) + "x" + std::to_string(opt.height);
	av_dict_set(&options, "video_size", size_req.c_str(), 0);

	// adapt pix format from openpnp to ffmpeg
	std::string var, val;
	if (opt.fourcc == "YUY2" || opt.fourcc == "YUYV")
	{
		var = "pixel_format";
		val = "yuyv422";
	}
	else if (opt.fourcc == "MJPG")
	{
		var = "pixel_format";
		val = "mjpeg";
	}
	else if (opt.fourcc == "RGB3")
	{
		var = "pixel_format";
		val = "rgb24";
	}
	else if (opt.fourcc == "RGB4")
	{
		var = "pixel_format";
		val = "rgb32";
	}
	else if (opt.fourcc == "GREY")
	{
		var = "pixel_format";
		val = "gray";
	}
	else
	{
		ERRT("Formato de vídeo no soportado: " + opt.fourcc);
	}
	av_dict_set(&options, var.c_str(), val.c_str(), 0);


#ifdef _WIN32
	// First, we will try to use dshow
	const AVInputFormat* ifmt = av_find_input_format("dshow");
	if (avformat_open_input(&pFormatCtx, ("video=" + ci.name).c_str(), ifmt, &options) != 0) {
		ERRT("No se ha podido abrir la cámara " + ci.name);
	}
#elif __linux__
	//Linux
	AVInputFormat* ifmt = av_find_input_format("video4linux2");
	if (avformat_open_input(&pFormatCtx, ci.devicePath.c_str(), ifmt, &options) != 0) {
		ERRT("No se ha podido abrir la cámara " + ci.name);
	}
#else
	static_assert(false, "Not implemented");
#endif
	INFO("Cámara " + ci.name + " abierta correctamente");
	// Ignored options
	AVDictionaryEntry* t = nullptr;
	while ((t = av_dict_get(options, "", t, AV_DICT_IGNORE_SUFFIX)) != nullptr) {

		WARN(std::string("Opción no reconocida por la cámara: ") + t->key + "=" + t->value);
	}
	av_dict_free(&options);
}
