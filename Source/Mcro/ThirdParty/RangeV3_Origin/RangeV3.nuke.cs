// Fill in Copyright info...

using Nuke.Common;
using Nuke.Common.IO;
using Nuke.Common.Tooling;
using Nuke.Cola;
using Nuke.Cola.Tooling;
using Nuke.Cola.BuildPlugins;
using Nuke.Unreal;
using Nuke.Unreal.BoilerplateGenerators.XRepo;
using Serilog;
using System;

[ImplicitBuildInterface]
[ParameterPrefix("range-v3")]
public interface IUseRangeV3 : INukeBuild
{
    AbsolutePath RangeV3InputDir => this.ScriptFolder() / "range-v3";

    string[] RangeV3Configs => [ "Release", "Debug" ];

    Target PrepareRangeV3 => _ => _
        .DependentFor<UnrealBuild>(b => b.Prepare)
        .Executes(() =>
        {
            ((UnrealBuild) this).InstallXRepoLibrary(
                "range-v3", "", this.ScriptFolder(), "Origin"
            );
        });
}