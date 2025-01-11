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

namespace ModuleExtensions.Origin
{
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
	
	public static class ModuleRuleExtensions
	{
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

		public static void IsSharedModule(this ModuleRules self, bool defineLinkMacro = true)
		{
			var moduleBaseName = self.GetBaseModuleName();
			if (defineLinkMacro)
				self.PublicDefinitions.Add($"{moduleBaseName.ToUpper()}_API={self.GetType().Name.ToUpper()}_API");
		}

		public static void UseRuntimeDependencies(this ModuleRules self, bool allowDebugLibraries = true)
		{
			var manifestFile = $"{self.ModuleDirectory}/RuntimeDeps.xml";
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
				.Select(i => $"{self.PluginDirectory}/{i.Value}")
				.ToList();

			foreach (var dep in runtimeDeps) self.RuntimeDependencies.Add(dep);

			var runtimeLibPath = deps.RuntimeLibraryPath
				.Where(i => i.Platform?.Contains(self.Target.Platform.ToString()) ?? true)
				.Where(i => i.Config?.Contains(self.GetLibraryConfig(allowDebugLibraries)) ?? true)
				.Select(i => i.Value)
				.FirstOrDefault()
				?? $"Binaries/ThirdParty/{self.GetBaseModuleName()}/{self.Target.Platform}/{self.GetLibraryConfig(allowDebugLibraries)}";

			self.PublicRuntimeLibraryPaths.Add($"{self.PluginDirectory}/{runtimeLibPath}");
			self.PublicDefinitions.Add($"{self.GetBaseModuleName().ToUpper()}_DLL_PATH=TEXT(\"{runtimeLibPath}\")");

			var dllDeps = deps.Dlls
				.Where(i => i.Platform?.Contains(self.Target.Platform.ToString()) ?? true)
				.Where(i => i.Config?.Contains(self.GetLibraryConfig(allowDebugLibraries)) ?? true)
				.Select(i => i.Value)
				.ToList();
			
			self.PublicDelayLoadDLLs.AddRange(dllDeps);
			
			var dllList = string.Join(',', dllDeps.Select(d => $"TEXT(\"{d}\")"));
			self.PublicDefinitions.Add($"{self.GetBaseModuleName().ToUpper()}_DLL_FILES={dllList}");
		}
	}
}
