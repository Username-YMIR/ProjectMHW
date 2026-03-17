#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "mh_debug_damage_trigger.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHDebugDamageTrigger, Log, All);

class UBoxComponent;
class AMHPlayerCharacter;

UCLASS()
class PROJECTMHW_API AMHDebugDamageTrigger : public AActor
{
    GENERATED_BODY()

public:
    AMHDebugDamageTrigger();

protected:
    virtual void BeginPlay() override;

protected:
#pragma region Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug|Damage")
    TObjectPtr<UBoxComponent> TriggerBox;
#pragma endregion

#pragma region DebugDamage
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug|Damage")
    float PhysicalDamage = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug|Damage")
    FGameplayTag AttackTag;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug|Damage")
    bool bRepeatable = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug|Damage", meta = (ClampMin = "0.0"))
    float TriggerCooldown = 0.25f;

    UPROPERTY(Transient)
    TMap<TObjectPtr<AMHPlayerCharacter>, float> PlayerNextTriggerTimeMap;
#pragma endregion

protected:
    UFUNCTION()
    void HandleTriggerBoxBeginOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    bool CanTriggerForPlayer(AMHPlayerCharacter* InPlayer) const;
    void TriggerDamageForPlayer(AMHPlayerCharacter* InPlayer);
};
