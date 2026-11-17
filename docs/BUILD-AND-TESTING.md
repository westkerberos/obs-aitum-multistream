# Build and verification — WEST Gaming Aitum Multistream bridge

## Scope

This guide applies to the WEST Gaming fork of Aitum Multistream on branch `feature/tiktok-vertical-target`. It covers the Multistream-side TikTok destination bridge only. The matching WEST Gaming [Vertical Canvas fork](https://github.com/westkerberos/obs-vertical-canvas) owns the TikTok connection dialog and runtime.

## Required companion

Install compatible versions of both plugins:

1. WEST Gaming `obs-vertical-canvas`, branch `feature/tiktok-native-output`.
2. WEST Gaming `obs-aitum-multistream`, branch `feature/tiktok-vertical-target`.

The bridge is the OBS proc-handler function `aitum_vertical_configure_tiktok_output`. If Vertical Canvas is not loaded or does not expose that function, the TikTok target configuration dialog cannot open. Updating Multistream alone cannot supply the missing TikTok runtime.

## Build configuration

The standalone CMake target links OBS libobs, OBS frontend API, Qt 6 Core/Widgets and libcurl. The repository's workflow is configured to build Windows only.

With the OBS development files and toolchain installed:

```sh
cmake -S . -B build -DBUILD_OUT_OF_TREE=ON
cmake --build build --config Release
```

The repository uses the upstream Aitum build/package workflow adapted for this fork. macOS and Ubuntu jobs are intentionally skipped by the current fork workflow.

## CI and package status

Latest Windows x64 build/package, including the companion requirement and license notice, passed:

- Workflow: [#44159148944](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/44159148944)
- Commit: `455c0bf95e72de242cd66cc942f94c2788b3a80f`
- Artifact: `aitum-multistream-1.0.9-windows-x64-ddb120a0c`
- The latest Actions artifact is available from the linked workflow run. The previously inspected standard ZIP included `data/obs-plugins/aitum-multistream/licenses/GPL-2.0.txt`; verify the current artifact contents again before publishing a release.
- Signing: skipped because the signing secrets are not configured.
- Linux/macOS: not built by this fork's workflow.

## What this build proves

A successful Multistream CI run proves that the bridge plugin compiles and packages for the configured Windows x64 target. It does not prove the TikTok LIVE API, cookie session, hosted signing, room creation or Dual LIVE runtime independently; those are implemented in Vertical Canvas.

The repository owner has tested the matching plugin pair in OBS, including the TikTok destination and automatic Dual LIVE workflow. This maintainer test does not establish a broad hardware or OS support matrix.

## Release checklist

- [ ] Merge the companion Vertical Canvas PR and this bridge PR together or coordinate compatible release commits.
- [ ] Keep the proc-handler name/signature synchronized across repositories.
- [ ] Install/test both plugins in a clean OBS profile.
- [ ] Verify add/edit/persistence of a TikTok Vertical Canvas target.
- [ ] Verify normal target start/stop delegates to Vertical Canvas.
- [ ] Verify the bridge reports a useful warning when Vertical Canvas is absent/outdated.
- [ ] Decide whether Windows code signing will be configured.
- [ ] Publish a GitHub Release only after matching versions and tested assets are available.

## Troubleshooting

| Symptom | Likely cause | Action |
| --- | --- | --- |
| TikTok connection dialog is unavailable | Missing/outdated Vertical Canvas plugin | Install the matching Vertical Canvas build and restart OBS. |
| TikTok target appears in the wizard but cannot start | The TikTok runtime failed in Vertical Canvas | Inspect the Vertical Canvas error and redacted OBS log. |
| Native TikTok target has no pasted server/key | Expected for dynamic LIVE Studio/Mobile/Streamlabs methods | Vertical Canvas obtains the endpoint/key when the output starts. |
| Automatic Dual LIVE fails for missing Main Canvas encoders | The OBS Main Canvas stream is not active | Start the horizontal OBS stream before starting TikTok Dual LIVE. |

Never attach cookies, RapidAPI keys, Streamlabs tokens or an unredacted OBS profile to an issue.
