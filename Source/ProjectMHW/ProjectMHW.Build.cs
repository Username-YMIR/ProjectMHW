// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectMHW : ModuleRules
{
	public ProjectMHW(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities", 
			"GameplayTags",
			"GameplayTasks",
			"Niagara",
			"AIModule",
			"NavigationSystem"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{

		});

		PublicIncludePaths.AddRange(new string[] {
			"ProjectMHW",
			"ProjectMHW/Variant_Platforming",
			"ProjectMHW/Variant_Platforming/Animation",
			"ProjectMHW/Variant_Combat",
			"ProjectMHW/Variant_Combat/AI",
			"ProjectMHW/Variant_Combat/Animation",
			"ProjectMHW/Variant_Combat/Gameplay",
			"ProjectMHW/Variant_Combat/Interfaces",
			"ProjectMHW/Variant_Combat/UI",
			"ProjectMHW/Variant_SideScrolling",
			"ProjectMHW/Variant_SideScrolling/AI",
			"ProjectMHW/Variant_SideScrolling/Gameplay",
			"ProjectMHW/Variant_SideScrolling/Interfaces",
			"ProjectMHW/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
