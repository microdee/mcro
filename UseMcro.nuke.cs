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
}