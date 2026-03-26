#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/GreatSword/MHGreatSwordComboTypes.h"
#include "MHGreatSwordChargeStateComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHGreatSwordChargeStateComponent, Log, All);

UCLASS(ClassGroup = (Weapon), BlueprintType, meta = (BlueprintSpawnableComponent))
class PROJECTMHW_API UMHGreatSwordChargeStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMHGreatSwordChargeStateComponent();

// ===== Charge =====
public:
    void ResetChargeState();

    bool IsCharging() const { return CurrentChargeFamily != EMHGreatSwordChargeFamily::None; }
    EMHGreatSwordChargeFamily GetCurrentChargeFamily() const { return CurrentChargeFamily; }
    int32 GetCurrentChargeLevel() const { return CurrentChargeLevel; }
    bool IsReleaseReady() const { return bChargeReleaseReady; }

    void BeginCharge(EMHGreatSwordChargeFamily InFamily, bool bInStartedFromSheathedForward);
    void NotifyChargeLevelReached(int32 InChargeLevel);
    bool BuildReleaseDirective(FMHGreatSwordActionDirective& OutDirective);
    bool BuildAutoReleaseDirective(FMHGreatSwordActionDirective& OutDirective);

// ===== Tackle =====
public:
    void HandleTackleFromCurrentCharge();

    EMHGreatSwordChargeFamily GetPendingPostTackleChargeFamily() const { return PendingPostTackleChargeFamily; }
    EMHGreatSwordChargeFamily GetLastTackleSourceChargeFamily() const { return LastTackleSourceChargeFamily; }
    EMHGreatSwordChargeFamily ConsumePostTackleChargeFamily();
    void ClearPostTackleChargeState();

// ===== Debug =====
public:
    EMHGreatSwordChargeFamily GetLastReleasedChargeFamily() const { return LastReleasedChargeFamily; }
    int32 GetLastReleasedChargeLevel() const { return LastReleasedChargeLevel; }

    void SyncFromMirroredState(
        EMHGreatSwordChargeFamily InCurrentChargeFamily,
        int32 InCurrentChargeLevel,
        EMHGreatSwordChargeFamily InPendingPostTackleChargeFamily,
        EMHGreatSwordChargeFamily InLastTackleSourceChargeFamily,
        EMHGreatSwordChargeFamily InLastReleasedChargeFamily,
        int32 InLastReleasedChargeLevel,
        bool bInChargeReleaseReady,
        bool bInStartedFromSheathedForwardInput
    );

// ===== Internal =====
private:
    EMHGreatSwordChargeFamily ResolveNextChargeFamilyAfterTackle(EMHGreatSwordChargeFamily InCurrentFamily) const;
    FGameplayTag ResolveChargeReleaseMoveTag(EMHGreatSwordChargeFamily InFamily) const;

// ===== Runtime =====
private:
    UPROPERTY(Transient)
    EMHGreatSwordChargeFamily CurrentChargeFamily = EMHGreatSwordChargeFamily::None;

    UPROPERTY(Transient)
    EMHGreatSwordChargeFamily PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;

    UPROPERTY(Transient)
    EMHGreatSwordChargeFamily LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;

    UPROPERTY(Transient)
    EMHGreatSwordChargeFamily LastReleasedChargeFamily = EMHGreatSwordChargeFamily::None;

    UPROPERTY(Transient)
    int32 CurrentChargeLevel = 0;

    UPROPERTY(Transient)
    int32 LastReleasedChargeLevel = 0;

    UPROPERTY(Transient)
    bool bChargeReleaseReady = false;

    UPROPERTY(Transient)
    bool bStartedFromSheathedForwardInput = false;
};
