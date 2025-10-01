using Nuke.Common;
using Nuke.Common.IO;
using Nuke.Common.Tooling;
using Nuke.Cola;
using Nuke.Cola.BuildPlugins;
using Nuke.Unreal;
using Serilog;
using System;
using Nuke.Cola.FolderComposition;
using System.Security.Cryptography;
using Nuke.Unreal.Plugins;
using NuGet.ProjectModel;

public static class UseMcroGraph
{
    public static ITargetDefinition McroGraph(this ITargetDefinition target) => target
        .After<IMcroLicenseRegion>(_ => _.EnsureMcroLicense, _ => _.RenderMcroAttribution)
        .After<IUseYamlCpp>()
        .After<IUseRangeV3>()
    ;
}

[ImplicitBuildInterface]
public interface IUseMcro : INukeBuild
{
    Target GenerateMcroDocs => _ => _
        .Triggers<IMcroLicenseRegion>(_ => _.RenderMcroAttribution)
        .Executes(() =>
        {
            ToolResolver.GetPathTool("doxygen")(
                (this.ScriptFolder() / "Doxyfile").ToString(),
                workingDirectory: this.ScriptFolder()
            );
        });

    Target DistributeMcro => _ => _
        .DependsOn<IUseYamlCpp>()
        .DependsOn<IUseRangeV3>()
        .Executes(() =>
        {
            var (_, output) = UnrealPlugin.Get(this.ScriptFolder()).DistributeSource((UnrealBuild)this);
            Log.Information("Find distribution in {0}", output);
        });

    Target BuildMcro => _ => _
        .DependsOn<IUseYamlCpp>()
        .DependsOn<IUseRangeV3>()
        .Executes(() =>
        {
            var output = UnrealPlugin.Get(this.ScriptFolder())
                .BuildPlugin(
                    (UnrealBuild)this,
                    buildOptions: new() { Method = PluginBuildMethod.UBT }
                );
            Log.Information("Find build in {0}", output);
        });
}