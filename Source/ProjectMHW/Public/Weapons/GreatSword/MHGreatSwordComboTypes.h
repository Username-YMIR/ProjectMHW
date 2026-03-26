#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MHGreatSwordComboTypes.generated.h"

// ===== Enums =====
UENUM(BlueprintType)
enum class EMHGreatSwordChargeFamily : uint8
{
    None        UMETA(DisplayName = "None"),
    Charge      UMETA(DisplayName = "Charge"),
    Strong      UMETA(DisplayName = "Strong"),
    TrueCharge  UMETA(DisplayName = "TrueCharge")
};

UENUM(BlueprintType)
enum class EMHGreatSwordTransitionPhase : uint8
{
    None            UMETA(DisplayName = "None"),
    Entry           UMETA(DisplayName = "Entry"),
    EarlyTransition UMETA(DisplayName = "EarlyTransition"),
    ChargeFollowUp  UMETA(DisplayName = "ChargeFollowUp"),
    Charging        UMETA(DisplayName = "Charging")
};

UENUM(BlueprintType)
enum class EMHGreatSwordDirectiveType : uint8
{
    None          UMETA(DisplayName = "None"),
    PlayMove      UMETA(DisplayName = "PlayMove"),
    BeginCharge   UMETA(DisplayName = "BeginCharge"),
    ReleaseCharge UMETA(DisplayName = "ReleaseCharge"),
    EnterGuard    UMETA(DisplayName = "EnterGuard"),
    ExitGuard     UMETA(DisplayName = "ExitGuard")
};

// ===== Runtime Types =====
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHGreatSwordActionDirective
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GreatSword")
    EMHGreatSwordDirectiveType Type = EMHGreatSwordDirectiveType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GreatSword")
    FGameplayTag MoveTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GreatSword")
    EMHGreatSwordChargeFamily ChargeFamilyHint = EMHGreatSwordChargeFamily::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GreatSword")
    bool bUsePostTackleChargeFamily = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GreatSword")
    bool bStartedFromSheathedForward = false;

    void Reset()
    {
        Type = EMHGreatSwordDirectiveType::None;
        MoveTag = FGameplayTag::EmptyTag;
        ChargeFamilyHint = EMHGreatSwordChargeFamily::None;
        bUsePostTackleChargeFamily = false;
        bStartedFromSheathedForward = false;
    }

    bool IsValid() const
    {
        if (Type == EMHGreatSwordDirectiveType::PlayMove || Type == EMHGreatSwordDirectiveType::EnterGuard)
        {
            return MoveTag.IsValid();
        }

        return Type != EMHGreatSwordDirectiveType::None;
    }
};

USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHGreatSwordTransitionContext
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword")
    FGameplayTag CurrentMoveTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword")
    FGameplayTag ChargeFollowUpSourceMoveTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword")
    EMHGreatSwordTransitionPhase Phase = EMHGreatSwordTransitionPhase::None;
};
