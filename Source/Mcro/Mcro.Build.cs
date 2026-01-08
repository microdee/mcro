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
/// Low level C++ (mostly) templating utilities for a variety of common problems occuring during Unreal Development
/// </summary>
public class Mcro : ModuleRules
{
	public Mcro(ReadOnlyTargetRules Target) : base(Target)
	{
		// C++23
		bUseUnity = false;
		CppStandard = CppStandardVersion.Latest;

#if UE_5_6_OR_LATER
		const string boostVersion = "1.85.0";
#elif UE_5_4_OR_LATER
		const string boostVersion = "1_82_0";
#else
		const string boostVersion = "1_80_0";
#endif
		PublicSystemIncludePaths.AddRange(new []
		{
#if UE_5_6_OR_LATER
			$"{Target.UEThirdPartySourceDirectory}/Boost/Deploy/boost-{boostVersion}/include"
#else
			$"{Target.UEThirdPartySourceDirectory}/Boost/boost-{boostVersion}/include"
#endif
		});
		
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"RenderCore",
			"ApplicationCore",
			"Projects",
			"Slate",
			"SlateCore",
			
			"Ctre",
			"MagicEnum",
			"YamlCpp",
			"RangeV3"
		});
		
		PrivateDependencyModuleNames.AddRange(new[]
		{
			"CoreUObject",
			"Engine",
		});

		if (Target.Type == TargetType.Editor)
		{
			PrivateDependencyModuleNames.AddRange(new[]
			{
				"UnrealEd",
				"MainFrame"
			});
		}
	}
}
