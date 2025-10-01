# Usage {#Usage}


## Installation

You can clone/submodule a repository as-is based on your build preferences:

* **[MCRO Sources](https://github.com/microdee/mcro.git)** if your project is already set up to use [Nuke.Unreal](https://github.com/microdee/Nuke.Unreal) *(see Build from source)*
* **[MCRO Distribution](https://github.com/microdee/mcro-dist.git)** for a vanilla Unreal plugin which doesn't need extra tooling.

> [!WARNING]
> **Do not use 'Download ZIP'!** It won't resolve LFS files and submodules. Doing so will yield missing files and/or corrupted binaries. **Always clone with Git!**

## Build from source

MCRO needs some preparation for 3rd-party libraries which is orchestrated by Nuke.Unreal. [Follow these instructions](https://github.com/microdee/Nuke.Unreal?tab=readme-ov-file#install-via-remote-script) to set up your host project with it. MCRO will automatically do its preparations when the following targets are invoked: `prepare`, `generate`, `build-editor`, `build`, `cook`, `package`. Use `--skip prepare` if you don't want to execute those preparations every time you invoke those targets.

Nuke.Unreal relies on [Xmake](https://xmake.io) (specifically its [packages](https://xmake.microblock.cc) or formerly [Xrepo](https://xrepo.xmake.io) feature) to manage some 3rd-party libraries. It will attempt to automatically install it on your system if it's not in PATH. Xmake may attempt to use other build tools for preparing library files (like CMake and platform specific C++ toolchains).