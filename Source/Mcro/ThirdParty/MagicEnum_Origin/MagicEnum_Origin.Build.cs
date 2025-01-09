
using UnrealBuildTool;

public class MagicEnum_Origin : ModuleRules
{
	public MagicEnum_Origin(ReadOnlyTargetRules target) : base(target)
	{
		Type = ModuleType.External;
		PublicIncludePaths.Add($"{ModuleDirectory}/Include");
	}
}