# WEST Gaming Aitum Multistream — fork differences

**Status date:** 27 November 2026  
**Current branch:** main · Aitum Multistream 1.0.10 · OBS 33.0.3

## Upstream project

This repository is based on [Aitum/obs-aitum-multistream](https://github.com/Aitum/obs-aitum-multistream). The WEST fork retains its Main Canvas dock/output workflow and adds Twitch EBC plus a Vertical Canvas bridge.

## WEST Gaming additions

### Twitch Enhanced Broadcasting

- Requests Twitch track configuration and negotiates encoder tracks for Main Canvas and, when available, the Vertical Canvas portrait canvas.
- Creates a separate EBC-managed Twitch RTMP output and retains its encoder-group/canvas references through output lifetime.
- Publishes output under the name prefix aitum_multi_ebc_output_ so the companion Vertical Canvas plugin can discover the active EBC output.
- Keeps track/encoder setup and cleanup inside Multistream; Vertical Canvas does not negotiate a second Twitch EBC session.

### TikTok target bridge

- TikTok is selectable in the Vertical Canvas destination workflow.
- Provider-specific settings are persisted with the target.
- Multistream calls aitum_vertical_configure_tiktok_output to open/save the Vertical Canvas TikTok configuration dialog.
- The actual session, authentication, signing, endpoints, media output and Dual LIVE remain in Vertical Canvas.

### Vertical output reload

After OBS fires OBS_FRONTEND_EVENT_FINISHED_LOADING, Multistream loads its settings and then calls LoadVerticalOutputs(false) so the Vertical Canvas target list is refreshed after OBS startup.

## Scope retained from upstream

- Main Canvas output creation/edit and ordinary RTMP destinations remain on the upstream Aitum path.
- The existing Multistream dock remains responsible for destination presentation and start/stop action.
- TikTok is not a second provider runtime inside this repository.

## Upstream Aitum 1.0.10 port

- Commit 64ef323 adds the post-load Vertical Canvas destination refresh.
- Commit 41329c5 bumps version/manifest to 1.0.10 and adds the OBS 33 ProgramData Windows installer layout.
- WEST buildspec remains pinned to OBS 33.0.3 and 2026-10-23 deps/Qt; do not replace with upstream's older dependency baseline.

## Build and test status

Windows x64 CI [#45899264700](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/45899264700) passed at commit 6a5578990456c9c55403ea9cdc51d18d3af3d5b1. The 27 Nov user log shows Twitch EBC track creation and the paired TikTok feeds working with the previous Vertical Canvas 1.6.7 build. The exact companion 1.6.8 runtime test and longer soak remain pending.

See the [WEST Gaming full-stack source of truth](https://github.com/westkerberos/obs-vertical-canvas/blob/main/docs/WEST-GAMING-STREAMING-STACK.md).
