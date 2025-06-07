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
public class RuntimeDependency
{
	[XmlText]
	public string Value = "";

	[XmlAttribute]
	public string Platform;

	[XmlAttribute]
	public string Config;
};

public class RuntimeDependencies
{
	[XmlElement]
	public RuntimeDependency[] RuntimeLibraryPath = Array.Empty<RuntimeDependency>();
	public RuntimeDependency[] Files = Array.Empty<RuntimeDependency>();
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

public static partial class ModuleRuleExtensions
{
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
	
	public static void DefineDllPath(this ModuleRules self, string pluginRelativePath)
	{
		self.PublicRuntimeLibraryPaths.Add(self.PluginPath() / pluginRelativePath);
		self.PublicDefinitions.Add($"{self.GetBaseModuleName().ToUpper()}_DLL_PATH=TEXT(\"{pluginRelativePath}\")");
	}

	public static void DefineDllList(this ModuleRules self, IEnumerable<string> dlls)
	{
		var dllsCache = dlls.ToArray();
		var dllList = string.Join(',', dllsCache.Select(d => $"TEXT(\"{d}\")"));
		self.PublicDefinitions.Add($"{self.GetBaseModuleName().ToUpper()}_DLL_FILES={dllList}");
		self.PublicDelayLoadDLLs.AddRange(dllsCache);
	}
	
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
