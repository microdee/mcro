/** @noop License Comment
 *  @file
 *  @copyright
 *  This Source Code is subject to the terms of the Mozilla Public License, v2.0.
 *  If a copy of the MPL was not distributed with this file You can obtain one at
 *  https://mozilla.org/MPL/2.0/
 *  
 *  @author David Mórász
 *  @date 2025
 */

/**
 * @file
 * The UBT target and module rule scripts are handled as one big C# project with all the freedom that implies allowed.
 * Any `*.Build.cs` file will be picked up but they not necessarily need to declare a module or a target, or multiple
 * C# sources can declare one target (via partial classes of course)
 * @file
 * Realizing this MCRO provides some very handy utilities to make writing these rules much more convenient, especially
 * for handling the intricacies of linking elaborate third-party libraries in Unreal.
 */

using System.Linq;
using UnrealBuildTool;

namespace McroBuild;

/// <summary>
/// Convenience utilities for module rules 
/// </summary>
public static partial class ModuleRuleExtensions
{
	/// <returns>Path to the module folder</returns>
	public static AbsolutePath ModulePath(this ModuleRules self) => self.ModuleDirectory.AsPath();
	
	/// <returns>Path to the plugin folder to which this module belongs</returns>
	public static AbsolutePath PluginPath(this ModuleRules self) => self.PluginDirectory.AsPath();
	
	/// <returns>Path to the project folder to which this module belongs</returns>
	public static AbsolutePath ProjectPath(this ModuleRules self) => self.Target.ProjectFile!.Directory.AsPath();
	
	/// <returns>A consistent place for plugin binaries</returns>
	public static AbsolutePath PluginBinaries(this ModuleRules self) => self.PluginPath() / "Binaries";
	
	/// <param name="self"></param>
	/// <param name="insert">A path segment between main plugin binaries folder and the platform specifier</param>
	/// <returns>A consistent place for plugin binaries regarding current platform</returns>
	public static AbsolutePath PluginBinariesPlatform(this ModuleRules self, string insert = "")
		=> self.PluginBinaries() / insert / self.Target.Platform.ToString();
	
	/// <param name="self"></param>
	/// <param name="insert">A path segment between main plugin binaries folder and the current module name</param>
	/// <returns>A consistent place for module binaries in owning plugin.</returns>
	public static AbsolutePath PluginModuleBinaries(this ModuleRules self, string insert = "")
		=> self.PluginBinaries() / insert / self.GetBaseModuleName();
	
	/// <param name="self"></param>
	/// <param name="insert">A path segment between main plugin binaries folder and the current module name</param>
	/// <returns>A consistent place for module binaries in owning plugin and regarding current platform.</returns>
	public static AbsolutePath PluginModuleBinariesPlatform(this ModuleRules self, string insert = "")
		=> self.PluginBinaries() / insert / self.GetBaseModuleName() / self.Target.Platform.ToString();
	
	/// <summary>
	/// Is the current build fully linked as debug build? This is usually only the case when building engine as Debug
	/// from source (optionally via a project)
	/// </summary>
	public static bool IsReallyDebug(this ModuleRules self) =>
		self.Target is { Configuration: UnrealTargetConfiguration.Debug, bDebugBuildsActuallyUseDebugCRT: true };

	/// <summary>
	/// Get the actual preferred linkage for a third-party library
	/// </summary>
	public static string GetLibraryConfig(this ModuleRules self, bool allowDebugLibraries = true)
		=> allowDebugLibraries && self.IsReallyDebug() ? "Debug" : "Release";
	
	/// <summary>
	/// Infer module name from its class name
	/// </summary>
	public static string GetBaseModuleName(this ModuleRules self)
	{
		if (!self.GetType().Name.Contains("_")) return self.GetType().Name;
		var moduleNameComponents = self.GetType().Name
			.Split('_')
			.SkipLast(1);
		return string.Join('_', moduleNameComponents);
	}
}
