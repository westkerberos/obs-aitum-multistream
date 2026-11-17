# TikTok target in Aitum Multistream

This document describes the TikTok destination bridge in WEST Gaming's Aitum Multistream fork. The actual TikTok connection implementation is in the matching WEST Gaming Vertical Canvas fork.

## Component ownership

| Responsibility | Owner |
| --- | --- |
| Aitum Multistream dock, destination list, wizard and start/stop control | `obs-aitum-multistream` |
| TikTok connection dialog, credentials, QR login, account/device lookups | `obs-vertical-canvas` |
| TikTok room creation/resume, dynamic endpoint/key, signing, SEI metadata, heartbeat and cleanup | `obs-vertical-canvas` |
| LIVE violations/appeals and Dual LIVE runtime | `obs-vertical-canvas` |
| TikTok identity marker and provider-specific target wiring | Shared through the Vertical Canvas output settings object |

This separation is intentional. Do not add duplicate TikTok session, heartbeat, signing or encoder implementations to Multistream.

## How the bridge works

1. The output wizard is invoked for a **Vertical Canvas** target.
2. Choosing TikTok creates or edits a target with the `tiktok_target` marker.
3. The wizard offers **Configure TikTok connection…**.
4. Multistream calls the OBS proc-handler function `aitum_vertical_configure_tiktok_output`, passing the parent widget, temporary OBS settings object and destination name.
5. Vertical Canvas opens `ConfigureTikTokOutput` and writes the connection fields to the supplied settings object.
6. Multistream stores those provider-specific fields with the vertical destination and restores them when the destination is edited.

The bridge is a configuration call only. At stream start, the Vertical Canvas output lifecycle reads the saved provider configuration, prepares the TikTok session and obtains the actual endpoint/key. The Multistream target's placeholder RTMP values are not the production ingest credentials for native TikTok methods.

If the proc-handler call is unavailable, the matching Vertical Canvas plugin is missing or outdated. Update Vertical Canvas rather than trying to add the TikTok protocol implementation to Multistream.

## User workflow

The integration is available from the **Vertical Canvas** page of the Aitum Multistream settings dialog:

- Add an output and select TikTok.
- Name the destination.
- Open the Vertical Canvas-provided TikTok configuration dialog.
- Select Custom RTMP, LIVE Studio Desktop, LIVE Studio Mobile or Streamlabs Desktop.
- Save and start the destination from the existing Multistream dock.

Main Canvas output creation/editing remains on the original Aitum path. The TikTok provider option is scoped to Vertical Canvas destinations.

## Dual LIVE behavior

Dual LIVE is controlled by the Vertical Canvas destination configuration. In the matching implementation, its default mode reuses the existing Vertical Canvas output encoders for Portrait and the active OBS Main Canvas streaming encoders for Landscape. An active Main Canvas stream is therefore required before starting automatic Dual LIVE.

The Multistream UI does not configure separate encoders for each layout, create the second TikTok ingest feed or manage the second output transport. Vertical Canvas owns all of that. See [the Vertical Canvas Dual LIVE guide](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-output/docs/TIKTOK-LIVE.md#dual-live-portrait--landscape).

## Persistence and compatibility

Provider-specific settings are stored with the named Vertical Canvas destination. When Multistream updates/rebuilds its vertical output list, the TikTok fields must be preserved for the matching destination; editing a target must not silently replace its credentials or Dual LIVE configuration.

Use matching builds of both plugins. The Vertical Canvas bridge contract is currently named `aitum_vertical_configure_tiktok_output`; changing that proc-handler signature/name requires coordinated changes in both repositories.

## Troubleshooting

| Symptom | Likely cause | Action |
| --- | --- | --- |
| TikTok configuration button says Vertical Canvas is unavailable/outdated | Vertical Canvas is absent or does not expose the bridge | Install/update the matching WEST Gaming Vertical Canvas build, restart OBS and retry. |
| Custom RTMP destination cannot be saved | Server/key are empty or invalid | Enter the provider's server URL and stream key. |
| Native method has no server/key in the wizard | Expected for a dynamic TikTok provider | Save the target; Vertical Canvas obtains the endpoint/key when the output starts. |
| Native TikTok startup fails | Missing/invalid RapidAPI key, cookie/session, account eligibility, TikTok room or endpoint problem | Inspect the redacted Vertical Canvas log/error. Do not paste credentials into an issue. |
| Dual LIVE fails before TikTok connects | Main Canvas is not active or its output has no video/audio encoders | Start the horizontal OBS Main Canvas stream first, then start the TikTok destination. |
| Vertical target settings disappear after changing outputs | Settings preservation regression during the Multistream/Vertical Canvas handoff | Report the target name and redacted OBS log; do not attach the profile or cookies. |

## Changes compared with upstream

See [FORK-DIFFERENCES.md](FORK-DIFFERENCES.md) for the source files and exact scope of the WEST Gaming bridge changes.
