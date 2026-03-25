#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Type/MHWeaponAnimStructType.h"
#include "MHGreatSwordActionComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHGreatSwordActionComponent, Log, All);

class UAnimMontage;
class UDataTable;
struct FMHAttackMetaRow;

UENUM(BlueprintType)
enum class EMHGreatSwordChargeFamily : uint8
{
    None        UMETA(DisplayName = "None"),
    Charge      UMETA(DisplayName = "Charge"),
    Strong      UMETA(DisplayName = "Strong"),
    TrueCharge  UMETA(DisplayName = "TrueCharge")
};

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
    WeaponSpecial   UMETA(DisplayName = "WeaponSpecial"),
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

    // 좌클릭 입력 시작 시 현재 문맥에 맞는 대검 기술을 결정한다.
    bool HandlePrimaryPressed(bool bInForwardInput, bool bInSheathed);

    // 좌클릭 입력 종료 시 현재 차징을 실제 공격으로 확정한다.
    bool HandlePrimaryReleased();

    // 우클릭 입력 시 베어넘기기 또는 태클 파생을 결정한다.
    bool HandleSecondaryPressed();

    // Mouse4 입력 시 가드 또는 옆으로 치기를 결정한다.
    bool HandleWeaponSpecialPressed(bool bInSheathed);

    // Mouse4 입력 해제 시 가드 종료를 요청한다.
    bool HandleWeaponSpecialReleased();

    // Mouse5 입력 시 베어올리기를 결정한다.
    bool HandleSimultaneousPressed();

    // 현재 대기 중인 기술 태그를 가져오고 비운다.
    FGameplayTag ConsumePendingMoveTag();

    // 현재 대기 중인 기술 태그를 조회한다.
    const FGameplayTag& GetPendingMoveTag() const { return PendingMoveTag; }

    // 현재 대기 중인 기술이 있는지 확인한다.
    bool HasPendingMove() const { return PendingMoveTag.IsValid(); }

    // 현재 차징 중인지 확인한다.
    bool IsCharging() const { return ActionState == EMHGreatSwordActionState::Charging; }

    // 현재 가드 중인지 확인한다.
    bool IsGuarding() const { return ActionState == EMHGreatSwordActionState::Guarding; }

    // 현재 공격 후 4방향 구르기 윈도우가 열려 있는지 확인한다.
    bool IsAttackRollWindowOpen() const { return bAttackRollWindowOpen; }

    // 조기 전환 또는 차지 후속 전이 윈도우가 열려 있는지 확인한다.
    bool IsAnyTransitionWindowOpen() const;

    // 현재 대검 런타임 상태를 백업한다.
    void CaptureRuntimeSnapshot(FMHGreatSwordRuntimeSnapshot& OutSnapshot) const;

    // 저장한 대검 런타임 상태를 복구한다.
    void RestoreRuntimeSnapshot(const FMHGreatSwordRuntimeSnapshot& InSnapshot);

    // 현재 문맥에서 대검 회피 입력을 해석한다.
    bool HandleDodgePressed(bool bInSheathed, EMHDirectionalVariant InDirectionalVariant);

    // 현재 대검 액션 상태를 조회한다.
    EMHGreatSwordActionState GetActionState() const { return ActionState; }

    // 현재 차징 계열을 조회한다.
    EMHGreatSwordChargeFamily GetChargeFamily() const { return ChargeFamily; }

    // 현재 차징 단계를 조회한다.
    int32 GetChargeLevel() const { return CurrentChargeLevel; }

    // 현재 액션에서 사용할 차징 단계별 데미지 배율을 계산한다.
    float ResolveChargeDamageScaleForMove(const FGameplayTag& InMoveTag) const;

    // 실행한 공격 기술 태그를 기록한다.
    void CommitExecutedMove(const FGameplayTag& InMoveTag);

    // 유틸리티 몽타주 시작 상태를 기록한다.
    void NotifyUtilityMoveStarted(const FGameplayTag& InMoveTag);

    // 유틸리티 몽타주 종료 시 런타임 상태를 정리한다.
    void NotifyUtilityMoveEnded(const FGameplayTag& InMoveTag, bool bInterrupted);

    // 액션 종료 후 중립 상태로 되돌린다.
    void NotifyActionFinished();

    // 기술 태그에 대응하는 몽타주를 찾는다.
    UAnimMontage* ResolveMontageForMove(const FGameplayTag& InMoveTag) const;

    // 기술 태그에 대응하는 공격 메타를 찾는다.
    bool FindAttackMetaRow(const FGameplayTag& InMoveTag, FMHAttackMetaRow& OutAttackMetaRow) const;

    // 차징 단계 노티파이에서 현재 차징 단계를 갱신한다.
    void NotifyChargeLevelReached(int32 InChargeLevel);

    // 최대 차징 시점 노티파이에서 자동 릴리즈를 요청한다.
    bool NotifyChargeAutoReleaseRequested();

    // 공격 후 4방향 구르기 윈도우를 연다.
    void NotifyBeginAttackRollWindow();

    // 공격 후 4방향 구르기 윈도우를 닫는다.
    void NotifyEndAttackRollWindow();

    // 공격 조기 전환 윈도우를 연다.
    void NotifyBeginEarlyTransitionWindow();

    // 공격 조기 전환 윈도우를 닫는다.
    void NotifyEndEarlyTransitionWindow();

    // 다음 단계 차징 후속 윈도우를 연다.
    void NotifyBeginChargeFollowUpWindow(const FGameplayTag& InSourceMoveTag);

    // 다음 단계 차징 후속 윈도우를 닫는다.
    void NotifyEndChargeFollowUpWindow();

    // 현재 문맥에서 사용할 구르기 기술 태그를 결정한다.
    FGameplayTag ResolveDodgeMoveTag(bool bInSheathed, EMHDirectionalVariant InDirectionalVariant) const;

    // 현재 기술이 유틸리티 몽타주 직재생 대상인지 확인한다.
    bool IsUtilityMoveTag(const FGameplayTag& InMoveTag) const;

protected:
    // 차징 계열에 대응하는 차징 시작 몽타주 태그를 반환한다.
    FGameplayTag ResolveChargeChargingMoveTag(EMHGreatSwordChargeFamily InFamily) const;

    // 차징 계열에 대응하는 실제 릴리즈 공격 태그를 반환한다.
    FGameplayTag ResolveChargeReleaseMoveTag(EMHGreatSwordChargeFamily InFamily) const;

    // 차징 중 태클 후 다음 차징 계열을 계산한다.
    EMHGreatSwordChargeFamily ResolveNextChargeFamilyAfterTackleFromCharge(EMHGreatSwordChargeFamily InCurrentFamily) const;

    // 현재 차징 컨텍스트를 초기화한다.
    void ResetChargeContext();

    // 조기 전환 버퍼 입력을 비운다.
    void ClearBufferedInput();

    // 입력을 조기 전환 버퍼에 저장한다.
    void BufferInput(EMHGreatSwordBufferedInputType InInputType, bool bInForwardInput);

    // 조기 전환 윈도우가 열렸을 때 버퍼 입력을 소비한다.
    bool TryConsumeBufferedTransitionInput();

    // 현재 공격 중 조기 전환 가능한 문맥에서 좌클릭 입력을 해석한다.
    bool ResolvePrimaryDuringEarlyTransition(bool bInForwardInput);

    // 현재 공격 중 조기 전환 가능한 문맥에서 우클릭 입력을 해석한다.
    bool ResolveSecondaryDuringEarlyTransition();

    // 현재 공격 중 조기 전환 가능한 문맥에서 Mouse4 입력을 해석한다.
    bool ResolveWeaponSpecialDuringEarlyTransition();

    // 현재 공격 중 조기 전환 가능한 문맥에서 Mouse5 입력을 해석한다.
    bool ResolveSimultaneousDuringEarlyTransition();

    // 새 대기 기술을 기록하고 액션 상태를 갱신한다.
    void QueuePendingMove(const FGameplayTag& InMoveTag, EMHGreatSwordActionState InNextState);

    // 차징 시작 상태를 기록한다.
    void BeginCharging(EMHGreatSwordChargeFamily InChargeFamily, bool bInStartedFromSheathedForwardInput);

    // 현재 차징을 실제 릴리즈 공격으로 확정한다.
    bool FinalizeCurrentChargeRelease();

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Data")
    TObjectPtr<UDataTable> AttackMetaTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Animation")
    TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>> MoveMontageMap;

    // 모아베기 릴리즈 시 차징 단계별 데미지 배율을 적용한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Charge")
    FVector ChargeSlashLevelDamageScale = FVector(1.0f, 1.2f, 1.45f);

    // 강모아베기 릴리즈 시 차징 단계별 데미지 배율을 적용한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Charge")
    FVector StrongChargeSlashLevelDamageScale = FVector(1.0f, 1.25f, 1.55f);

    // 참모아베기 릴리즈 시 차징 단계별 데미지 배율을 적용한다.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Charge")
    FVector TrueChargeSlashLevelDamageScale = FVector(1.0f, 1.3f, 1.65f);

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

    // 태클 직전 차징 계열을 기록해 태클 후 다음 단계 차징 판정에 사용한다.
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
};
