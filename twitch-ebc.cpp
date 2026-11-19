#include "twitch-ebc.hpp"

#include <obs.h>
#include <util/platform.h>
#include <curl/curl.h>

#include <QSysInfo>
#include <QUrl>
#include <QUrlQuery>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <memory>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <dxgi.h>
#endif

namespace {
constexpr const char *kConfigurationUrl = "https://ingest.twitch.tv/api/v3/GetClientConfiguration";

size_t CurlWriteCallback(char *data, size_t size, size_t count, void *user_data)
{
	const size_t bytes = size * count;
	auto *response = static_cast<std::string *>(user_data);
	response->append(data, bytes);
	return bytes;
}

void SetObject(obs_data_t *parent, const char *name, obs_data_t *child)
{
	obs_data_set_obj(parent, name, child);
	obs_data_release(child);
}

obs_data_t *CreateGoLiveRequest(const char *stream_key)
{
	static constexpr const char *skeleton =
		"{"
		"\"service\":\"IVS\","
		"\"schema_version\":\"2025-01-25\","
		"\"authentication\":\"\","
		"\"client\":{\"name\":\"obs-studio\",\"version\":\"\",\"supported_codecs\":[]},"
		"\"capabilities\":{"
		"  \"cpu\":{\"physical_cores\":1,\"logical_cores\":1,\"speed\":0,\"name\":\"Unknown\"},"
		"  \"memory\":{\"total\":0,\"free\":0},"
		"  \"system\":{\"version\":\"unknown\",\"name\":\"unknown\",\"build\":0,\"release\":\"unknown\","
		"             \"revision\":\"0\",\"bits\":64,\"arm\":false,\"armEmulation\":false},"
		"  \"gpu\":[]"
		"},"
		"\"preferences\":{"
		"  \"maximum_aggregate_bitrate\":null,"
		"  \"maximum_video_tracks\":null,"
		"  \"vod_track_audio\":false,"
		"  \"composition_gpu_index\":0,"
		"  \"audio_samples_per_sec\":48000,"
		"  \"audio_channels\":2,"
		"  \"audio_fixed_buffering\":false,"
		"  \"audio_max_buffering_ms\":960,"
		"  \"canvases\":[]"
		"}"
		"}";

	obs_data_t *root = obs_data_create_from_json(skeleton);
	if (!root) {
		return nullptr;
	}
	obs_data_set_string(root, "authentication", stream_key ? stream_key : "");

	obs_data_t *client = obs_data_get_obj(root, "client");
	obs_data_set_string(client, "version", obs_get_version_string());
	obs_data_array_t *codecs = obs_data_array_create();
	for (size_t i = 0; i < 128; ++i) {
		const char *encoder_id = nullptr;
		if (!obs_enum_encoder_types(i, &encoder_id)) {
			break;
		}
		const char *codec = obs_get_encoder_codec(encoder_id);
		if (!codec || obs_get_encoder_type(encoder_id) != OBS_ENCODER_VIDEO) {
			continue;
		}
		if (astrcmpi(codec, "h264") == 0 || astrcmpi(codec, "hevc") == 0 || astrcmpi(codec, "av1") == 0) {
			bool already_added = false;
			const size_t codec_count = obs_data_array_count(codecs);
			for (size_t j = 0; j < codec_count; ++j) {
				obs_data_t *entry = obs_data_array_item(codecs, j);
				const bool matches = strcmp(obs_data_get_string(entry, "value"), codec) == 0;
				obs_data_release(entry);
				if (matches) {
					already_added = true;
					break;
				}
			}
			// JSON arrays of strings are represented by obs_data_array_push_back
			// on an object with a single string value.
			if (!already_added) {
				obs_data_t *entry = obs_data_create();
				obs_data_set_string(entry, "value", codec);
				obs_data_array_push_back(codecs, entry);
				obs_data_release(entry);
			}
		}
	}
	// obs_data arrays serialize objects, so construct supported_codecs as a JSON
	// string array below from the encoder registry instead of using this array.
	obs_data_array_release(codecs);
	obs_data_release(client);

	client = obs_data_create_from_json("{\"name\":\"obs-studio\",\"version\":\"\",\"supported_codecs\":[\"h264\"]}");
	obs_data_set_string(client, "version", obs_get_version_string());
	obs_data_array_t *codec_array = obs_data_array_create();
	bool has_h264 = false, has_hevc = false, has_av1 = false;
	for (size_t i = 0; i < 128; ++i) {
		const char *encoder_id = nullptr;
		if (!obs_enum_encoder_types(i, &encoder_id)) {
			break;
		}
		if (obs_get_encoder_type(encoder_id) != OBS_ENCODER_VIDEO) {
			continue;
		}
		const char *codec = obs_get_encoder_codec(encoder_id);
		if (!codec) {
			continue;
		}
		if (astrcmpi(codec, "h264") == 0) has_h264 = true;
		if (astrcmpi(codec, "hevc") == 0) has_hevc = true;
		if (astrcmpi(codec, "av1") == 0) has_av1 = true;
	}
	// JSON string arrays can be safely injected through the initial data object.
	std::string supported = "[";
	bool comma = false;
	if (has_h264) { supported += "\"h264\""; comma = true; }
	if (has_hevc) { if (comma) supported += ","; supported += "\"h265\""; comma = true; }
	if (has_av1) { if (comma) supported += ","; supported += "\"av1\""; }
	supported += "]";
	obs_data_release(client);
	std::string client_json = "{\"name\":\"obs-studio\",\"version\":\"";
	client_json += obs_get_version_string();
	client_json += "\",\"supported_codecs\":";
	client_json += supported;
	client_json += "}";
	client = obs_data_create_from_json(client_json.c_str());
	SetObject(root, "client", client);
	obs_data_array_release(codec_array);

	obs_data_t *capabilities = obs_data_get_obj(root, "capabilities");
	obs_data_t *cpu = obs_data_get_obj(capabilities, "cpu");
	const uint32_t logical_cores = std::max(1u, os_get_logical_cores());
	const uint32_t physical_cores = std::max(1u, os_get_physical_cores());
	obs_data_set_int(cpu, "logical_cores", logical_cores);
	obs_data_set_int(cpu, "physical_cores", physical_cores);
	obs_data_set_string(cpu, "name", "Unknown");
	obs_data_set_int(cpu, "speed", 0);
	SetObject(capabilities, "cpu", cpu);

	obs_data_t *memory = obs_data_get_obj(capabilities, "memory");
	obs_data_set_int(memory, "total", (long long)os_get_sys_total_size());
	obs_data_set_int(memory, "free", (long long)os_get_sys_free_size());
	SetObject(capabilities, "memory", memory);

	obs_data_t *system = obs_data_get_obj(capabilities, "system");
	obs_data_set_string(system, "name", QSysInfo::productType().toUtf8().constData());
	obs_data_set_string(system, "version", QSysInfo::kernelVersion().toUtf8().constData());
	obs_data_set_string(system, "release", QSysInfo::productVersion().toUtf8().constData());
	obs_data_set_int(system, "bits", sizeof(void *) * 8);
#if defined(__aarch64__) || defined(_M_ARM64)
	obs_data_set_bool(system, "arm", true);
#else
	obs_data_set_bool(system, "arm", false);
#endif
	obs_data_set_bool(system, "armEmulation", false);
	SetObject(capabilities, "system", system);

#ifdef _WIN32
	obs_data_array_t *gpus = obs_data_array_create();
	IDXGIFactory1 *factory = nullptr;
	if (SUCCEEDED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void **>(&factory)))) {
		for (UINT index = 0;; ++index) {
			IDXGIAdapter1 *adapter = nullptr;
			if (factory->EnumAdapters1(index, &adapter) != S_OK) {
				break;
			}
			DXGI_ADAPTER_DESC1 desc = {};
			if (SUCCEEDED(adapter->GetDesc1(&desc)) && !(desc.VendorId == 0x1414 && desc.DeviceId == 0x8c)) {
				char model[512] = {};
				WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, model, sizeof(model), nullptr, nullptr);
				obs_data_t *gpu = obs_data_create();
				obs_data_set_string(gpu, "model", model);
				obs_data_set_int(gpu, "vendor_id", desc.VendorId);
				obs_data_set_int(gpu, "device_id", desc.DeviceId);
				obs_data_set_int(gpu, "dedicated_video_memory", (long long)desc.DedicatedVideoMemory);
				obs_data_set_int(gpu, "shared_system_memory", (long long)desc.SharedSystemMemory);
				obs_data_set_string(gpu, "driver_version", "unknown");
				obs_data_array_push_back(gpus, gpu);
				obs_data_release(gpu);
			}
			adapter->Release();
		}
		factory->Release();
	}
	obs_data_set_array(capabilities, "gpu", gpus);
	obs_data_array_release(gpus);
#endif
	obs_data_release(capabilities);

	obs_video_info video = {};
	if (obs_get_video_info(&video)) {
		obs_data_t *preferences = obs_data_get_obj(root, "preferences");
		obs_data_set_int(preferences, "composition_gpu_index", video.adapter);
		obs_data_array_t *canvases = obs_data_array_create();
		obs_data_t *canvas = obs_data_create();
		obs_data_set_int(canvas, "width", video.output_width);
		obs_data_set_int(canvas, "height", video.output_height);
		obs_data_set_int(canvas, "canvas_width", video.base_width);
		obs_data_set_int(canvas, "canvas_height", video.base_height);
		obs_data_t *fps = obs_data_create();
		obs_data_set_int(fps, "numerator", video.fps_num);
		obs_data_set_int(fps, "denominator", video.fps_den);
		SetObject(canvas, "framerate", fps);
		obs_data_array_push_back(canvases, canvas);
		obs_data_release(canvas);
		obs_data_set_array(preferences, "canvases", canvases);
		obs_data_array_release(canvases);
		SetObject(root, "preferences", preferences);
	}

	return root;
}

bool DownloadConfiguration(const char *stream_key, obs_data_t **response, QString &error)
{
	obs_data_t *request = CreateGoLiveRequest(stream_key);
	if (!request) {
		error = QStringLiteral("No se pudo preparar la solicitud de configuración de Twitch.");
		return false;
	}
	const char *json = obs_data_get_json(request);
	const std::string request_body = json ? json : "{}";
	obs_data_release(request);

	static const bool curl_initialized = curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK;
	if (!curl_initialized) {
		error = QStringLiteral("No se pudo inicializar libcurl.");
		return false;
	}
	CURL *curl = curl_easy_init();
	if (!curl) {
		error = QStringLiteral("No se pudo inicializar la conexión con Twitch.");
		return false;
	}
	std::string response_body;
	struct curl_slist *headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Accept: application/json");
	curl_easy_setopt(curl, CURLOPT_URL, kConfigurationUrl);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request_body.c_str());
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request_body.size());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_body);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	const CURLcode result = curl_easy_perform(curl);
	long http_status = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_status);
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	if (result != CURLE_OK) {
		error = QStringLiteral("No se pudo contactar a Twitch: %1").arg(QString::fromUtf8(curl_easy_strerror(result)));
		return false;
	}
	if (http_status < 200 || http_status >= 300) {
		error = QStringLiteral("Twitch rechazó Enhanced Broadcasting (HTTP %1). Respuesta: %2")
				.arg(http_status)
				.arg(QString::fromUtf8(response_body.substr(0, 1200).c_str()));
		return false;
	}
	*response = obs_data_create_from_json(response_body.c_str());
	if (!*response) {
		error = QStringLiteral("Twitch devolvió una configuración que no se pudo interpretar.");
		return false;
	}
	return true;
}

bool EncoderTypeAvailable(const char *type, enum obs_encoder_type expected_type)
{
	for (size_t i = 0; i < 128; ++i) {
		const char *current = nullptr;
		if (!obs_enum_encoder_types(i, &current)) {
			break;
		}
		if (strcmp(current, type) == 0 && obs_get_encoder_type(current) == expected_type) {
			return true;
		}
	}
	return false;
}

obs_scale_type ParseScaleType(const char *value)
{
	if (!value) return OBS_SCALE_BICUBIC;
	if (strcmp(value, "OBS_SCALE_POINT") == 0) return OBS_SCALE_POINT;
	if (strcmp(value, "OBS_SCALE_BILINEAR") == 0) return OBS_SCALE_BILINEAR;
	if (strcmp(value, "OBS_SCALE_LANCZOS") == 0) return OBS_SCALE_LANCZOS;
	if (strcmp(value, "OBS_SCALE_AREA") == 0) return OBS_SCALE_AREA;
	return OBS_SCALE_BICUBIC;
}

video_colorspace ParseColorSpace(const char *value)
{
	if (!value) return VIDEO_CS_709;
	if (strcmp(value, "VIDEO_CS_601") == 0) return VIDEO_CS_601;
	if (strcmp(value, "VIDEO_CS_SRGB") == 0) return VIDEO_CS_SRGB;
	if (strcmp(value, "VIDEO_CS_2100_PQ") == 0) return VIDEO_CS_2100_PQ;
	if (strcmp(value, "VIDEO_CS_2100_HLG") == 0) return VIDEO_CS_2100_HLG;
	return VIDEO_CS_709;
}

video_range_type ParseRange(const char *value)
{
	if (value && strcmp(value, "VIDEO_RANGE_FULL") == 0) return VIDEO_RANGE_FULL;
	return VIDEO_RANGE_PARTIAL;
}

video_format ParseVideoFormat(const char *value)
{
	if (!value) return VIDEO_FORMAT_NV12;
	if (strcmp(value, "VIDEO_FORMAT_I420") == 0) return VIDEO_FORMAT_I420;
	if (strcmp(value, "VIDEO_FORMAT_BGRA") == 0) return VIDEO_FORMAT_BGRA;
	if (strcmp(value, "VIDEO_FORMAT_I444") == 0) return VIDEO_FORMAT_I444;
	if (strcmp(value, "VIDEO_FORMAT_I010") == 0) return VIDEO_FORMAT_I010;
	if (strcmp(value, "VIDEO_FORMAT_P010") == 0) return VIDEO_FORMAT_P010;
	if (strcmp(value, "VIDEO_FORMAT_P216") == 0) return VIDEO_FORMAT_P216;
	if (strcmp(value, "VIDEO_FORMAT_P416") == 0) return VIDEO_FORMAT_P416;
	return VIDEO_FORMAT_NV12;
}

obs_encoder_t *CreateAudioEncoder(const char *codec, const char *name, obs_data_t *config)
{
	const char *encoder_id = nullptr;
	for (size_t i = 0; i < 128; ++i) {
		if (!obs_enum_encoder_types(i, &encoder_id)) break;
		if (obs_get_encoder_type(encoder_id) != OBS_ENCODER_AUDIO) continue;
		const char *available_codec = obs_get_encoder_codec(encoder_id);
		if (!available_codec || astrcmpi(codec, available_codec) != 0) continue;
		obs_data_t *settings = obs_data_get_obj(config, "settings");
		if (!settings) settings = obs_data_create();
		obs_encoder_t *encoder = obs_audio_encoder_create(encoder_id, name, settings, 0, nullptr);
		obs_data_release(settings);
		if (!encoder) return nullptr;
		obs_encoder_set_audio(encoder, obs_get_audio());
		return encoder;
	}
	return nullptr;
}
} // namespace

bool CreateTwitchEnhancedBroadcastingOutput(const char *output_name, const char *stream_server,
					    const char *stream_key, obs_output_t **output,
					    TwitchEbcEncoderGroup &encoder_group, QString &error)
{
	if (!output || !stream_server || !stream_key || !stream_key[0]) {
		error = QStringLiteral("Falta el servidor o la clave de transmisión de Twitch.");
		return false;
	}
	*output = nullptr;

	obs_data_t *configuration = nullptr;
	if (!DownloadConfiguration(stream_key, &configuration, error)) {
		return false;
	}

	obs_data_array_t *ingest_endpoints = obs_data_get_array(configuration, "ingest_endpoints");
	std::string ingest_authentication = stream_key;
	bool found_rtmp = false;
	const size_t endpoint_count = obs_data_array_count(ingest_endpoints);
	for (size_t i = 0; i < endpoint_count; ++i) {
		obs_data_t *endpoint = obs_data_array_item(ingest_endpoints, i);
		const char *protocol = obs_data_get_string(endpoint, "protocol");
		if (protocol && astrcmpi(protocol, "RTMP") == 0) {
			const char *authentication = obs_data_get_string(endpoint, "authentication");
			if (authentication && authentication[0]) ingest_authentication = authentication;
			found_rtmp = true;
			obs_data_release(endpoint);
			break;
		}
		obs_data_release(endpoint);
	}
	obs_data_array_release(ingest_endpoints);
	if (!found_rtmp) {
		obs_data_release(configuration);
		error = QStringLiteral("Twitch no devolvió un endpoint RTMP para Enhanced Broadcasting.");
		return false;
	}

	const char *config_id = nullptr;
	obs_data_t *meta = obs_data_get_obj(configuration, "meta");
	config_id = meta ? obs_data_get_string(meta, "config_id") : nullptr;
	const std::string config_id_value = config_id ? config_id : "";
	if (meta) obs_data_release(meta);
	if (config_id_value.empty()) {
		obs_data_release(configuration);
		error = QStringLiteral("La configuración de Twitch no contiene config_id.");
		return false;
	}

	obs_data_array_t *video_configs = obs_data_get_array(configuration, "encoder_configurations");
	const size_t video_count = obs_data_array_count(video_configs);
	if (!video_count || video_count > MAX_OUTPUT_VIDEO_ENCODERS) {
		obs_data_array_release(video_configs);
		obs_data_release(configuration);
		error = QStringLiteral("Twitch devolvió una cantidad de encoders de vídeo no válida.");
		return false;
	}

	obs_data_t *service_settings = obs_data_create();
	std::string server_url = stream_server;
	const std::string stream_key_token = "/{stream_key}";
	const size_t token_position = server_url.find(stream_key_token);
	if (token_position != std::string::npos) server_url.erase(token_position);
	obs_data_set_string(service_settings, "server", server_url.c_str());

	QUrl user_key(QString::fromUtf8(stream_key));
	QUrlQuery user_key_query(user_key);
	QUrlQuery output_query;
	for (const auto &query_item : user_key_query.queryItems()) {
		output_query.addQueryItem(query_item.first, query_item.second);
	}
	output_query.addQueryItem(QStringLiteral("clientConfigId"), QString::fromStdString(config_id_value));
	QUrl configured_key(QString::fromUtf8(ingest_authentication.c_str()));
	configured_key.setQuery(output_query);
	obs_data_set_string(service_settings, "key", configured_key.toString(QUrl::FullyEncoded).toUtf8().constData());

	obs_service_t *service = obs_service_create("rtmp_custom", output_name, service_settings, nullptr);
	obs_data_release(service_settings);
	if (!service) {
		obs_data_array_release(video_configs);
		obs_data_release(configuration);
		error = QStringLiteral("No se pudo crear el servicio RTMP de Twitch.");
		return false;
	}

	std::string internal_output_name = "aitum_multi_ebc_output_";
	internal_output_name += output_name;
	obs_output_t *ebc_output = obs_output_create("rtmp_output", internal_output_name.c_str(), nullptr, nullptr);
	if (!ebc_output) {
		obs_service_release(service);
		obs_data_array_release(video_configs);
		obs_data_release(configuration);
		error = QStringLiteral("No se pudo crear la salida RTMP de Enhanced Broadcasting.");
		return false;
	}
	obs_output_set_service(ebc_output, service);
	obs_service_release(service);

	encoder_group = TwitchEbcEncoderGroup(obs_encoder_group_create(), obs_encoder_group_destroy);
	if (!encoder_group) {
		obs_output_release(ebc_output);
		obs_data_array_release(video_configs);
		obs_data_release(configuration);
		error = QStringLiteral("No se pudo crear el grupo de encoders de Twitch.");
		return false;
	}

	obs_video_info video = {};
	obs_get_video_info(&video);
	for (size_t i = 0; i < video_count; ++i) {
		obs_data_t *encoder_config = obs_data_array_item(video_configs, i);
		const char *encoder_type = obs_data_get_string(encoder_config, "type");
		if (!encoder_type || !EncoderTypeAvailable(encoder_type, OBS_ENCODER_VIDEO)) {
			obs_data_release(encoder_config);
			obs_output_release(ebc_output);
			obs_data_array_release(video_configs);
			obs_data_release(configuration);
			error = QStringLiteral("El encoder de vídeo solicitado por Twitch no está disponible en OBS.");
			return false;
		}
		obs_data_t *encoder_settings = obs_data_get_obj(encoder_config, "settings");
		if (!encoder_settings) encoder_settings = obs_data_create();
		obs_data_set_bool(encoder_settings, "disable_scenecut", true);
		std::string encoder_name = "aitum_twitch_ebc_video_";
		encoder_name += std::to_string(i);
		encoder_name += "_";
		encoder_name += output_name;
		obs_encoder_t *encoder = obs_video_encoder_create(encoder_type, encoder_name.c_str(), encoder_settings, nullptr);
		obs_data_release(encoder_settings);
		if (!encoder) {
			obs_data_release(encoder_config);
			obs_output_release(ebc_output);
			obs_data_array_release(video_configs);
			obs_data_release(configuration);
			error = QStringLiteral("OBS no pudo crear uno de los encoders de vídeo solicitados por Twitch.");
			return false;
		}
		obs_encoder_set_video(encoder, obs_get_video());
		const uint32_t width = (uint32_t)obs_data_get_int(encoder_config, "width");
		const uint32_t height = (uint32_t)obs_data_get_int(encoder_config, "height");
		if (!width || !height) {
			obs_encoder_release(encoder);
			obs_data_release(encoder_config);
			obs_output_release(ebc_output);
			obs_data_array_release(video_configs);
			obs_data_release(configuration);
			error = QStringLiteral("Twitch devolvió una resolución inválida para uno de los encoders.");
			return false;
		}
		obs_encoder_set_scaled_size(encoder, width, height);
		obs_encoder_set_gpu_scale_type(encoder, ParseScaleType(obs_data_get_string(encoder_config, "gpu_scale_type")));
		obs_encoder_set_preferred_video_format(encoder, ParseVideoFormat(obs_data_get_string(encoder_config, "format")));
		obs_encoder_set_preferred_color_space(encoder, ParseColorSpace(obs_data_get_string(encoder_config, "colorspace")));
		obs_encoder_set_preferred_range(encoder, ParseRange(obs_data_get_string(encoder_config, "range")));

		obs_data_t *framerate = obs_data_get_obj(encoder_config, "framerate");
		if (framerate && video.fps_num && video.fps_den) {
			const uint32_t numerator = (uint32_t)obs_data_get_int(framerate, "numerator");
			const uint32_t denominator = (uint32_t)obs_data_get_int(framerate, "denominator");
			if (numerator && denominator) {
				const double current_fps = (double)video.fps_num / video.fps_den;
				const double requested_fps = (double)numerator / denominator;
				const uint32_t divisor = (uint32_t)std::max(1.0, std::round(current_fps / requested_fps));
				if (divisor > 1) obs_encoder_set_frame_rate_divisor(encoder, divisor);
			}
			obs_data_release(framerate);
		}
		if (!obs_encoder_set_group(encoder, encoder_group.get())) {
			obs_encoder_release(encoder);
			obs_data_release(encoder_config);
			obs_output_release(ebc_output);
			obs_data_array_release(video_configs);
			obs_data_release(configuration);
			error = QStringLiteral("OBS no pudo asociar un encoder al grupo de Enhanced Broadcasting.");
			return false;
		}
		obs_output_set_video_encoder2(ebc_output, encoder, i);
		obs_encoder_release(encoder);
		obs_data_release(encoder_config);
	}
	obs_data_array_release(video_configs);

	obs_data_t *audio_configurations = obs_data_get_obj(configuration, "audio_configurations");
	obs_data_array_t *audio_configs = audio_configurations ? obs_data_get_array(audio_configurations, "live") : nullptr;
	const size_t audio_count = obs_data_array_count(audio_configs);
	if (!audio_count || audio_count > MAX_OUTPUT_AUDIO_ENCODERS) {
		obs_data_array_release(audio_configs);
		if (audio_configurations) obs_data_release(audio_configurations);
		obs_output_release(ebc_output);
		obs_data_release(configuration);
		error = QStringLiteral("Twitch devolvió una configuración de audio vacía o no válida.");
		return false;
	}
	for (size_t i = 0; i < audio_count; ++i) {
		obs_data_t *audio_config = obs_data_array_item(audio_configs, i);
		const char *codec = obs_data_get_string(audio_config, "codec");
		std::string audio_name = "aitum_twitch_ebc_audio_";
		audio_name += std::to_string(i);
		audio_name += "_";
		audio_name += output_name;
		obs_encoder_t *audio_encoder = CreateAudioEncoder(codec, audio_name.c_str(), audio_config);
		if (!audio_encoder) {
			obs_data_release(audio_config);
			obs_data_array_release(audio_configs);
			obs_data_release(audio_configurations);
			obs_output_release(ebc_output);
			obs_data_release(configuration);
			error = QStringLiteral("No hay un encoder de audio compatible con el codec %1 que solicitó Twitch.")
					.arg(QString::fromUtf8(codec ? codec : "desconocido"));
			return false;
		}
		obs_output_set_audio_encoder(ebc_output, audio_encoder, i);
		obs_encoder_release(audio_encoder);
		obs_data_release(audio_config);
	}
	obs_data_array_release(audio_configs);
	if (audio_configurations) obs_data_release(audio_configurations);

	obs_data_release(configuration);
	*output = ebc_output;
	return true;
}
