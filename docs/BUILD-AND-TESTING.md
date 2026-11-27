# Build and verification — WEST Gaming Aitum Multistream

**Status date:** 27 November 2026  
**Target:** OBS Studio 33.0.3 · Windows x64 · Aitum Multistream 1.0.10

## Scope and matched companion

The merged main branch provides Main Canvas destination orchestration, Twitch EBC and the TikTok configuration bridge. It must be paired with [WEST Vertical Canvas 1.6.8](https://github.com/westkerberos/obs-vertical-canvas/tree/main), which owns vertical outputs, the EBC portrait video-encoder reuse and the TikTok runtime/Dual LIVE.

The end-to-end ownership map, encoder routes and paired test matrix are maintained in the [WEST Gaming Streaming Stack source of truth](https://github.com/westkerberos/obs-vertical-canvas/blob/main/docs/WEST-GAMING-STREAMING-STACK.md).

## Build configuration and upstream integration

- Aitum Multistream version: **1.0.10**.
- OBS source dependency: **33.0.3**.
- Windows obs-deps/Qt6 snapshot: **2026-10-23**.
- The Windows installer includes the OBS 33 ProgramData third-party plugin locations.
- The official Aitum 1.0.10 Vertical destination reload fix is present: after OBS finishes loading, Multistream calls LoadSettingsFile() and then LoadVerticalOutputs(false).
- WEST Twitch EBC and TikTok target bridge functionality remain intact.

Do not replace this fork with the official Aitum binary; it does not include the WEST EBC implementation and TikTok bridge. Do not overwrite the OBS 33.0.3 buildspec with upstream's older dependency pins.

## Latest merged Windows x64 CI

- Workflow: [#46035894193](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/46035894193)
- Branch: main
- Commit: 6a5578990456c9c55403ea9cdc51d18d3af3d5b1
- Result: **success** (Windows x64 build/package).
- Artifact: aitum-multistream-1.0.10-windows-x64-6a5578990.
- Ubuntu/macOS and formatting: skipped by the current Windows-only workflow.

This establishes compilation/packaging against OBS 33.0.3 sources. It does not by itself prove Twitch EBC negotiation, track assignment, OBS encoder identity/reuse, TikTok session startup or long-duration stability.

## Runtime evidence from 27 November

The user's 13:01 OBS 33.0.3 log reports Multistream 1.0.10:
- Twitch EBC started and created five H.264 video tracks including a 1080×1920 Vertical Canvas portrait track.
- Vertical Canvas found the active Multistream EBC output and selected/reused the 1080×1920 encoder.
- YouTube horizontal/Main Canvas started.
- TikTok Portrait and Landscape connected and remained active until the operator stopped them; the log contains no TikTok RTMP send errors in that final session.

This was a short runtime session using Vertical Canvas 1.6.7, not the new 1.6.8 build. The paired 1.6.8 + 1.0.10 runtime regression still needs to be recorded.

## Functional ownership to keep intact

- Main Canvas regular destinations continue using the original Aitum path.
- Twitch EBC is negotiated and started here; it includes Main Canvas and, when available, the portrait Vertical Canvas.
- Vertical Canvas destination list and start/stop actions remain refreshed after OBS startup.
- TikTok target setup is delegated through proc-handler aitum_vertical_configure_tiktok_output.
- TikTok LIVE session, signing, dynamic ingest, safety and Dual LIVE belong exclusively to Vertical Canvas.
- Vertical Canvas opts into the EBC portrait video encoder using the reserved selector __aitum_twitch_ebc_vertical__.
- Recording, replay and virtual-camera paths remain separate from stream encoder selection.

## Paired regression tests

1. Clean OBS 33.0.3 standard and portable installations: verify one copy of each matching plugin loads from the intended plugin folder.
2. Main Canvas normal outputs and Twitch EBC.
3. EBC request includes both Main Canvas and Vertical Canvas portrait canvas.
4. Verify negotiated encoders/track resolutions and that Twitch's landscape + portrait outputs both start/stop.
5. Verify each opted-in vertical output is attached to the same OBS video encoder object as the EBC portrait track.
6. Verify Vertical Canvas destination list refresh after OBS startup, profile changes, restart and saved target edit.
7. Verify TikTok configuration handoff and that the bridge does not duplicate TikTok session/runtime.
8. Exercise the sequence Twitch EBC → YouTube Vertical → YouTube Horizontal → Instagram Vertical → TikTok Dual LIVE.
9. Disconnect one output and ensure unrelated destinations remain active; verify reconnect and stop callbacks are safe.
10. Test profile save/reload, repeated start/stop and long-duration runtime on the exact paired commits.

## Release gates

- [x] OBS 33.0.3 Windows x64 build/package.
- [x] Aitum 1.0.10 version/manifest, OBS 33 installer layout and Vertical destination reload fix.
- [x] TikTok configuration bridge and Twitch EBC implementation present in source.
- [x] Short-session runtime log shows EBC + Main Canvas + TikTok Dual LIVE working with the previous Vertical Canvas 1.6.7 build.
- [ ] Paired runtime validation with the exact Vertical Canvas 1.6.8 build.
- [ ] Extended Twitch EBC/TikTok soak and failure/reconnect test.
- [ ] Source-license/provenance blockers in the companion TikTok runtime independently cleared or remediated before public binary distribution.

No stable combined WEST Gaming binary release is claimed.
