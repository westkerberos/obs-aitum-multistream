# TikTok target in WEST Gaming Aitum Multistream

This document describes the TikTok destination bridge in WEST Gaming's Aitum Multistream 1.0.10 fork. The actual TikTok connection/runtime is owned by the matching WEST Gaming Vertical Canvas 1.6.8 integration.

See the [WEST Gaming full-stack source of truth](https://github.com/westkerberos/obs-vertical-canvas/blob/main/docs/WEST-GAMING-STREAMING-STACK.md) for component ownership, version matrix, encoder routing, the recommended start order and test matrix.

## Component ownership

| Responsibility | Owner |
|---|---|
| Aitum Multistream dock, Main Canvas destinations and start/stop UI | obs-aitum-multistream |
| Twitch Enhanced Broadcasting negotiation/output and EBC encoder tracks | obs-aitum-multistream |
| Vertical destination list/start-stop bridge | shared proc-handler contract |
| TikTok connection dialog, credentials, QR login, account/device lookups | obs-vertical-canvas |
| TikTok room/session, dynamic endpoints, signing, heartbeat and cleanup | obs-vertical-canvas |
| LIVE violations/appeals and Dual LIVE runtime | obs-vertical-canvas |

Do not add duplicate TikTok session, heartbeat, signing or encoder implementations to Multistream.

## How the bridge works

1. The output wizard is invoked for a Vertical Canvas destination.
2. Choosing TikTok creates or edits a target with the provider marker and provider-specific settings.
3. The wizard offers the Vertical Canvas TikTok configuration dialog.
4. Multistream calls the OBS proc-handler function aitum_vertical_configure_tiktok_output, passing the parent widget, temporary OBS settings object and destination name.
5. Vertical Canvas opens its TikTok editor and writes the connection fields to the supplied settings object.
6. Multistream stores those provider-specific fields with the vertical destination and restores them when the destination is edited.

The bridge is a configuration call only. At stream start, Vertical Canvas reads the saved provider configuration, prepares the TikTok session and obtains the actual endpoint/key. Placeholder RTMP values in the Multistream target are not production ingest credentials for native TikTok methods.

If the proc-handler call is unavailable, the matching Vertical Canvas plugin is missing or outdated. Update the paired Vertical Canvas build rather than adding TikTok protocol code to Multistream.

## User workflow

From the Vertical Canvas page in the Aitum Multistream settings dialog:
- Add a Vertical Canvas output and select TikTok.
- Name the destination.
- Open the Vertical Canvas-provided TikTok configuration dialog.
- Choose Custom RTMP, LIVE Studio Desktop, LIVE Studio Mobile or Streamlabs Desktop.
- Save and start the destination from the existing Multistream dock.

## Dual LIVE and encoder prerequisites

Dual LIVE runtime belongs to Vertical Canvas. Multistream does not create the TikTok media feeds or own the TikTok session.

Automatic Dual LIVE has two independent prerequisites:
- Start Twitch EBC first when the TikTok Portrait destination is configured to reuse the negotiated 1080×1920 EBC video encoder.
- Start a horizontal Main Canvas stream before TikTok Dual LIVE. The TikTok Landscape feed reuses the active Main Canvas video/audio encoders.

Recommended WEST Gaming order:
1. Twitch EBC.
2. YouTube Vertical.
3. YouTube Horizontal/Main Canvas.
4. Instagram Vertical.
5. TikTok Dual LIVE.

Vertical Canvas shows a pre-flight message if Twitch EBC's portrait track is unavailable or if the Main Canvas horizontal stream is missing. It can explicitly continue with an independent vertical encoder only after warning about the additional encoder load.

## Persistence and compatibility

TikTok provider fields are stored with the Vertical Canvas destination settings, not as a second Multistream TikTok account/session store. Keep the bridge signature and saved provider fields compatible when updating either fork.

## Troubleshooting

- **TikTok configuration dialog does not open:** check that Vertical Canvas is loaded, then confirm its version and proc-handler registration in the OBS startup log.
- **TikTok Portrait creates another vertical encoder:** check whether Twitch EBC was active and had created the 1080×1920 track before the vertical destination started. Review the Vertical Canvas EBC selection and pre-flight warning.
- **TikTok Landscape does not start:** confirm an active OBS Main Canvas horizontal output exists before starting Dual LIVE.
- **Only Portrait remains active:** check the Landscape RTMP output's own log/reconnect state. A secondary Landscape failure no longer forcibly tears down Portrait; inspect whether Landscape has reconnected instead of assuming both feeds are live.
- **Do not post:** cookies, RapidAPI keys, stream keys, Streamlabs tokens, unredacted profiles or account IDs.

## Changes compared with upstream

This is a WEST Gaming extension to the upstream Aitum Multistream workflow. Upstream Main Canvas destinations remain on their normal Aitum implementation. TikTok settings are delegated to Vertical Canvas through the bridge; native TikTok session/runtime remains exclusively in that companion plugin.
