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

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Xml.Serialization;
using EpicGames.Core;
using UnrealBuildTool;

namespace McroBuild;

public static partial class ModuleRuleExtensions
{
	public static AbsolutePath ModulePath(this ModuleRules self) => self.ModuleDirectory.AsPath();
	public static AbsolutePath PluginPath(this ModuleRules self) => self.PluginDirectory.AsPath();
	public static AbsolutePath ProjectPath(this ModuleRules self) => self.Target.ProjectFile!.Directory.AsPath();
	public static AbsolutePath PluginBinaries(this ModuleRules self) => self.PluginPath() / "Binaries";
	public static AbsolutePath PluginBinariesPlatform(this ModuleRules self, string insert = "")
		=> self.PluginBinaries() / insert / self.Target.Platform.ToString();
	
	public static AbsolutePath PluginModuleBinaries(this ModuleRules self, string insert = "")
		=> self.PluginBinaries() / insert / self.GetBaseModuleName();
	
	public static AbsolutePath PluginModuleBinariesPlatform(this ModuleRules self, string insert = "")
		=> self.PluginBinaries() / insert / self.GetBaseModuleName() / self.Target.Platform.ToString();
	
	public static bool IsReallyDebug(this ModuleRules self) =>
		self.Target is { Configuration: UnrealTargetConfiguration.Debug, bDebugBuildsActuallyUseDebugCRT: true };

	public static string GetLibraryConfig(this ModuleRules self, bool allowDebugLibraries = true)
		=> allowDebugLibraries && self.IsReallyDebug() ? "Debug" : "Release";
	
	public static string GetBaseModuleName(this ModuleRules self)
	{
		if (!self.GetType().Name.Contains("_")) return self.GetType().Name;
		var moduleNameComponents = self.GetType().Name
			.Split('_')
			.SkipLast(1);
		return string.Join('_', moduleNameComponents);
	}
}
