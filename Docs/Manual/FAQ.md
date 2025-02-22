# Bitter Recompense {#FAQ}

## C++ 23

> Isn't Unreal only compatible with `<insert C++ version here>`?

Unreal vanilla modules are written with the mindset that all should be compatible with a minimal version of C++ (in case of Unreal 5.5 this is C++17 despite the fact that it is compiled with C++20). Compatibility here means header-source syntax compatibility.

The compiled modules don't care which language version has been used as long as the results are ABI compatible. In fact they don't care about the language either (for example ISPC source files are mixed with C++ source files).

> Wouldn't I need to use Engine Sources for this?

**No**. When compiling your modules/plugins, in 99% of cases it is perfectly fine to use a newer C++ version than what the vanilla engines are using. That means you can use C++23+ with vanilla installed engine (in fact MCRO is developed with vanilla Unreal as well).

> How do I set my module rules to support MCRO?

As of time of writing, setting `CppStandard = CppStandardVersion.Latest;` is required. Setting `bUseUnity = false` is strongly recommended for avoiding surprise redefinitions, but it's not required. Setting `DefaultBuildSettings = BuildSettingsVersion.V5;` in your Target rules is also recommended but that's even more optional.

> Does my project need to use C++ 23 because another plugin uses MCRO internally?

If

* that plugin uses MCRO features in public headers
* AND that plugin is included in the project to be used from C++

then yes.

> All these templating must be hard on code-completion

I've been developing this library with Rider and their code-completion fairs pretty well with elaborate templates even for new C++ features or when multiple `decltype` expressions are involved. Concept constrained templates even get very readable list of failure causality when substitution fails right in the editor even before compiling.

## Usage

> Can I use MCRO in my close-sourced commercial project?

**Yes**, *free of charge for any scale.* Please list MCRO and the [open-source libraries MCRO uses](#Attribution) in your open-source attribution. MCRO itself is [licensed under MPL 2.0](#Legal).

> Can I use MCRO in my plugin published on FAB for a price?

**Yes**, *free of charge, no royalties or anything* . Please list MCRO and the [open-source libraries MCRO uses](#Attribution) when you submit your plugin to FAB. MCRO itself is [licensed under MPL 2.0](#Legal). Also it is recommended to compose the MCRO proto-plugin into yours with the help of [Nuke.Unreal](https://github.com/microdee/Nuke.Unreal).