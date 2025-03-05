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

public static class UseMcroGraph
{
    public static ITargetDefinition McroGraph(this ITargetDefinition target) => target
        .After<IMcroLicenseRegion>(_ => _.EnsureMcroLicense, _ => _.RenderMcroAttribution)
        .After<IUseYamlCpp>()
        .After<IUseRangeV3>()
    ;
}

public interface IUseMcro : INukeBuild
{
    void UseMcroAt(AbsolutePath pluginFolder, string suffix)
    {
        this.ImportFolders(suffix,
            (this.ScriptFolder() / "Source", pluginFolder, new ExportManifest
            {
                Use = { new() { Directory = "**"} }
            }),
            (this.ScriptFolder() / "Content", pluginFolder, new ExportManifest
            {
                Copy = { new() { Directory = "Slate" }}
            })
        );

        (pluginFolder / "Source" / "Mcro" / "Ignore.LicenseRegion.txt").WriteAllText(
            "This file is here so the Nuke.Cola LicenseRegion feature would not modify licenses in this folder"
        );
    }
}

[ImplicitBuildInterface]
public interface IImplicitMcroTargets : INukeBuild
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
}