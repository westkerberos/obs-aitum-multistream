# WEST Gaming — Aitum Multistream for OBS Studio

A WEST Gaming-maintained fork of [Aitum Multistream](https://github.com/Aitum/obs-aitum-multistream), retaining its multiple-output dock and adding a **TikTok destination option for Vertical Canvas outputs**.

This repository is the Multistream-side configuration bridge. It is **not** the TikTok account/session/signing implementation. Those responsibilities live in the matching [WEST Gaming Vertical Canvas fork](https://github.com/westkerberos/obs-vertical-canvas).

> **Development status — 17 November 2026:** the TikTok destination integration is in [PR #1](https://github.com/westkerberos/obs-aitum-multistream/pull/1), branch `feature/tiktok-vertical-target`. The default `main` branch does not contain these changes until the PR is merged. There is no GitHub Release for this fork yet.

## Who should use this fork

Use this project if you already use **Aitum Multistream together with Vertical Canvas** and want TikTok to appear as a named Vertical Canvas destination in the Multistream settings/dock workflow.

Do not install this plugin by itself expecting a working TikTok integration. The matching WEST Gaming [obs-vertical-canvas](https://github.com/westkerberos/obs-vertical-canvas) plugin is required and owns the entire TikTok runtime. This bridge does not add TikTok as the ordinary OBS Main Canvas streaming provider; it is specifically for Vertical Canvas destinations.

## What this fork adds

The existing Aitum Multistream dock continues to manage multiple output destinations. WEST Gaming adds a TikTok choice to the **Vertical Canvas → Add Output** workflow.

For a TikTok destination, Multistream:
1. Collects the destination name and identifies the provider as TikTok.
2. Calls the OBS proc-handler bridge exported by the matching Vertical Canvas plugin.
3. Opens Vertical Canvas's native TikTok connection dialog.
4. Saves the provider settings with the vertical destination and restores them when that destination is edited.
5. Keeps using the normal Multistream dock to present and start/stop the destination.

TikTok connection methods available through the bridge are Custom RTMP, TikTok LIVE Studio Desktop, TikTok LIVE Studio Mobile and Streamlabs Desktop.

## Required companion plugin

The TikTok configuration UI is provided by the WEST Gaming Vertical Canvas fork. Install compatible versions of **both** plugins:

- [WEST Gaming obs-aitum-multistream](https://github.com/westkerberos/obs-aitum-multistream) — this repository.
- [WEST Gaming obs-vertical-canvas](https://github.com/westkerberos/obs-vertical-canvas) — owns the TikTok dialog and runtime.

Vertical Canvas exposes the bridge call `aitum_vertical_configure_tiktok_output`. If Vertical Canvas is missing or too old to expose it, Multistream cannot configure the native TikTok destination and shows an update/availability warning.

The matching Vertical Canvas development branch is `feature/tiktok-native-output`; its current implementation and automatic Dual LIVE behavior are described in [its TikTok guide](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-output/docs/TIKTOK-LIVE.md).

## Add a TikTok destination

1. Install and enable both matching WEST Gaming plugins.
2. Open the Aitum Multistream dock and its settings.
3. Open the **Vertical Canvas** settings page.
4. Choose **Add Output** and select **TikTok**.
5. Enter a destination name and select **Configure TikTok connection…**.
6. In the dialog supplied by Vertical Canvas, choose the connection method and provide its credentials:
   - **Custom RTMP / stream key:** enter the endpoint and key supplied by the provider.
   - **TikTok LIVE Studio — Desktop:** RapidAPI key plus a TikTok cookie file, or QR login.
   - **TikTok LIVE Studio — Mobile:** RapidAPI key plus a TikTok cookie file, or QR login; configure the mobile client/device profile.
   - **Streamlabs Desktop:** Streamlabs Desktop access token.
7. Configure LIVE title, topic/game and other options in the Vertical Canvas dialog. Save the destination.
8. Start the TikTok destination from the Aitum Multistream dock.

For native TikTok methods, the real server URL and stream key are obtained dynamically by Vertical Canvas when the output starts. They are not values that the user needs to paste into the Multistream service form. Custom RTMP remains a manually supplied endpoint/key.

See [TikTok target configuration and troubleshooting](docs/TIKTOK-TARGET.md) for details.

## Dual LIVE

Dual LIVE is configured and run by Vertical Canvas, not by the Multistream bridge. The matching Vertical Canvas build provides a default reuse mode:

- Portrait inherits the encoders already assigned to the Vertical Canvas TikTok destination.
- Landscape inherits the active OBS Main Canvas streaming encoders.
- The Main Canvas stream must be active before TikTok Dual LIVE starts.
- An advanced manual override remains available in Vertical Canvas.

Multistream only identifies the target as TikTok and opens the configuration dialog. It does not create a second TikTok session, signing worker or encoder set. The paired-feed behavior and restrictions are documented in [Vertical Canvas's Dual LIVE guide](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-output/docs/TIKTOK-LIVE.md#dual-live-portrait--landscape).

## What remains from upstream

This fork retains Aitum Multistream's existing destination management and normal platform-output workflow. The WEST Gaming change is scoped to vertical destination creation/editing and the TikTok configuration bridge. Main Canvas output creation/editing remains on the original path.

Do not install this fork by itself expecting it to add standalone TikTok support to the Main Canvas stream settings. It is designed to cooperate with the matching Vertical Canvas fork.

## Compatibility, build and release status

- Both plugins must be installed and loaded in OBS.
- The Multistream bridge PR builds Windows x64. macOS and Linux jobs are intentionally skipped in this fork's workflow.
- The latest verified bridge build and artifact are listed in [Build and testing](docs/BUILD-AND-TESTING.md).
- The matching Vertical Canvas runtime has its own build and test status. A green Multistream build does not prove the TikTok API/runtime is available; see [Vertical Canvas build/testing notes](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-output/docs/BUILD-AND-TESTING.md).
- There is no GitHub Release published for this repository yet. CI artifacts are test packages, not a stable release channel.
- Unsigned Windows test packages are expected while signing secrets are unavailable.

## Build

The standalone CMake target links OBS libobs, obs-frontend-api, Qt 6 Widgets/Core and libcurl. With the required OBS development packages available:

```sh
cmake -S . -B build -DBUILD_OUT_OF_TREE=ON
cmake --build build --config Release
```

See [the fork-differences guide](docs/FORK-DIFFERENCES.md) for the exact bridge responsibilities and inherited behavior.

## Security and issue reports

TikTok credentials are handled by Vertical Canvas, not by a separate Multistream signing implementation. Never attach cookie files, RapidAPI keys, Streamlabs tokens, unredacted OBS profiles or unredacted logs to issues.

When reporting a problem, include both plugin commit/build identifiers, OBS version, whether the output is Main Canvas or Vertical Canvas, selected TikTok method, and a redacted OBS log excerpt.