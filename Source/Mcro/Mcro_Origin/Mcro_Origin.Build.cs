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
using ModuleExtensions.Origin;

public class Mcro_Origin : ModuleRules
{
	public Mcro_Origin(ReadOnlyTargetRules Target) : base(Target)
	{
		this.IsSharedModule();
		
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
			
			"Ctre_Origin",
			"MagicEnum_Origin",
			"YamlCpp_Origin",
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
