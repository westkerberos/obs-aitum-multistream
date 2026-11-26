# Build and verification — WEST Gaming Aitum Multistream EBC fork

## Scope and matched companion

This guide applies to the WEST Gaming branch `feature/aitum-twitch-enhanced-broadcasting`, [PR #2](https://github.com/westkerberos/obs-aitum-multistream/pull/2). It is the Multistream component of a matched system with:

- Vertical Canvas: `feature/tiktok-native-ebc-combined`, [PR #6](https://github.com/westkerberos/obs-vertical-canvas/pull/6).
- Full architecture, encoder routing and regression plan: [WEST Gaming Streaming Stack — Source of Truth](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-ebc-combined/docs/WEST-GAMING-STREAMING-STACK.md).

Do not validate or release this repository in isolation. It owns Main Canvas output orchestration and Twitch EBC; Vertical Canvas owns vertical outputs, EBC portrait video-encoder reuse and the native TikTok runtime.

## Build configuration mismatch

The current fork `buildspec.json` still declares Multistream **1.0.9** and OBS source dependency **31.1.0**. The user target is OBS 33.0.3. A successful CI result using the existing buildspec does not establish OBS 33.0.3 compatibility.

Before claiming support:
- Pin the OBS source/dependency and Windows build toolchain to OBS 33.0.3's build requirements.
- Port Aitum Multistream 1.0.10 changes without replacing the WEST EBC/TikTok code.
- Port the OBS 33 third-party plugin install layout.
- Build this branch with the matching Vertical Canvas commit.
- Run the paired test plan in the source-of-truth document.

## Latest known Windows CI

- Workflow: [#44808003981](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/44808003981)
- Branch: `feature/aitum-twitch-enhanced-broadcasting`
- Commit: `8f7780ded10f0afa43a35b02f117e8f95567b2c3`
- Windows x64 build: **success**.
- Ubuntu/macOS: **skipped** by the Windows-only fork workflow.
- Formatting: **skipped**.
- Build baseline: existing fork buildspec, which pins OBS 31.1.0.
- This run validates compilation/package only. It does not prove Twitch EBC negotiation, simultaneous Twitch landscape+portrait output, encoder object reuse, TikTok session/runtime or long-duration stability in OBS 33.0.3.

## Required upstream Aitum 1.0.10 port

Our current fork is still 1.0.9 and lacks both changes below:

1. Upstream [commit 64ef323](https://github.com/Aitum/obs-aitum-multistream/commit/64ef3233a8b078268adf68ec8427d0bfba68678d) calls `LoadVerticalOutputs(false)` after `LoadSettingsFile()` during `OBS_FRONTEND_EVENT_FINISHED_LOADING`. Port this so the Vertical Canvas target list is refreshed after OBS finishes loading, preserving EBC and TikTok bridge code.
2. Upstream [commit 41329c5](https://github.com/Aitum/obs-aitum-multistream/commit/41329c56d8f1254b7b55a2821a44b08abeeab3bb) bumps version to 1.0.10 and changes the Windows installer to the new OBS 33 third-party plugin folder layout. Port the installer change.

Do not install the official Aitum 1.0.10 binary over this fork. It does not contain WEST Gaming's custom EBC implementation and TikTok destination bridge.

## Functional ownership to keep intact

- Main Canvas regular destinations remain supported through the upstream Aitum path.
- Twitch EBC is negotiated and started by Multistream; when Vertical Canvas is available, the Twitch request includes both Main Canvas and portrait canvas.
- EBC uses a dedicated Multistream-managed Twitch output and track group.
- Vertical Canvas targets are displayed and managed by the existing Multistream dock.
- TikTok configuration is handed to Vertical Canvas through `aitum_vertical_configure_tiktok_output`; the TikTok runtime and Dual LIVE do not belong in this repository.
- Vertical Canvas opts destinations into the EBC portrait video encoder via `__aitum_twitch_ebc_vertical__`. The actual encoder attach/lookup/fallback is implemented in the companion Vertical Canvas plugin.

## Paired regression tests

1. OBS 33.0.3 clean portable install: verify only one copy of each matched plugin loads from the new plugin path.
2. Main Canvas outputs: Twitch EBC and ordinary YouTube/Kick/other destination paths.
3. Twitch EBC: verify landscape and portrait track enumeration, requested capabilities, resolved encoders and output startup/stop.
4. Vertical Canvas destinations: verify start/stop individually and Start All.
5. Confirm each opted-in vertical destination references the *same OBS video encoder object* as the negotiated Twitch portrait track. Test the independent vertical fallback when EBC is absent and explicit per-destination encoder override.
6. Verify Vertical Canvas destinations reload after OBS startup, profile changes and restart; edit/save targets without losing provider settings.
7. TikTok target wizard: create/edit/save and delegate the configuration dialog to Vertical Canvas. Then exercise TikTok runtime using the companion test plan.
8. Disconnect one output; make sure unrelated destinations remain active and repeated stops/reconnects do not duplicate outputs or encoders.
9. Close and reopen OBS, repeat at least ten start/stop cycles, then perform a long soak test.

Record exact OBS build, both plugin commits, DLL hashes, GPU driver and redacted logs. Do not publish a stable release based solely on CI.

## Release gates

- [ ] OBS 33.0.3 Windows build succeeds from the updated buildspec.
- [ ] Aitum 1.0.10 vertical-output reload fix is ported.
- [ ] OBS 33 new plugin install layout is included and tested.
- [ ] Matching Vertical Canvas branch builds with the same OBS baseline.
- [ ] Twitch landscape + portrait EBC works with both canvases.
- [ ] All selected Vertical destinations reuse the correct EBC portrait video encoder.
- [ ] TikTok destination configuration bridge and persistence work.
- [ ] Full TikTok runtime/Dual LIVE is validated in the companion plugin.
- [ ] Disconnect, restart, shutdown and long-running tests pass.
- [ ] Companion TikTok source-license issues are remediated or independently cleared.

No stable WEST Gaming release or combined installer exists yet.
