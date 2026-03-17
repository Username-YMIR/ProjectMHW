#include "Combat/Debug/mh_debug_damage_trigger.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Components/BoxComponent.h"

DEFINE_LOG_CATEGORY(LogMHDebugDamageTrigger);

AMHDebugDamageTrigger::AMHDebugDamageTrigger()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
    SetRootComponent(TriggerBox);

    TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    TriggerBox->SetGenerateOverlapEvents(true);
    TriggerBox->SetBoxExtent(FVector(80.0f, 80.0f, 80.0f));
}

void AMHDebugDamageTrigger::BeginPlay()
{
    Super::BeginPlay();

    if (ensure(TriggerBox))
    {
        TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AMHDebugDamageTrigger::HandleTriggerBoxBeginOverlap);
    }
}

void AMHDebugDamageTrigger::HandleTriggerBoxBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    AMHPlayerCharacter* PlayerCharacter = Cast<AMHPlayerCharacter>(OtherActor);
    if (!PlayerCharacter)
    {
        return;
    }

    if (!CanTriggerForPlayer(PlayerCharacter))
    {
        return;
    }

    TriggerDamageForPlayer(PlayerCharacter);
}

bool AMHDebugDamageTrigger::CanTriggerForPlayer(AMHPlayerCharacter* InPlayer) const
{
    if (!IsValid(InPlayer))
    {
        return false;
    }

    if (bRepeatable)
    {
        const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
        if (const float* NextTriggerTime = PlayerNextTriggerTimeMap.Find(InPlayer))
        {
            if (CurrentTime < *NextTriggerTime)
            {
                return false;
            }
        }

        return true;
    }

    return !PlayerNextTriggerTimeMap.Contains(InPlayer);
}

void AMHDebugDamageTrigger::TriggerDamageForPlayer(AMHPlayerCharacter* InPlayer)
{
    if (!IsValid(InPlayer))
    {
        return;
    }

    InPlayer->ApplyDebugDamageFromSource(this, PhysicalDamage, AttackTag);

    const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
    if (bRepeatable)
    {
        PlayerNextTriggerTimeMap.Add(InPlayer, CurrentTime + TriggerCooldown);
    }
    else
    {
        PlayerNextTriggerTimeMap.Add(InPlayer, TNumericLimits<float>::Max());
    }

    UE_LOG(
        LogMHDebugDamageTrigger,
        Log,
        TEXT("%s : Debug damage triggered for %s. Physical=%.2f AttackTag=%s Repeatable=%s"),
        *GetName(),
        *GetNameSafe(InPlayer),
        PhysicalDamage,
        AttackTag.IsValid() ? *AttackTag.ToString() : TEXT("None"),
        bRepeatable ? TEXT("true") : TEXT("false")
    );
}
