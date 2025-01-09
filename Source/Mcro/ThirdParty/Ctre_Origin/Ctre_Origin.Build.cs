
using UnrealBuildTool;

public class Ctre_Origin : ModuleRules
{
	public Ctre_Origin(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		PublicIncludePaths.Add($"{ModuleDirectory}/Include");
	}
}