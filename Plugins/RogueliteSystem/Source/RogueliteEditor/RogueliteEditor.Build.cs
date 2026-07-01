using UnrealBuildTool;

public class RogueliteEditor : ModuleRules
{
    public RogueliteEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "RogueliteCore",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UnrealEd",
                "EditorStyle",
                "InputCore",
                "PropertyEditor",
                "GameplayTags",
                "AssetRegistry",
                "ToolMenus",
                "WorkspaceMenuStructure",
            }
        );
    }
}