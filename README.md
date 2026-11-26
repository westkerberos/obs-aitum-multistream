# WEST Gaming Aitum Multistream for OBS Studio

A WEST Gaming-maintained fork of [Aitum Multistream](https://github.com/Aitum/obs-aitum-multistream). This is the Main Canvas and Twitch Enhanced Broadcasting (EBC) half of a matched streaming system with the WEST Gaming [Vertical Canvas fork](https://github.com/westkerberos/obs-vertical-canvas/tree/feature/tiktok-native-ebc-combined).

**Status: experimental development — no stable WEST Gaming release is published.**

## Complete system architecture

The authoritative end-to-end design, component ownership, encoder routing, tested-version matrix and update checklist are maintained in the [WEST Gaming Streaming Stack — Source of Truth](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-ebc-combined/docs/WEST-GAMING-STREAMING-STACK.md).

This repository is responsible for:
- Main Canvas output destinations and the existing Aitum Multistream dock.
- Twitch Enhanced Broadcasting negotiation/output, with both Main Canvas landscape and Vertical Canvas portrait tracks when the vertical canvas is available.
- The Vertical Canvas destinations page/list and start/stop controls.
- The TikTok target configuration bridge to Vertical Canvas.

The companion Vertical Canvas repository is responsible for:
- Vertical scenes/canvas and vertical destination outputs (including YouTube, TikTok and Instagram).
- Reusing Twitch EBC's negotiated portrait **video encoder** for each opted-in vertical destination; using a shared independent vertical video encoder when EBC is unavailable or not selected.
- TikTok credentials, login/QR, session, signing, dynamic ingest, LIVE metadata/safety, and TikTok Dual LIVE runtime.

Do not treat either plugin as a standalone update. Build, install and validate the matching pair together.

## Development source and validation

- Current EBC branch: [`feature/aitum-twitch-enhanced-broadcasting`](https://github.com/westkerberos/obs-aitum-multistream/tree/feature/aitum-twitch-enhanced-broadcasting).
- Current integration PR: [PR #2](https://github.com/westkerberos/obs-aitum-multistream/pull/2) — open draft.
- Required companion Vertical Canvas branch: [`feature/tiktok-native-ebc-combined`](https://github.com/westkerberos/obs-vertical-canvas/tree/feature/tiktok-native-ebc-combined), [PR #6](https://github.com/westkerberos/obs-vertical-canvas/pull/6).
- Latest known Windows x64 build: [CI run #44808003981](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/44808003981) — success on the fork's configured OBS 31.1.0 dependency baseline. Ubuntu/macOS and formatting jobs were skipped.
- This successful compile does **not** prove OBS 33.0.3 compatibility or runtime EBC/encoder reuse.
- No stable combined installer or GitHub Release exists.

## TikTok integration boundary

The TikTok option is a Vertical Canvas destination. Multistream calls the Vertical Canvas proc-handler `aitum_vertical_configure_tiktok_output` and retains provider settings with the destination. The Vertical Canvas fork owns the actual TikTok session/runtime. This bridge is not a second TikTok implementation.

## Upstream update currently required

Our fork is still declared as version 1.0.9. Port Aitum Multistream 1.0.10's vertical-output reload after OBS finishes loading, and its OBS 33 third-party Windows install layout, while preserving the WEST EBC and TikTok bridge changes. See [EBC integration notes](docs/TWITCH-EBC-AND-STACK-INTEGRATION.md).

Do not replace this fork with the official Aitum 1.0.10 binary: that would remove the custom EBC implementation and TikTok bridge.

## Development documentation

- [Twitch EBC and complete stack integration](docs/TWITCH-EBC-AND-STACK-INTEGRATION.md)
- [TikTok target bridge and troubleshooting](docs/TIKTOK-TARGET.md)
- [Fork differences](docs/FORK-DIFFERENCES.md)
- [Windows build and paired test status](docs/BUILD-AND-TESTING.md)
- [WEST Gaming complete stack source of truth](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-ebc-combined/docs/WEST-GAMING-STREAMING-STACK.md)

The companion Vertical Canvas repository still has unresolved source-license/provenance issues. Do not redistribute a combined binary until these are remediated or independently cleared.
