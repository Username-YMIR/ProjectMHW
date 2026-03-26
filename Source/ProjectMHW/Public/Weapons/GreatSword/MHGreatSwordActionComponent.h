#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Type/MHWeaponAnimStructType.h"
#include "Weapons/GreatSword/MHGreatSwordComboTypes.h"
#include "MHGreatSwordActionComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHGreatSwordActionComponent, Log, All);

class UAnimMontage;
class UDataTable;
class UMHGreatSwordChargeStateComponent;
class UMHGreatSwordComboGraph;
struct FMHAttackMetaRow;

UENUM(BlueprintType)
enum class EMHGreatSwordActionState : uint8
{
    None        UMETA(DisplayName = "None"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Charging    UMETA(DisplayName = "Charging"),
    Acting      UMETA(DisplayName = "Acting"),
    Guarding    UMETA(DisplayName = "Guarding")
};

UENUM(BlueprintType)
enum class EMHGreatSwordBufferedInputType : uint8
{
    None            UMETA(DisplayName = "None"),
    Primary         UMETA(DisplayName = "Primary"),
    Secondary       UMETA(DisplayName = "Secondary"),
    Simultaneous    UMETA(DisplayName = "Simultaneous")
};

struct FMHGreatSwordRuntimeSnapshot
{
    EMHGreatSwordActionState ActionState = EMHGreatSwordActionState::Neutral;
    EMHGreatSwordChargeFamily ChargeFamily = EMHGreatSwordChargeFamily::None;
    FGameplayTag PendingMoveTag;
    FGameplayTag LastCommittedMoveTag;
    FGameplayTag ActiveUtilityMoveTag;
    EMHGreatSwordChargeFamily PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;
    EMHGreatSwordChargeFamily LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;
    int32 CurrentChargeLevel = 0;
    int32 LastReleasedChargeLevel = 0;
    EMHGreatSwordChargeFamily LastReleasedChargeFamily = EMHGreatSwordChargeFamily::None;
    int32 ChargeSessionId = 0;
    bool bChargeReleaseReady = false;
    bool bChargeAutoReleaseRequested = false;
    bool bChargeStartedFromSheathedForwardInput = false;
    bool bGuardHeld = false;
    bool bAttackRollWindowOpen = false;
    bool bEarlyTransitionWindowOpen = false;
    bool bChargeFollowUpWindowOpen = false;
    FGameplayTag ChargeFollowUpSourceMoveTag;
    bool bHasBufferedInput = false;
    EMHGreatSwordBufferedInputType BufferedInputType = EMHGreatSwordBufferedInputType::None;
    bool bBufferedForwardInput = false;
};

UCLASS(ClassGroup=(Weapon), Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class PROJECTMHW_API UMHGreatSwordActionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMHGreatSwordActionComponent();

    // ===== 설정 =====
    void SetComboGraph(UMHGreatSwordComboGraph* InComboGraph);
    void SetChargeStateComponent(UMHGreatSwordChargeStateComponent* InChargeStateComponent);

    // ===== 입력 처리 =====
    bool HandleResolvedInputPattern(const FGameplayTag& InPatternTag);
    bool HandleInputPatternWithBuffering(const FGameplayTag& InPatternTag);
    bool HandlePrimaryPressed(bool bInForwardInput, bool bInSheathed);
    bool HandlePrimaryReleased();
    bool HandleSecondaryPressed();
    bool HandleWeaponSpecialPressed(bool bInSheathed);
    bool HandleWeaponSpecialReleased();
    bool HandleSimultaneousPressed();
    bool HandleDodgePressed(bool bInSheathed, EMHDirectionalVariant InDirectionalVariant);

    // ===== 런타임 조회 =====
    FGameplayTag ConsumePendingMoveTag();
    const FGameplayTag& GetPendingMoveTag() const { return PendingMoveTag; }
    bool HasPendingMove() const { return PendingMoveTag.IsValid(); }
    bool IsCharging() const { return ActionState == EMHGreatSwordActionState::Charging; }
    bool IsGuarding() const { return ActionState == EMHGreatSwordActionState::Guarding; }
    bool IsAttackRollWindowOpen() const { return bAttackRollWindowOpen; }
    bool IsAnyTransitionWindowOpen() const;
    EMHGreatSwordActionState GetActionState() const { return ActionState; }
    EMHGreatSwordChargeFamily GetChargeFamily() const { return ChargeFamily; }
    int32 GetChargeLevel() const { return CurrentChargeLevel; }
    float ResolveChargeDamageScaleForMove(const FGameplayTag& InMoveTag) const;
    UAnimMontage* ResolveMontageForMove(const FGameplayTag& InMoveTag) const;
    bool FindAttackMetaRow(const FGameplayTag& InMoveTag, FMHAttackMetaRow& OutAttackMetaRow) const;
    FGameplayTag ResolveDodgeMoveTag(bool bInSheathed, EMHDirectionalVariant InDirectionalVariant) const;
    bool IsUtilityMoveTag(const FGameplayTag& InMoveTag) const;

    // ===== 스냅샷 =====
    void CaptureRuntimeSnapshot(FMHGreatSwordRuntimeSnapshot& OutSnapshot) const;
    void RestoreRuntimeSnapshot(const FMHGreatSwordRuntimeSnapshot& InSnapshot);

    // ===== 런타임 알림 =====
    void CommitExecutedMove(const FGameplayTag& InMoveTag);
    void NotifyUtilityMoveStarted(const FGameplayTag& InMoveTag);
    void NotifyUtilityMoveEnded(const FGameplayTag& InMoveTag, bool bInterrupted);
    void NotifyActionFinished();
    void NotifyChargeLevelReached(int32 InChargeLevel);
    bool NotifyChargeAutoReleaseRequested();
    void NotifyBeginAttackRollWindow();
    void NotifyEndAttackRollWindow();
    void NotifyBeginEarlyTransitionWindow();
    void NotifyEndEarlyTransitionWindow();
    void NotifyBeginChargeFollowUpWindow(const FGameplayTag& InSourceMoveTag);
    void NotifyEndChargeFollowUpWindow();

protected:
    // ===== 차지 헬퍼 =====
    FGameplayTag ResolveChargeChargingMoveTag(EMHGreatSwordChargeFamily InFamily) const;
    FGameplayTag ResolveChargeReleaseMoveTag(EMHGreatSwordChargeFamily InFamily) const;
    EMHGreatSwordChargeFamily ResolveNextChargeFamilyAfterTackleFromCharge(EMHGreatSwordChargeFamily InCurrentFamily) const;
    void ResetChargeContext();
    void ClearPostTackleChargeContext();
    void SyncChargeStateComponentFromLegacyState();

    // ===== 입력 버퍼 =====
    void ClearBufferedInput();
    void BufferInput(EMHGreatSwordBufferedInputType InInputType, bool bInForwardInput);
    bool TryConsumeBufferedTransitionInput();

    // ===== 그래프 전이 =====
    bool TryResolveGraphDrivenDirective(const FGameplayTag& InPatternTag, FMHGreatSwordActionDirective& OutDirective) const;
    bool ExecuteResolvedDirective(const FMHGreatSwordActionDirective& InDirective);
    EMHGreatSwordTransitionPhase ResolveTransitionPhase() const;
    void QueuePendingMove(const FGameplayTag& InMoveTag, EMHGreatSwordActionState InNextState);
    void BeginCharging(EMHGreatSwordChargeFamily InChargeFamily, bool bInStartedFromSheathedForwardInput);
    bool HandleChargingSecondaryInput();
    bool FinalizeCurrentChargeRelease();

    // ===== 레거시 호환 =====
    bool TryConsumeLegacyBufferedTransitionInput(EMHGreatSwordBufferedInputType InBufferedType, bool bInForwardInput);
    bool ResolvePrimaryDuringEarlyTransition(bool bInForwardInput);
    bool ResolveSecondaryDuringEarlyTransition();
    bool ResolveSimultaneousDuringEarlyTransition();

#pragma region References
    UPROPERTY(Transient)
    TObjectPtr<UMHGreatSwordComboGraph> ComboGraph;

    UPROPERTY(Transient)
    TObjectPtr<UMHGreatSwordChargeStateComponent> ChargeStateComponent;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Data")
    TObjectPtr<UDataTable> AttackMetaTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Animation")
    TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>> MoveMontageMap;
#pragma endregion

#pragma region ChargeTuning
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Charge")
    FVector ChargeSlashLevelDamageScale = FVector(1.0f, 1.2f, 1.45f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Charge")
    FVector StrongChargeSlashLevelDamageScale = FVector(1.0f, 1.25f, 1.55f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Charge")
    FVector TrueChargeSlashLevelDamageScale = FVector(1.0f, 1.3f, 1.65f);
#pragma endregion

#pragma region RuntimeState
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    EMHGreatSwordActionState ActionState = EMHGreatSwordActionState::Neutral;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    EMHGreatSwordChargeFamily ChargeFamily = EMHGreatSwordChargeFamily::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    FGameplayTag PendingMoveTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    FGameplayTag LastCommittedMoveTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    FGameplayTag ActiveUtilityMoveTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    EMHGreatSwordChargeFamily PendingPostTackleChargeFamily = EMHGreatSwordChargeFamily::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    EMHGreatSwordChargeFamily LastTackleSourceChargeFamily = EMHGreatSwordChargeFamily::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    int32 CurrentChargeLevel = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    int32 LastReleasedChargeLevel = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    EMHGreatSwordChargeFamily LastReleasedChargeFamily = EMHGreatSwordChargeFamily::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    int32 ChargeSessionId = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bChargeReleaseReady = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bChargeAutoReleaseRequested = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bChargeStartedFromSheathedForwardInput = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bGuardHeld = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bAttackRollWindowOpen = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bEarlyTransitionWindowOpen = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bChargeFollowUpWindowOpen = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    FGameplayTag ChargeFollowUpSourceMoveTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bHasBufferedInput = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    EMHGreatSwordBufferedInputType BufferedInputType = EMHGreatSwordBufferedInputType::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bBufferedForwardInput = false;
#pragma endregion
};