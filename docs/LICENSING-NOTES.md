# Licensing and distribution notices

The Aitum Multistream fork's root `LICENSE` is GNU GPL version 2. The same license text is copied into `data/licenses/GPL-2.0.txt` so the plugin data package can carry the main license notice with the installed plugin.

This repository's TikTok addition is a thin configuration bridge to the matching WEST Gaming Vertical Canvas plugin. It does not copy or implement the TikTok-native session, signing, safety, account, device-registration or Dual LIVE runtime. It calls the Vertical Canvas configuration bridge and must be used with a matching Vertical Canvas build for the TikTok native feature.

## Companion integration is not currently distribution-cleared

The companion [WEST Vertical Canvas repository](https://github.com/westkerberos/obs-vertical-canvas) is based on Aitum's GPL-2.0 project. Its current TikTok integration contains at least 24 files identified as derived from GPL-3.0-only sources, compiled into the same Vertical Canvas module. The source audit also found device-registration code ported from a public repository with no published LICENSE.

These findings do not describe a problem in this thin Multistream bridge's TikTok UI handoff itself. They do mean that **a paired installer, combined package or recommended two-plugin distribution is not ready to be presented as license-cleared** until the companion Vertical Canvas implementation is remediated and audited. See the companion [licensing notes](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-output/docs/LICENSING-NOTES.md), [source inventory](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-output/docs/LICENSING-INVENTORY.md), and [architecture options](https://github.com/westkerberos/obs-vertical-canvas/blob/feature/tiktok-native-output/docs/LICENSING-ARCHITECTURE-OPTIONS.md). The current preferred experiment is a process-separated TikTok relay communicating over local RTMP/FLV plus a minimal documented control protocol. The OBS plugin and relay must not share DLLs, C++ classes, OBS objects or internal data structures. The relay's dependency tree must also be audited: the existing GPL-3.0 runtime depends on unmarked helper files from the GPL-2.0 Loukious fork, so they cannot simply be copied into the relay. This architecture is only a proposal, not yet implemented or legally cleared.

Do not solve this by replacing either Aitum root LICENSE or by removing third-party notices. Keep the inherited Aitum terms and all applicable notices intact. The current maintainer decision is to replace incompatible or unlicensed portions independently rather than request special relicensing permission.

## Windows package verification

The latest successful Windows x64 bridge build/package is workflow [#44164183646](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/44164183646), commit `71c9dda444407bd62720411ebbb1f12387f03783`, artifact `aitum-multistream-1.0.9-windows-x64-74e7f1c88`. The previously inspected standard ZIP was confirmed to contain the main license text at `data/obs-plugins/aitum-multistream/licenses/GPL-2.0.txt`. Re-inspect the latest artifact's file list before publishing any future release.

This repository has no standalone TikTok runtime and should not be described as a working standalone TikTok integration.
