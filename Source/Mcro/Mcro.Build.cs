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

public class Mcro : ModuleRules
{
	public Mcro(ReadOnlyTargetRules Target) : base(Target)
	{
		// C++23
		bUseUnity = false;
		CppStandard = CppStandardVersion.Latest;
		
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"RenderCore",
			"ApplicationCore",
			"Slate",
			"SlateCore",
			"Boost",
			
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
