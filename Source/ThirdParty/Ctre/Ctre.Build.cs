
using UnrealBuildTool;

public class Ctre : ModuleRules
{
	public Ctre(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;
		PublicIncludePaths.Add($"{ModuleDirectory}/Include");
	}
}