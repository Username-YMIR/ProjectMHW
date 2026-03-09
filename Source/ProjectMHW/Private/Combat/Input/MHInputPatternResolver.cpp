#include "Combat/Input/MHInputPatternResolver.h"

#include "Combat/Input/DataAsset_LSInputPatternSet.h"

//손승우 추가: 입력 패턴 해석기 구현

DEFINE_LOG_CATEGORY(LogMHInputPatternResolver);

UMHInputPatternResolver::UMHInputPatternResolver()
{
}

void UMHInputPatternResolver::ResetResolver()
{
	PressedButtonTimeMap.Reset();
	HeldButtons.Reset();
}

void UMHInputPatternResolver::NotifyButtonPressed(EMHCombatInputButton InButton, float InCurrentTimeSeconds)
{
	if (InButton == EMHCombatInputButton::None)
	{
		return;
	}

	HeldButtons.Add(InButton);
	PressedButtonTimeMap.FindOrAdd(InButton) = InCurrentTimeSeconds;
}

void UMHInputPatternResolver::NotifyButtonReleased(EMHCombatInputButton InButton)
{
	if (InButton == EMHCombatInputButton::None)
	{
		return;
	}

	HeldButtons.Remove(InButton);
	PressedButtonTimeMap.Remove(InButton);
}

bool UMHInputPatternResolver::ResolveBestPattern(
	const UDataAsset_LSInputPatternSet* InPatternSet,
	float InCurrentTimeSeconds,
	const FGameplayTag& InWeaponTypeTag,
	const FGameplayTag& InSheathStateTag,
	const FGameplayTag& InCombatStateTag,
	bool bInIsStandingStill,
	bool bInHasMoveInput,
	FMHResolvedInputPattern& OutResolvedPattern) const
{
	OutResolvedPattern.Reset();

	if (!IsValid(InPatternSet))
	{
		UE_LOG(LogMHInputPatternResolver, Warning, TEXT("ResolveBestPattern : PatternSet is invalid"));
		return false;
	}

	for (const FMHInputPatternDefinition& PatternDefinition : InPatternSet->GetPatternDefinitions())
	{
		if (!IsPatternSatisfied(
				PatternDefinition,
				InCurrentTimeSeconds,
				InWeaponTypeTag,
				InSheathStateTag,
				InCombatStateTag,
				bInIsStandingStill,
				bInHasMoveInput))
		{
			continue;
		}

		const bool bIsHigherPriority = PatternDefinition.Priority > OutResolvedPattern.Priority;
		const bool bIsSamePriorityButMoreSpecific =
			PatternDefinition.Priority == OutResolvedPattern.Priority &&
			PatternDefinition.RequiredButtons.Num() > OutResolvedPattern.MatchedButtons.Num();

		if (!bIsHigherPriority && !bIsSamePriorityButMoreSpecific)
		{
			continue;
		}

		OutResolvedPattern.PatternTag = PatternDefinition.PatternTag;
		OutResolvedPattern.DisplayName = PatternDefinition.DisplayName;
		OutResolvedPattern.MatchedButtons = PatternDefinition.RequiredButtons;
		OutResolvedPattern.Priority = PatternDefinition.Priority;
	}

	return OutResolvedPattern.IsValid();
}

bool UMHInputPatternResolver::IsPatternSatisfied(
	const FMHInputPatternDefinition& InPatternDefinition,
	float InCurrentTimeSeconds,
	const FGameplayTag& InWeaponTypeTag,
	const FGameplayTag& InSheathStateTag,
	const FGameplayTag& InCombatStateTag,
	bool bInIsStandingStill,
	bool bInHasMoveInput) const
{
	if (!InPatternDefinition.PatternTag.IsValid())
	{
		return false;
	}

	if (!AreRequiredButtonsHeld(InPatternDefinition))
	{
		return false;
	}

	if (!AreStateConditionsSatisfied(
			InPatternDefinition,
			InWeaponTypeTag,
			InSheathStateTag,
			InCombatStateTag,
			bInIsStandingStill,
			bInHasMoveInput))
	{
		return false;
	}

	if (InPatternDefinition.bRequireSimultaneous && !IsSimultaneousWindowSatisfied(InPatternDefinition))
	{
		return false;
	}

	if (InPatternDefinition.bAllowHold && !IsHoldConditionSatisfied(InPatternDefinition, InCurrentTimeSeconds))
	{
		return false;
	}

	return true;
}

bool UMHInputPatternResolver::AreRequiredButtonsHeld(const FMHInputPatternDefinition& InPatternDefinition) const
{
	if (InPatternDefinition.RequiredButtons.IsEmpty())
	{
		return false;
	}

	for (const EMHCombatInputButton RequiredButton : InPatternDefinition.RequiredButtons)
	{
		if (!HeldButtons.Contains(RequiredButton))
		{
			return false;
		}
	}

	return true;
}

bool UMHInputPatternResolver::IsSimultaneousWindowSatisfied(const FMHInputPatternDefinition& InPatternDefinition) const
{
	if (InPatternDefinition.RequiredButtons.Num() <= 1)
	{
		return true;
	}

	float MinPressedTime = TNumericLimits<float>::Max();
	float MaxPressedTime = TNumericLimits<float>::Lowest();

	for (const EMHCombatInputButton RequiredButton : InPatternDefinition.RequiredButtons)
	{
		const float* FoundPressedTime = PressedButtonTimeMap.Find(RequiredButton);
		if (FoundPressedTime == nullptr)
		{
			return false;
		}

		MinPressedTime = FMath::Min(MinPressedTime, *FoundPressedTime);
		MaxPressedTime = FMath::Max(MaxPressedTime, *FoundPressedTime);
	}

	return (MaxPressedTime - MinPressedTime) <= InPatternDefinition.MaxSimultaneousGap;
}

bool UMHInputPatternResolver::IsHoldConditionSatisfied(const FMHInputPatternDefinition& InPatternDefinition, float InCurrentTimeSeconds) const
{
	if (InPatternDefinition.MinHoldTime <= 0.0f)
	{
		return true;
	}

	for (const EMHCombatInputButton RequiredButton : InPatternDefinition.RequiredButtons)
	{
		const float* FoundPressedTime = PressedButtonTimeMap.Find(RequiredButton);
		if (FoundPressedTime == nullptr)
		{
			return false;
		}

		if ((InCurrentTimeSeconds - *FoundPressedTime) < InPatternDefinition.MinHoldTime)
		{
			return false;
		}
	}

	return true;
}

bool UMHInputPatternResolver::AreStateConditionsSatisfied(
	const FMHInputPatternDefinition& InPatternDefinition,
	const FGameplayTag& InWeaponTypeTag,
	const FGameplayTag& InSheathStateTag,
	const FGameplayTag& InCombatStateTag,
	bool bInIsStandingStill,
	bool bInHasMoveInput) const
{
	if (InPatternDefinition.AllowedWeaponTypeTag.IsValid() &&
		InPatternDefinition.AllowedWeaponTypeTag != InWeaponTypeTag)
	{
		return false;
	}

	if (InPatternDefinition.AllowedSheathStateTag.IsValid() &&
		InPatternDefinition.AllowedSheathStateTag != InSheathStateTag)
	{
		return false;
	}

	if (InPatternDefinition.AllowedCombatStateTag.IsValid() &&
		InPatternDefinition.AllowedCombatStateTag != InCombatStateTag)
	{
		return false;
	}

	if (InPatternDefinition.bRequireStandingStill && !bInIsStandingStill)
	{
		return false;
	}

	if (InPatternDefinition.bRequireMoveInput && !bInHasMoveInput)
	{
		return false;
	}

	if (!InPatternDefinition.bAllowMoving && bInHasMoveInput)
	{
		return false;
	}

	return true;
}
