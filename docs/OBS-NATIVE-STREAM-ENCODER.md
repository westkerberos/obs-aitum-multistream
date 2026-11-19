# OBS native stream encoder for YouTube (experimental)

This branch adds an **OBS Native Stream Encoder** source to Aitum Multistream's Main Canvas output settings.

## Why this exists

OBS creates a conventional stream encoder in both output modes:

- Advanced Output: `advanced_video_stream`
- Simple Output: `simple_video_stream`

When Twitch Enhanced Broadcasting succeeds, OBS uses a separate multitrack output assembled from Twitch's returned encoder configuration. The active streaming output exposed through `obs_frontend_get_streaming_output()` is then the multitrack output. Aitum's existing **Main Encoder** selector reads encoders attached to that output, so it sees Twitch's negotiated tracks rather than OBS's conventional stream encoder.

The new selector lets Aitum retrieve the OBS-owned conventional stream encoder by name and attach that exact encoder instance to an Aitum RTMP destination. Aitum does not create a replacement video encoder for this selection and does not transcode a Twitch 1080p track to 1440p.

## How to use

1. Start the OBS main stream with Twitch Enhanced Broadcasting enabled.
2. Open the YouTube destination in Aitum Multistream and enable its **Advanced** settings.
3. Under **Video Encoder**, select **OBS Native Stream Encoder**. The option label reports the encoder's dimensions, FPS and bitrate when available.
4. Keep the destination's audio encoder set to the main output audio track.
5. Start the YouTube destination and inspect the OBS log for the line beginning `[Aitum Multistream] Output`. It reports the encoder name, actual encoded dimensions, FPS, bitrate and active state.

## Important behavior and limitations

- Twitch continues to use its own Enhanced Broadcasting tracks. This change does not add a 1440p track to Twitch's negotiated multitrack session and does not bypass Twitch account eligibility.
- YouTube receives video from OBS's conventional stream encoder. That encoder may be dormant until Aitum attaches it and starts its output.
- The encoder's actual dimensions come from OBS's video output and its stream-rescale settings. The Video tab showing 2560x1440 is not, by itself, proof that this encoder is 2560x1440.
- OBS may apply Twitch's service bitrate recommendations to the conventional encoder. Check the logged bitrate. A 1440p60 encoder left at 6000 Kbps will not provide the intended YouTube quality; adjust OBS's service-setting/bitrate behavior before relying on it.
- Treat this as experimental until a Windows build and a live YouTube test confirm that YouTube reports 2560x1440 at 60 FPS.

## Verification target

Success requires both:

1. OBS log: Aitum's YouTube output starts with the OBS native stream encoder at **2560x1440**, **60 FPS**, and the intended bitrate.
2. YouTube playback diagnostics: the received stream exposes **1440p60** as the source quality.

A configured canvas size, encoder object name, or local OBS setting alone is not sufficient verification.
