# UnLua Upstream

This directory is vendored from Tencent UnLua:

- Repository: https://github.com/Tencent/UnLua
- Branch: `develop`
- Commit: `4a534502bbf2c5e51b456d9142acfa631e53350c`
- Manifest version: `2.3.6`
- Vendored: 2026-07-23

Only the upstream `Plugins/UnLua` directory is included. The `develop` pin is
intentional: unlike the current `master` and `release/2.4` snapshots, this commit
contains the UE 5.6 build-rule, type-trait, and net8 UHT compatibility changes.

Update the plugin as one unit and re-run the project Editor, Development, and
Shipping builds after changing the pin.
