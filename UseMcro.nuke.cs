using Nuke.Common;
using Nuke.Common.IO;
using Nuke.Common.Tooling;
using Nuke.Cola;
using Nuke.Cola.BuildPlugins;
using Nuke.Unreal;
using Serilog;
using System;
using Nuke.Cola.FolderComposition;

public static class UseMcroGraph
{
    public static ITargetDefinition McroGraph(this ITargetDefinition target) => target
        .After<IUseYamlCpp>();
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
    }
}