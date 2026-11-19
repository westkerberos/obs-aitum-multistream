# WEST Gaming Aitum Multistream for OBS Studio

A WEST Gaming-maintained fork of [Aitum Multistream](https://github.com/Aitum/obs-aitum-multistream). The project adds a TikTok destination to the **Vertical Canvas** output workflow through a small configuration bridge.

**Status: experimental development — no stable WEST Gaming release is published.**

The TikTok destination work is in [Pull Request #1](https://github.com/westkerberos/obs-aitum-multistream/pull/1), branch `feature/tiktok-vertical-target`. The branch builds for Windows x64. It must be paired with the matching [WEST Gaming Vertical Canvas fork](https://github.com/westkerberos/obs-vertical-canvas), which owns the TikTok dialog and runtime.

## What this fork adds

- TikTok as a selectable destination under **Vertical Canvas → Add Output**.
- A settings handoff to Vertical Canvas through the OBS proc-handler bridge `aitum_vertical_configure_tiktok_output`.
- Saved destination settings that can be restored when a TikTok destination is edited.
- The existing Multistream dock remains responsible for displaying and starting/stopping the destination.

This repository is **not** a standalone TikTok implementation. It does not own TikTok cookies, session creation, request signing, RTMP ingest, safety monitoring or Dual LIVE. Those are the responsibility of the companion Vertical Canvas integration.

## Required companion

Install compatible builds of both plugins when a release is available:

1. [WEST Gaming Aitum Multistream](https://github.com/westkerberos/obs-aitum-multistream) — this bridge.
2. [WEST Gaming Vertical Canvas](https://github.com/westkerberos/obs-vertical-canvas) — TikTok configuration and runtime.

The original OBS Multi-RTMP plugin is not required to be enabled for this integration.

## Release and compatibility status

There is no stable GitHub Release or supported installer for this fork yet. The default `main` branch does not contain the TikTok bridge until PR #1 is merged. Do not install this repository by itself expecting a working TikTok destination.

The companion Vertical Canvas development branch has unresolved licensing/provenance remediation. A green Multistream build does not clear or validate the companion TikTok runtime. No combined release should be published until the companion's source audit, architecture, packaging and runtime tests are complete.

## OBS native stream encoder (experimental)

This branch also adds an experimental **OBS Native Stream Encoder** source for Aitum Main Canvas destinations. It allows an Aitum output such as YouTube to reuse OBS's conventional stream encoder instead of selecting one of Twitch Enhanced Broadcasting's negotiated tracks. See [OBS native stream encoder setup and verification](docs/OBS-NATIVE-STREAM-ENCODER.md) for behavior and limitations.

## Development links

- [TikTok target setup and troubleshooting](https://github.com/westkerberos/obs-aitum-multistream/blob/feature/tiktok-vertical-target/docs/TIKTOK-TARGET.md)
- [Fork differences and bridge ownership](https://github.com/westkerberos/obs-aitum-multistream/blob/feature/tiktok-vertical-target/docs/FORK-DIFFERENCES.md)
- [Build and testing status](https://github.com/westkerberos/obs-aitum-multistream/blob/feature/tiktok-vertical-target/docs/BUILD-AND-TESTING.md)
- [Matching Vertical Canvas TikTok guide](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-output/docs/TIKTOK-LIVE.md)
- [Upstream Aitum Multistream](https://github.com/Aitum/obs-aitum-multistream)

## Search terms

OBS Studio plugin, Aitum Multistream, TikTok LIVE, TikTok destination, Vertical Canvas, OBS multi-streaming, RTMP, WEST Gaming.
