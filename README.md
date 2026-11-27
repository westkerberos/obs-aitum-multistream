# WEST Gaming Aitum Multistream for OBS Studio

A WEST Gaming-maintained fork of [Aitum Multistream](https://github.com/Aitum/obs-aitum-multistream). This repository owns the Main Canvas destination workflow and Twitch Enhanced Broadcasting (EBC), and provides the Vertical Canvas destination/TikTok configuration bridge.

**Current target — 27 November 2026:** OBS Studio 33.0.3 · Aitum Multistream 1.0.10 · Windows x64.

## Component ownership

This repository owns:
- Main Canvas destinations and their normal Aitum output lifecycle.
- Twitch Enhanced Broadcasting negotiation, track/encoder creation and the Twitch EBC output lifecycle.
- The Vertical Canvas destination list, configuration handoff and start/stop controls.
- A thin TikTok destination wizard bridge.

The matching [WEST Gaming Vertical Canvas fork](https://github.com/westkerberos/obs-vertical-canvas) owns:
- Vertical scenes/canvas and vertical destinations (YouTube, Instagram, TikTok).
- Reuse of Twitch EBC's portrait video encoder by opted-in vertical targets.
- TikTok account/session, cookies/QR, signing, dynamic ingest, LIVE metadata/safety and Dual LIVE runtime.

Do not implement a second TikTok session/signing path in Multistream. Do not treat either plugin as a standalone update; build/install/test the pair together.

## Current merged integration

- This repository: Aitum Multistream **1.0.10**, merged into `main` via [PR #2](https://github.com/westkerberos/obs-aitum-multistream/pull/2), merge commit `6d4c0922f60ddd0012778c69dc7584a0cef55d15`. The latest documentation head is recorded in the source-of-truth.
- Post-merge Windows x64 CI on `main`: [#46036486335](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/46036486335) succeeded; artifact `aitum-multistream-1.0.10-windows-x64-6d4c0922f`.
- Required companion: [WEST Vertical Canvas 1.6.8 integration branch](https://github.com/westkerberos/obs-vertical-canvas/tree/main), Windows x64 CI [#46034312506](https://github.com/westkerberos/obs-vertical-canvas/actions/runs/46034312506).

The end-to-end architecture, version matrix, test evidence and operator start order are maintained in the [WEST Gaming Streaming Stack source of truth](https://github.com/westkerberos/obs-vertical-canvas/blob/main/docs/WEST-GAMING-STREAMING-STACK.md). The reusable update checklist is [here](https://github.com/westkerberos/obs-vertical-canvas/blob/main/docs/UPDATE-PROCEDURE.md).

## Operator start order

1. Start Twitch EBC and wait for the EBC tracks to initialize.
2. Start YouTube Vertical.
3. Start YouTube Horizontal/Main Canvas.
4. Start Instagram Vertical.
5. Start TikTok Dual LIVE.

Twitch EBC supplies the portrait video encoder reused by opted-in Vertical Canvas targets. TikTok Landscape separately needs an active OBS Main Canvas streaming output. Twitch being active alone does not satisfy that prerequisite; Vertical Canvas displays a missing-prerequisite warning.

## Upstream Aitum 1.0.10 changes integrated

The WEST fork contains both relevant official Aitum 1.0.10 changes:
- Reload Vertical Canvas destinations after OBS finishes loading.
- Use the OBS 33 ProgramData plugin installer layout.

The fork retains WEST's EBC implementation and TikTok bridge. Do not replace this fork with the official Aitum 1.0.10 DLL.

## Documentation

- [EBC and complete stack integration](docs/TWITCH-EBC-AND-STACK-INTEGRATION.md)
- [TikTok target bridge](docs/TIKTOK-TARGET.md)
- [Fork differences](docs/FORK-DIFFERENCES.md)
- [Build and paired test status](docs/BUILD-AND-TESTING.md)
- [WEST full-stack source of truth](https://github.com/westkerberos/obs-vertical-canvas/blob/main/docs/WEST-GAMING-STREAMING-STACK.md)

## Distribution status

This is a development/integration fork, not a stable public WEST Gaming release. The TikTok-native implementation in the companion Vertical Canvas repository still has unresolved source-license/provenance questions. Do not publish a combined binary or claim license clearance until those issues are remediated or independently cleared. This is not legal advice.
