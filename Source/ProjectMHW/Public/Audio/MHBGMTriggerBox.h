#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MHBGMTriggerBox.generated.h"

class UAudioComponent;
class UBoxComponent;
class USoundBase;
class AMHMonsterCharacterBase;
class UPrimitiveComponent;
struct FHitResult;

DECLARE_LOG_CATEGORY_EXTERN(LogMHBGMTriggerBox, Log, All);

UENUM(BlueprintType)
enum class EMHBGMTriggerState : uint8
{
    Default,
    Combat,
    Clear
};

UCLASS()
class PROJECTMHW_API AMHBGMTriggerBox : public AActor
{
    GENERATED_BODY()

public:
    AMHBGMTriggerBox();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION()
    void HandleTriggerBoxBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void HandleTargetMonsterDied(AMHMonsterCharacterBase* DeadMonster);

    void BindTargetMonsterDelegates();
    void UnbindTargetMonsterDelegates();
    void EvaluateClearState();
    void RequestBGMState(EMHBGMTriggerState NewState);
    void PlayPendingTrack();
    void PlayTrackImmediately(USoundBase* InSound, EMHBGMTriggerState InState);
    USoundBase* ResolveSoundForState(EMHBGMTriggerState InState) const;
    bool AreAllTargetMonstersDead() const;

protected:
    // ===== Components =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UBoxComponent> TriggerBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
    TObjectPtr<UAudioComponent> BGMComponent;
    // ===== End Components =====

    // ===== Tracks =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BGM|Tracks")
    TObjectPtr<USoundBase> DefaultBGM = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BGM|Tracks")
    TObjectPtr<USoundBase> CombatBGM = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BGM|Tracks")
    TObjectPtr<USoundBase> ClearBGM = nullptr;
    // ===== End Tracks =====

    // ===== Transition =====
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BGM|Transition", meta=(ClampMin="0.0"))
    float InitialFadeInDuration = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="BGM|Transition", meta=(ClampMin="0.0"))
    float TransitionFadeDuration = 1.0f;
    // ===== End Transition =====

    // ===== State =====
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BGM|State")
    bool bCombatTriggered = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="BGM|State")
    EMHBGMTriggerState CurrentBGMState = EMHBGMTriggerState::Default;

    UPROPERTY(Transient)
    TObjectPtr<USoundBase> PendingTrack = nullptr;

    UPROPERTY(Transient)
    EMHBGMTriggerState PendingBGMState = EMHBGMTriggerState::Default;
    // ===== End State =====

    FTimerHandle BGMTransitionTimer;
    TArray<TWeakObjectPtr<AMHMonsterCharacterBase>> BoundTargetMonsters;
};
