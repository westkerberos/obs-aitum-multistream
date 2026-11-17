# Licensing and distribution notices

The Aitum Multistream fork's root `LICENSE` is GNU GPL version 2. The same license text is copied into `data/licenses/GPL-2.0.txt` so the plugin data package can carry the main license notice with the installed plugin.

This repository's TikTok addition is a thin configuration bridge to the matching WEST Gaming Vertical Canvas plugin. It does not copy the TikTok-native session, signing, safety, account or Dual LIVE implementation into this repository. The relevant third-party TikTok-native attribution and GPL-3.0-only notice are maintained in the companion [Vertical Canvas repository](https://github.com/westkerberos/obs-vertical-canvas/tree/feature/tiktok-native-output/data/licenses).

Do not present this bridge as a standalone TikTok runtime. Keep the upstream Aitum notices and the companion plugin's notices intact. Review the license status of the companion integration separately before distributing a combined installer or bundle.

## Windows package verification

The latest Windows x64 artifact is produced by workflow [#44162614334](https://github.com/westkerberos/obs-aitum-multistream/actions/runs/44162614334), commit `abf4f9848b221a8c560d31e09c319c1970f9d73a`, artifact `aitum-multistream-1.0.9-windows-x64-1a5d88e83`. The previously inspected standard ZIP was confirmed to contain the main license text at `data/obs-plugins/aitum-multistream/licenses/GPL-2.0.txt`. Re-inspect the latest artifact's file list before publishing a release.
