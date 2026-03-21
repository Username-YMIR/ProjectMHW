#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
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
    TrueCharge  UMETA(DisplayName = "True")
};

UENUM(BlueprintType)
enum class EMHGreatSwordActionState : uint8
{
    None        UMETA(DisplayName = "None"),
    Neutral     UMETA(DisplayName = "Neutral"),
    Charging    UMETA(DisplayName = "Charging"),
    Acting      UMETA(DisplayName = "Acting")
};

UCLASS(ClassGroup=(Weapon), Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class PROJECTMHW_API UMHGreatSwordActionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMHGreatSwordActionComponent();

    virtual void TickComponent(
        float DeltaTime,
        enum ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    // 좌클릭 입력 시작 시 대검 기술을 결정한다.
    bool HandlePrimaryPressed(bool bInForwardInput, bool bInSheathed);

    // 좌클릭 입력 종료 시 현재 차징 단계를 확정한다.
    bool HandlePrimaryReleased();

    // 우클릭 입력 시 베어넘기기 또는 태클 파생을 결정한다.
    bool HandleSecondaryPressed();

    // Mouse4 입력 시 가드 또는 옆으로 치기를 결정한다.
    bool HandleWeaponSpecialPressed();

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

    // 현재 대검 액션 상태를 조회한다.
    EMHGreatSwordActionState GetActionState() const { return ActionState; }

    // 현재 차징 계열을 조회한다.
    EMHGreatSwordChargeFamily GetChargeFamily() const { return ChargeFamily; }

    // 현재 차징 단계를 조회한다.
    int32 GetChargeLevel() const;

    // 실행한 기술 태그를 기록한다.
    void CommitExecutedMove(const FGameplayTag& InMoveTag);

    // 액션 종료 후 중립 상태로 되돌린다.
    void NotifyActionFinished();

    // 기술 태그에 대응하는 몽타주를 찾는다.
    UAnimMontage* ResolveMontageForMove(const FGameplayTag& InMoveTag) const;

    // 기술 태그에 대응하는 공격 메타를 찾는다.
    bool FindAttackMetaRow(const FGameplayTag& InMoveTag, FMHAttackMetaRow& OutAttackMetaRow) const;

protected:
    // 차징 계열과 단계로 실제 발동 태그를 만든다.
    FGameplayTag ResolveChargeReleaseMoveTag(EMHGreatSwordChargeFamily InFamily, int32 InChargeLevel) const;

    // 현재까지 확정한 기술 기준으로 다음 차징 계열을 계산한다.
    EMHGreatSwordChargeFamily ResolveNextChargeFamilyAfterTackle() const;

    // 현재 차징 시간을 기준으로 단계를 계산한다.
    int32 ResolveChargeLevelFromElapsedSeconds(float InElapsedSeconds) const;

    // 새 대기 기술을 기록하고 액션 상태를 갱신한다.
    void QueuePendingMove(const FGameplayTag& InMoveTag, EMHGreatSwordActionState InNextState);

    // 차징 시작 상태를 기록한다.
    void BeginCharging(EMHGreatSwordChargeFamily InChargeFamily, bool bInForwardDrawEntry = false);

    // 차징 상태를 종료한다.
    void EndCharging();

    // 현재 차징을 자동 발동해야 하는지 검사한다.
    bool TryAutoReleaseCharge();

    // 소유자 기준으로 대기 기술 어빌리티를 바로 실행한다.
    bool TryActivateQueuedMoveFromOwner() const;

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Data")
    TObjectPtr<UDataTable> AttackMetaTable;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Animation")
    TMap<FGameplayTag, TSoftObjectPtr<UAnimMontage>> MoveMontageMap;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Charge", meta = (ClampMin = "0.0"))
    float ChargeLevel1Seconds = 0.35f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Charge", meta = (ClampMin = "0.0"))
    float ChargeLevel2Seconds = 0.8f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GreatSword|Charge", meta = (ClampMin = "0.0"))
    float ChargeAutoReleaseSeconds = 1.2f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    EMHGreatSwordActionState ActionState = EMHGreatSwordActionState::Neutral;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    EMHGreatSwordChargeFamily ChargeFamily = EMHGreatSwordChargeFamily::None;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    FGameplayTag PendingMoveTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    FGameplayTag LastCommittedMoveTag;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    float ChargeStartWorldSeconds = 0.0f;

    // 발도 전방 입력으로 차징을 시작했는지 기록한다.
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GreatSword|Runtime")
    bool bForwardDrawChargeEntry = false;
};
