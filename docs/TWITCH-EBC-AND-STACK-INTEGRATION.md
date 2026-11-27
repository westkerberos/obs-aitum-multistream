# WEST Gaming Aitum Multistream — EBC and stack integration

**Status date:** 27 November 2026  
**Current source:** feature/aitum-twitch-enhanced-broadcasting · Aitum Multistream 1.0.10 · OBS 33.0.3  
**Windows x64 CI:** [#45899264700](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/45899264700) — success

Maintain this repository together with the matching [WEST Vertical Canvas 1.6.8 update branch](https://github.com/westkerberos/obs-vertical-canvas/tree/release/obs33.0.3-vertical-1.6.8-west-stack-20261127). The end-to-end architecture and test sequence live in the [WEST Gaming Streaming Stack source of truth](https://github.com/westkerberos/obs-vertical-canvas/blob/release/obs33.0.3-vertical-1.6.8-west-stack-20261127/docs/WEST-GAMING-STREAMING-STACK.md).

## Responsibilities

This repository owns:
- Main Canvas destination configuration and output lifecycle.
- The Vertical Canvas destination list and start/stop bridge.
- Twitch Enhanced Broadcasting request/negotiation, encoder group creation and Twitch output lifecycle.
- The TikTok target wizard/configuration bridge to Vertical Canvas.

Vertical Canvas owns:
- Portrait canvas rendering, scenes and vertical output sessions.
- EBC portrait video-encoder lookup/reuse for opted-in vertical outputs.
- TikTok credentials/cookies, account/session, QR, signing, dynamic ingest, LIVE safety and Dual LIVE.

Do not move TikTok session/auth/signing/transport into Multistream or create a second TikTok implementation here.

## Twitch EBC data flow

When an EBC-enabled Twitch destination starts, Multistream:
1. Validates Twitch destination settings.
2. Requests Twitch client/track configuration.
3. Includes Main Canvas and, when available, the Vertical Canvas portrait canvas.
4. Creates the negotiated video/audio encoder track group.
5. Creates a dedicated output named with the prefix aitum_multi_ebc_output_<destinationName>.
6. Retains encoder/group/canvas references for the output lifetime and tears them down on stop.

The ordinary OBS Main Canvas stream is not converted into the EBC output; EBC is a separate Multistream-managed Twitch output.

## Vertical destinations and portrait encoder reuse

Vertical Canvas searches active OBS outputs for the EBC output prefix, inspects attached video encoders, matches the current portrait canvas/compatible portrait ratio and chooses the highest-resolution matching track.

The reserved Vertical Canvas encoder selection is __aitum_twitch_ebc_vertical__. When this is selected, the output should reuse the exact negotiated EBC portrait encoder object; it must not create another independent video encoder when a valid match exists.

If EBC is unavailable, Vertical Canvas now prompts the user to start Twitch first or explicitly continue with an independent vertical encoder. That choice is intentional because an independent encoder increases encoder load. This pre-flight is implemented for individual start and Start All.

## TikTok target bridge

The vertical output wizard selects TikTok and stores provider-specific settings. Multistream invokes the Vertical Canvas proc-handler aitum_vertical_configure_tiktok_output to open the configuration dialog. Vertical Canvas persists provider settings and owns the actual native TikTok session/runtime, dynamic RTMP endpoints, safety and Dual LIVE. Multistream must not own TikTok cookies, signing keys, remote endpoints or separate media/session logic.

## Upstream Aitum 1.0.10 changes integrated

- Aitum commit [64ef323](https://github.com/Aitum/obs-aitum-multistream/commit/64ef3233a8b078268adf68ec8427d0bfba68678d): after OBS FINISHED_LOADING, load settings then refresh vertical destinations through LoadVerticalOutputs(false).
- Aitum commit [41329c5](https://github.com/Aitum/obs-aitum-multistream/commit/41329c56d8f1254b7b55a2821a44b08abeeab3bb): bump Aitum version/manifest to 1.0.10 and use OBS 33 ProgramData Windows installer paths.

The WEST fork's buildspec pins OBS 33.0.3 and 2026-10-23 dependencies. Do not replace those pins with the upstream repo's older baseline.

## Runtime start order and dependencies

- Start Twitch EBC and wait for its portrait/landscape tracks to initialize.
- Start opted-in Vertical Canvas outputs so they can reuse the EBC portrait video encoder.
- Start a horizontal Main Canvas stream before TikTok Dual LIVE; the TikTok Landscape feed reuses the active Main Canvas video/audio encoders.
- Start TikTok Dual LIVE only after both prerequisites are satisfied.

Twitch being active alone is not enough for TikTok Landscape. Main Canvas being active alone is not enough for vertical targets configured to reuse EBC.

## Verification and maintenance

Compile/package success is established by Windows CI #45899264700. The 27 Nov 13:01 log shows EBC track creation and TikTok Dual LIVE working with the previous Vertical Canvas 1.6.7 pair. Exact 1.6.8 paired runtime validation, multi-destination soak, disconnect/reconnect and the complete test matrix remain in [Build and verification](BUILD-AND-TESTING.md).

See [the full update procedure](https://github.com/westkerberos/obs-vertical-canvas/blob/release/obs33.0.3-vertical-1.6.8-west-stack-20261127/docs/UPDATE-PROCEDURE.md). Keep output names, reserved encoder marker, proc-handler API names, settings keys and encoder lifetimes synchronized across both repositories.

## License and release status

The companion Vertical Canvas TikTok integration still has unresolved source-license/provenance questions. Do not publish a combined binary or claim license clearance until these issues are remediated or independently cleared. This is not legal advice.
