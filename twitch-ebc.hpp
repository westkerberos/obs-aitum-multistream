#pragma once

#include <obs.h>
#include <memory>
#include <QString>

using TwitchEbcEncoderGroup = std::shared_ptr<obs_encoder_group_t>;

// Creates a Twitch Enhanced Broadcasting RTMP output and its server-requested
// video/audio encoders. The caller owns the returned output reference and must
// retain encoder_group for the lifetime of the output.
bool CreateTwitchEnhancedBroadcastingOutput(const char *output_name, const char *stream_server,
					    const char *stream_key, obs_output_t **output,
					    TwitchEbcEncoderGroup &encoder_group, QString &error);

// Releases an EBC output and the service reference assigned to it.
void ReleaseTwitchEnhancedBroadcastingOutput(obs_output_t *output);
