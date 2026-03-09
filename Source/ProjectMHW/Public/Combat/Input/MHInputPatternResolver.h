#pragma once

//손승우 추가: 입력 패턴 해석기

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Combat/Input/MHCombatInputTypes.h"
#include "MHInputPatternResolver.generated.h"

class UDataAsset_LSInputPatternSet;

DECLARE_LOG_CATEGORY_EXTERN(LogMHInputPatternResolver, Log, All);

UCLASS(BlueprintType)
class PROJECTMHW_API UMHInputPatternResolver : public UObject
{
	GENERATED_BODY()

public:
	UMHInputPatternResolver();

	void ResetResolver();

	void NotifyButtonPressed(EMHCombatInputButton InButton, float InCurrentTimeSeconds);
	void NotifyButtonReleased(EMHCombatInputButton InButton);

	bool ResolveBestPattern(
		const UDataAsset_LSInputPatternSet* InPatternSet,
		float InCurrentTimeSeconds,
		const FGameplayTag& InWeaponTypeTag,
		const FGameplayTag& InSheathStateTag,
		const FGameplayTag& InCombatStateTag,
		bool bInIsStandingStill,
		bool bInHasMoveInput,
		FMHResolvedInputPattern& OutResolvedPattern) const;

protected:
	bool IsPatternSatisfied(
		const FMHInputPatternDefinition& InPatternDefinition,
		float InCurrentTimeSeconds,
		const FGameplayTag& InWeaponTypeTag,
		const FGameplayTag& InSheathStateTag,
		const FGameplayTag& InCombatStateTag,
		bool bInIsStandingStill,
		bool bInHasMoveInput) const;

	bool AreRequiredButtonsHeld(const FMHInputPatternDefinition& InPatternDefinition) const;
	bool IsSimultaneousWindowSatisfied(const FMHInputPatternDefinition& InPatternDefinition) const;
	bool IsHoldConditionSatisfied(const FMHInputPatternDefinition& InPatternDefinition, float InCurrentTimeSeconds) const;
	bool AreStateConditionsSatisfied(
		const FMHInputPatternDefinition& InPatternDefinition,
		const FGameplayTag& InWeaponTypeTag,
		const FGameplayTag& InSheathStateTag,
		const FGameplayTag& InCombatStateTag,
		bool bInIsStandingStill,
		bool bInHasMoveInput) const;

private:
	TMap<EMHCombatInputButton, float> PressedButtonTimeMap;
	TSet<EMHCombatInputButton> HeldButtons;
};
