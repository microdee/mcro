
using UnrealBuildTool;

public class MagicEnum : ModuleRules
{
	public MagicEnum(ReadOnlyTargetRules target) : base(target)
	{
		Type = ModuleType.External;
		PublicIncludePaths.Add($"{ModuleDirectory}/Include");
	}
}