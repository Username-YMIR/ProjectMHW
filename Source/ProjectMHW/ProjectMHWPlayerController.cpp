// Copyright Epic Games, Inc. All Rights Reserved.


#include "ProjectMHWPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "MHGameplayTags.h"
#include "Blueprint/UserWidget.h"
#include "ProjectMHW.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AProjectMHWPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (SVirtualJoystick::ShouldDisplayTouchInterface() && IsLocalPlayerController())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogProjectMHW, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AProjectMHWPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!SVirtualJoystick::ShouldDisplayTouchInterface())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AProjectMHWPlayerController::DebugPlayerDamage(float PhysicalDamage, const FString& AttackTagName)
{
	AMHPlayerCharacter* PlayerCharacter = Cast<AMHPlayerCharacter>(GetPawn());
	if (!IsValid(PlayerCharacter))
	{
		UE_LOG(LogProjectMHW, Warning, TEXT("DebugPlayerDamage failed: player pawn is not AMHPlayerCharacter."));
		return;
	}

	FGameplayTag AttackTag = AttackTagName.IsEmpty()
		? MHGameplayTags::Attack_Debug_Counterable
		: FGameplayTag::RequestGameplayTag(FName(*AttackTagName), false);

	if (!AttackTag.IsValid())
	{
		AttackTag = MHGameplayTags::Attack_Debug_Counterable;
	}

	if (!AttackTag.IsValid())
	{
		UE_LOG(LogProjectMHW, Warning, TEXT("DebugPlayerDamage failed: invalid attack tag."));
		return;
	}

	PlayerCharacter->ApplyDebugDamageFromSource(
		PlayerCharacter,
		FMath::Max(0.0f, PhysicalDamage),
		AttackTag
	);

	UE_LOG(
		LogProjectMHW,
		Log,
		TEXT("DebugPlayerDamage executed. Damage=%.2f AttackTag=%s Target=%s"),
		PhysicalDamage,
		*AttackTag.ToString(),
		*GetNameSafe(PlayerCharacter)
	);
}
