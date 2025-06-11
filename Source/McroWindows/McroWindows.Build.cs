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

using UnrealBuildTool;
using McroBuild;

/// <summary>
/// A module containing Windows platform specific utilities like interop between Unreal and WinRT/COM
/// </summary>
public class McroWindows : ModuleRules
{
	public McroWindows(ReadOnlyTargetRules target) : base(target)
	{
		if (target.Platform != UnrealTargetPlatform.Win64)
		{
			Type = ModuleType.External;
			return;
		}
		
		// C++23
		bUseUnity = false;
		CppStandard = CppStandardVersion.Latest;
		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"RHI",
			"D3D11RHI",
			"D3D12RHI",
			
			"Mcro",
		});
		
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CoreUObject",
			"Engine",
		});
		
		PublicSystemLibraries.AddRange(new[]
		{
			"WindowsApp.lib",
			"shlwapi.lib",
			"runtimeobject.lib",
			"comsuppw.lib",
			"dxgi.lib",
			"d3d11.lib",
			"d3d12.lib"
		});
		
		AddEngineThirdPartyPrivateStaticDependencies(target, "DX12");

		PublicIncludePaths.AddRange(new[]
		{
			$"{target.WindowsPlatform.WindowsSdkDir}/Include/{target.WindowsPlatform.WindowsSdkVersion}/cppwinrt"
		});

		if (target.Configuration <= UnrealTargetConfiguration.DebugGame)
		{
			PublicDefinitions.AddRange(new[]
			{
				"WINRT_NATVIS=1"
			});
		}

		if (target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new[]
			{
				"UnrealEd"
			});
		}
	}
}
