using UnrealBuildTool;
#pragma warning disable CA1050

public class EJson : ModuleRules
{
    public EJson(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
            }
        );
    }
}
