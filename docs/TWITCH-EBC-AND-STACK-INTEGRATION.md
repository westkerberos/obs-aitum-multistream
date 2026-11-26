# WEST Gaming Aitum Multistream — EBC and stack integration

**Status date:** 2026-11-26  
**Current branch:** `feature/aitum-twitch-enhanced-broadcasting`  
**Current source head:** `8f7780ded10f0afa43a35b02f117e8f95567b2c3`  
**Pull request:** [#2 — Draft Twitch Enhanced Broadcasting validation](https://github.com/westkerberos/obs-aitum-multistream/pull/2)

This fork must be maintained and tested together with the matching [WEST Gaming Vertical Canvas fork](https://github.com/westkerberos/obs-vertical-canvas/tree/feature/tiktok-native-ebc-combined). The complete system architecture, version matrix and paired validation plan are in the [WEST Gaming Streaming Stack source of truth](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-ebc-combined/docs/WEST-GAMING-STREAMING-STACK.md).

## Responsibilities

This repository owns:
- Main Canvas destination configuration and output lifecycle from the upstream Aitum Multistream workflow.
- The additional Vertical Canvas destination list, settings handoff and start/stop controls.
- Twitch Enhanced Broadcasting (EBC) request/negotiation, encoder group creation and Twitch output lifecycle.
- The thin TikTok target wizard/configuration bridge to Vertical Canvas.

The companion Vertical Canvas repository owns:
- Portrait canvas rendering, scenes and vertical output sessions.
- EBC portrait video-encoder lookup/reuse for opted-in vertical outputs.
- TikTok credentials/cookies, authentication, QR login, signing, account/LIVE operations, dynamic ingest, safety and Dual LIVE.

Do not move TikTok runtime/auth/transport into Multistream or create a second TikTok session implementation here.

## Twitch Enhanced Broadcasting data flow

When an EBC-enabled Twitch destination starts, Multistream:
1. Validates the Twitch server/key and EBC destination settings.
2. Queries Twitch's client configuration endpoint for the requested track capabilities.
3. Includes OBS Main Canvas and, when the Aitum Vertical canvas is available and portrait-oriented, the Vertical Canvas in the canvas configuration.
4. Uses DXGI-derived GPU/driver information and the active OBS video/audio capabilities in the Twitch request.
5. Creates the Twitch-provided video/audio encoder configuration and grouped tracks.
6. Creates a dedicated custom RTMP output named with the prefix `aitum_multi_ebc_output_<destinationName>`.
7. Retains the encoder-group/canvas references until the output is torn down.

This produces the Twitch landscape + portrait EBC path. It is a distinct Multistream-managed output; do not assume that the OBS built-in Main Canvas output itself becomes the EBC output.

## Vertical destinations and encoder reuse

The vertical destination UI is delegated to Vertical Canvas through OBS proc-handler APIs. New vertical destinations in this branch default to the reserved selector `__aitum_twitch_ebc_vertical__`. The selected output is configured to use the portrait video encoder from the active Twitch EBC grouped output. If no compatible EBC portrait track is available, Vertical Canvas falls back to its shared independent vertical encoder.

This selection is per vertical destination. An explicit encoder selection can opt out. Existing profiles must be reviewed/migrated; do not assume every saved target is configured identically.

The EBC optimization is primarily about sharing the **video/NVENC** encoder. The Vertical Canvas AAC sharing path is separate from Twitch's EBC audio track group.

## TikTok target bridge

The TikTok option appears only in the Vertical Canvas destination flow:
- Multistream opens the Vertical Canvas configuration dialog through `aitum_vertical_configure_tiktok_output`.
- Provider-specific fields are saved with the vertical target.
- The vertical destination dock is responsible for listing and starting/stopping that target.
- At stream start, Vertical Canvas prepares the TikTok session and obtains dynamic connection details where required.

Custom RTMP, LIVE Studio Desktop/Mobile, Streamlabs and TikTok Dual LIVE runtime belong to Vertical Canvas. This bridge is not itself a TikTok implementation.

## Upstream Aitum 1.0.10 changes missing from this fork

The current feature branch still declares version **1.0.9**. Aitum 1.0.10 contains two changes that must be ported while preserving WEST Gaming additions:

- [Commit 64ef323](https://github.com/Aitum/obs-aitum-multistream/commit/64ef3233a8b078268adf68ec8427d0bfba68678d) calls `LoadVerticalOutputs(false)` after `LoadSettingsFile()` on `OBS_FRONTEND_EVENT_FINISHED_LOADING`. Our current `frontend_event` handler does not call this. Porting it is required for the post-startup refresh of the Vertical destination list.
- [Commit 41329c5](https://github.com/Aitum/obs-aitum-multistream/commit/41329c56d8f1254b7b55a2821a44b08abeeab3bb) bumps the upstream plugin to 1.0.10 and adds the new OBS third-party Windows plugin install layout to the installer.

Do not replace this fork with the official 1.0.10 binary: that would lose the custom Twitch EBC implementation and TikTok bridge. Port the minimal upstream changes into this branch and test the paired plugin build.

## OBS 33.0.3 compatibility status

The current fork `buildspec.json` still declares:
- Aitum Multistream version: **1.0.9**.
- OBS source dependency: **31.1.0**.

Latest known Windows feature-branch build: [CI run #44808003981](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/44808003981), success for the configured dependency baseline. Ubuntu/macOS and formatting jobs were skipped. This is not proof of an OBS 33.0.3 build or full Twitch/TikTok runtime validation.

Before claiming OBS 33.0.3 support, update the OBS pin/toolchain and package layout, build this branch with the matching Vertical Canvas commit, then run the complete paired test plan in the [system source of truth](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-ebc-combined/docs/WEST-GAMING-STREAMING-STACK.md).

## Maintenance invariants

- Preserve both Main Canvas and Vertical Canvas in the Twitch EBC negotiation when the Vertical Canvas is available.
- Preserve the EBC track group and dedicated output lifetime until Twitch output teardown.
- Preserve Vertical Canvas encoder marker/prefix/proc-handler contracts.
- Preserve TikTok provider configuration fields and delegate the actual runtime to Vertical Canvas.
- Keep Main Canvas non-EBC targets working through the original Aitum output path.
- Test the full paired stack whenever either repository, OBS, FFmpeg/Qt dependencies, installer layout or output lifecycle changes.

## License and release status

This is an experimental open draft branch, not a stable release. The companion Vertical Canvas TikTok source has unresolved license/provenance matters documented in the companion repository. Do not publish a combined binary until those issues are remediated or independently cleared.
