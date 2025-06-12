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

/// <summary>
/// Model for one runtime dependency
/// </summary>
public class RuntimeDependency
{
	[XmlText]
	public string Value = "";

	[XmlAttribute]
	public string Platform;

	[XmlAttribute]
	public string Config;
};

/// <summary>
/// Model for reading a collection of runtime dependencies gathered by an external tool into an XML document 
/// </summary>
public class RuntimeDependencies
{
	[XmlElement]
	public RuntimeDependency[] RuntimeLibraryPath = Array.Empty<RuntimeDependency>();
	
	[XmlElement]
	public RuntimeDependency[] Files = Array.Empty<RuntimeDependency>();
	
	[XmlElement]
	public RuntimeDependency[] Dlls = Array.Empty<RuntimeDependency>();

	public void Serialize(TextWriter writer)
	{
		var serializer = new XmlSerializer(GetType());
		serializer.Serialize(writer, this);
	}

	public void Serialize(string file)
	{
		using TextWriter writer = new StreamWriter(file);
		Serialize(writer);
	}
	
	public static RuntimeDependencies Deserialize(FileStream stream)
	{
		var serializer = new XmlSerializer(typeof(RuntimeDependencies));
		var result = serializer.Deserialize(stream);
		return result as RuntimeDependencies;
	}

	public static RuntimeDependencies Deserialize(string file)
	{
		if (file == null || !File.Exists(file)) return null;
		using var stream = new FileStream(file, FileMode.Open);
		return Deserialize(stream);
	}
}

/// <summary>
/// Utilities for managing runtime file dependencies (usually DLL's) of third-party libraries in a more consistent and
/// automatic way than it is expected by vanilla templates.
/// </summary>
public static partial class ModuleRuleExtensions
{
	/// <summary>
	/// <para>
	/// Provide a folder from which runtime dependencies (by default dynamic libraries) are collected, copied to
	/// plugin-module binaries folder while keeping their relative folder structure and add them of course as
	/// `RuntimeDependencies` or if they're really dynamic libraries then add them to `PublicDelayLoadDLLs`.
	/// </para>
	/// <para>
	/// The list of DLL's and their base path is propagated to source code via preprocessor definitions
	/// `*_DLL_PATH` and `*_DLL_FILES`, where `*`˛is the capitalized module name. This goes in tandem with utilities
	/// inside `Mcro/Dll.h` especially `TModuleBoundDlls` which can sort out all your dynamic library loading chores
	/// on module startup and shutdown with a one-liner.
	/// </para>
	/// </summary>
	/// <example>
	/// TModuleBoundDlls&lt;FMyAwesomeModule&gt; GMyLibraryDlls {MYLIBRARY_DLL_PATH, MYLIBRARY_DLL_FILES};
	/// </example>
	/// <param name="self"></param>
	/// <param name="libraryFolder">Path containing the runtime dependencies</param>
	/// <param name="filePattern">Consider files with given glob pattern. Platform specific DLL files by default</param>
	/// <param name="thirdParty">Is this module importing a third party library. True by default</param>
	/// <param name="destinationPostfix">Extra levels in the plugin-module binaries folder. Empty by default</param>
	public static void PrepareRuntimeDependencies(
		this ModuleRules self,
		AbsolutePath libraryFolder,
		string filePattern = null,
		bool thirdParty = true,
		string destinationPostfix = ""
	) {
		var dllExtension = self.Target.Platform == UnrealTargetPlatform.Win64 ? "dll" : "so";
		filePattern ??= "*." + dllExtension;
		var binaries = thirdParty ? self.PluginBinaries() / "ThirdParty" : self.PluginBinaries();
		var dstDir = self.PluginModuleBinariesPlatform(thirdParty ? "ThirdParty" : "") / destinationPostfix;
		var files = libraryFolder.Copy(dstDir, filePattern);

		foreach (var dep in files) self.RuntimeDependencies.Add(dep);
		self.DefineDllPath(dstDir.RelativeToBase(self.PluginPath()));
		self.DefineDllList(
			files
					.Where(f => f.HasExtension("." + dllExtension))
					.Select(f => f.Name)
		);
	}
	
	/// <summary>
	/// Propagate a search path for module-specific delay loaded DLL files to C++ source via preprocessor.
	/// </summary>
	/// <param name="self"></param>
	/// <param name="pluginRelativePath">A relative path with the plugin as its base, which will be determined in runtime.</param>
	public static void DefineDllPath(this ModuleRules self, string pluginRelativePath)
	{
		self.PublicRuntimeLibraryPaths.Add(self.PluginPath() / pluginRelativePath);
		self.PublicDefinitions.Add($"{self.GetBaseModuleName().ToUpper()}_DLL_PATH=TEXT(\"{pluginRelativePath}\")");
	}

	/// <summary>
	/// Propagate delay loaded module-specific dynamic library files to C++ source via preprocessor.
	/// </summary>
	/// <param name="self"></param>
	/// <param name="dlls">List of DLL file names to load at some point by C++</param>
	public static void DefineDllList(this ModuleRules self, IEnumerable<string> dlls)
	{
		var dllsCache = dlls.ToArray();
		var dllList = string.Join(',', dllsCache.Select(d => $"TEXT(\"{d}\")"));
		self.PublicDefinitions.Add($"{self.GetBaseModuleName().ToUpper()}_DLL_FILES={dllList}");
		self.PublicDelayLoadDLLs.AddRange(dllsCache);
	}
	
	/// <summary>
	/// <para>
	/// Read a list of runtime dependencies from `RuntimeDeps.xml` located directly in the module folder. An external
	/// tool can generate this manifest for elaborate libraries and manage their runtime files themselves. The files
	/// listed in the manifest should be already copied to the place where they will be shipped for runtime as well
	/// (usually that's the plugin-module binaries folder)
	/// </para>
	/// <para>
	/// The list of DLL's and their base path is propagated to source code via preprocessor definitions
	/// `*_DLL_PATH` and `*_DLL_FILES`, where `*`˛is the capitalized module name. This goes in tandem with utilities
	/// inside `Mcro/Dll.h` especially `TModuleBoundDlls` which can sort out all your dynamic library loading chores
	/// on module startup and shutdown with a one-liner.
	/// </para>
	/// </summary>
	/// <example>
	/// TModuleBoundDlls&lt;FMyAwesomeModule&gt; GMyLibraryDlls {MYLIBRARY_DLL_PATH, MYLIBRARY_DLL_FILES};
	/// </example>
	/// <param name="self"></param>
	/// <param name="allowDebugLibraries"></param>
	public static void UseRuntimeDependencies(this ModuleRules self, bool allowDebugLibraries = true)
	{
		var manifestFile = self.ModulePath() / "RuntimeDeps.xml";
		var deps = RuntimeDependencies.Deserialize(manifestFile);
		if (deps == null)
		{
			Log.TraceInformationOnce(
				"{0}: Ignoring RuntimeDeps.xml because {1} doesn't exist.",
				self.GetType().Name,
				manifestFile
			);
			return;
		}

		var runtimeDeps = deps.Files
			.Where(i => i.Platform?.Contains(self.Target.Platform.ToString()) ?? true)
			.Where(i => i.Config?.Contains(self.GetLibraryConfig(allowDebugLibraries)) ?? true)
			.Select(i => self.PluginPath() / i.Value);

		foreach (var dep in runtimeDeps) self.RuntimeDependencies.Add(dep);

		var runtimeLibPath = deps.RuntimeLibraryPath
			.Where(i => i.Platform?.Contains(self.Target.Platform.ToString()) ?? true)
			.Where(i => i.Config?.Contains(self.GetLibraryConfig(allowDebugLibraries)) ?? true)
			.Select(i => i.Value)
			.FirstOrDefault()
			??
			$"Binaries/ThirdParty/{self.GetBaseModuleName()}/{self.Target.Platform}/{self.GetLibraryConfig(allowDebugLibraries)}";

		self.DefineDllPath(runtimeLibPath);

		var dllDeps = deps.Dlls
			.Where(i => i.Platform?.Contains(self.Target.Platform.ToString()) ?? true)
			.Where(i => i.Config?.Contains(self.GetLibraryConfig(allowDebugLibraries)) ?? true)
			.Select(i => i.Value);

		self.DefineDllList(dllDeps);
	}
}
