# Licensing and distribution notices

The Aitum Multistream fork's root `LICENSE` is GNU GPL version 2. The same license text is copied into `data/licenses/GPL-2.0.txt` so the plugin data package can carry the main license notice with the installed plugin.

This repository's TikTok addition is a thin configuration bridge to the matching WEST Gaming Vertical Canvas plugin. It does not copy the TikTok-native session, signing, safety, account or Dual LIVE implementation into this repository. The relevant third-party TikTok-native attribution and GPL-3.0-only notice are maintained in the companion [Vertical Canvas repository](https://github.com/westkerberos/obs-vertical-canvas/tree/feature/tiktok-native-output/data/licenses).

Do not present this bridge as a standalone TikTok runtime. Keep the upstream Aitum notices and the companion plugin's notices intact. Review the license status of the companion integration separately before distributing a combined installer or bundle.

## Windows package verification

The standard Windows ZIP from workflow [#44153195398](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/44153195398), artifact `aitum-multistream-1.0.9-windows-x64-9dc39805d` (SHA-256 `69c79203198a3b4b67c2fca510e2e17896915b9f71a5504c76742e46c9e4155b`), was inspected. It contains the main license text at `data/obs-plugins/aitum-multistream/licenses/GPL-2.0.txt`.
