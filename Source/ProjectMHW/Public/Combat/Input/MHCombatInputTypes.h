#pragma once

//손승우 추가: 입력 패턴 기반 전투 시스템 공통 타입

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MHCombatInputTypes.generated.h"

UENUM(BlueprintType)
enum class EMHCombatInputButton : uint8
{
	None	UMETA(DisplayName = "None"),
	LMB		UMETA(DisplayName = "LMB"),
	RMB		UMETA(DisplayName = "RMB"),
	Mouse4	UMETA(DisplayName = "Mouse4"),
	Mouse5	UMETA(DisplayName = "Mouse5"),
	Space	UMETA(DisplayName = "Space"),
	Shift	UMETA(DisplayName = "Shift"),
	C		UMETA(DisplayName = "C"),
	E		UMETA(DisplayName = "E"),
	Q		UMETA(DisplayName = "Q"),
	R		UMETA(DisplayName = "R")
};

USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHInputPatternDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	FGameplayTag PatternTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	TArray<EMHCombatInputButton> RequiredButtons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	TArray<EMHCombatInputButton> OptionalButtons;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	bool bRequireSimultaneous = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern", meta = (ClampMin = "0.0"))
	float MaxSimultaneousGap = 0.12f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	bool bAllowHold = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern", meta = (ClampMin = "0.0"))
	float MinHoldTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	int32 Priority = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	FGameplayTag AllowedWeaponTypeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	FGameplayTag AllowedSheathStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	FGameplayTag AllowedCombatStateTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	bool bRequireStandingStill = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	bool bRequireMoveInput = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input Pattern")
	bool bAllowMoving = true;
};

USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHResolvedInputPattern
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Pattern")
	FGameplayTag PatternTag;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Pattern")
	FText DisplayName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Pattern")
	TArray<EMHCombatInputButton> MatchedButtons;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input Pattern")
	int32 Priority = MIN_int32;

	void Reset()
	{
		PatternTag = FGameplayTag::EmptyTag;
		DisplayName = FText::GetEmpty();
		MatchedButtons.Reset();
		Priority = MIN_int32;
	}

	bool IsValid() const
	{
		return PatternTag.IsValid();
	}
};
