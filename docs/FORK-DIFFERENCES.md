# WEST Gaming fork differences

## Upstream project

This repository is based on [Aitum/obs-aitum-multistream](https://github.com/Aitum/obs-aitum-multistream). The fork retains the upstream Multistream dock and target-management workflow.

## WEST Gaming additions

The TikTok destination integration is in [PR #1](https://github.com/westkerberos/obs-aitum-multistream/pull/1), branch `feature/tiktok-vertical-target`.

### Output wizard changes

- The output dialog receives context indicating whether the destination is for the Vertical Canvas path.
- TikTok is selectable in the Vertical Canvas output wizard.
- TikTok targets are marked with `tiktok_target` and retain provider-specific configuration fields.
- Creating/editing a native TikTok target opens the configuration dialog exposed by Vertical Canvas.
- Dynamic TikTok methods do not require the user to paste a real server/key into the wizard; those values are acquired at output startup by Vertical Canvas.
- Custom RTMP continues to use the ordinary manual server/key fields.

### Bridge contract

Multistream calls the OBS proc-handler function:

```text
aitum_vertical_configure_tiktok_output
```

The call passes a parent widget, an OBS settings object and destination name; Vertical Canvas returns whether the dialog was accepted. If the handler is missing, Multistream reports that Vertical Canvas is unavailable or outdated.

The bridge is intentionally thin. It does not add:
- TikTok API requests or hosted signing.
- TikTok account/device/QR login code.
- LIVE creation, heartbeat or cleanup.
- LIVE violation monitoring or appeals.
- Portrait/landscape encoder creation or the second Dual LIVE transport.

Those capabilities belong to the matching Vertical Canvas fork.

### Scope retained from upstream

- The existing Main Canvas output creation/edit path remains on the upstream flow.
- Other platform targets continue to use the normal output wizard and service fields.
- The existing Multistream dock remains responsible for presenting targets and handling the user's start/stop action.

## Source files changed by the bridge PR

The PR touches the following areas:

- `.github/workflows/*.yaml`: this fork's Windows-only CI dispatch.
- `config-dialog.cpp`: Vertical Canvas destination creation/edit stores TikTok-specific settings with the target.
- `output-dialog.cpp` and `output-dialog.hpp`: provider selection, edit-mode restore, configuration button and proc-handler bridge call.
- `multistream.cpp`: TikTok target identity/icon handling in the vertical output dock.

The TikTok session, UI, output and safety code is intentionally not copied into this repository.

## Exact upstream scope

The fork is based on [Aitum/obs-aitum-multistream](https://github.com/Aitum/obs-aitum-multistream). As of 17 November 2026, the WEST Gaming PR contains **14 changed files, 906 additions and 35 deletions** relative to this fork's `main`.

The source-code additions are intentionally small compared with the Vertical Canvas TikTok implementation. The key changes are the Vertical Canvas-aware target creation/edit path in `config-dialog.cpp`, TikTok wizard and proc-handler bridge in `output-dialog.cpp/.hpp`, and TikTok target identification in `multistream.cpp`, plus Windows-only workflow configuration and this documentation.

A comparison against Aitum's upstream `main` shows the feature branch ahead and not behind. The exact ahead count includes inherited branch history and changes as documentation commits are added; the PR file list is the more useful measure of the actual Multistream bridge change.

## Build and test status

The bridge PR's Windows x64 build and package passed at commit `f72dec7f09cbba53dff1f669f9df1737cfef3eed`: [Actions run #43772925061](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/43772925061).

A separate successful Multistream build is not proof of the Vertical Canvas runtime. The latter must be installed and tested as a paired component. The matching Vertical Canvas PR and current runtime/build notes are linked from the [main README](../README.md).

This repository has no GitHub Release as of 17 November 2026. The feature branch must be merged and a release package created before the integration can be presented as a stable public release.
