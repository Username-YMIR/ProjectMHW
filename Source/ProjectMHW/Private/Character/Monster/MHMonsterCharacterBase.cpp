// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-06


#include "Character/Monster/MHMonsterCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "MHGameplayTags.h"
#include "Character/Monster/Attribute/MHMonsterAttributeSet.h"
#include "Components/CapsuleComponent.h"
#include "DataAsset/MHMonsterDataAsset.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(MonsterCharacter)

AMHMonsterCharacterBase::AMHMonsterCharacterBase()
{
    MonsterAttributes = CreateDefaultSubobject<UMHMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
    AttributeSet = MonsterAttributes;
}

void AMHMonsterCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    InitMonsterGAS();

    // 시작 상태: Unaware
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::State_Monster_Unaware);
    }

    // 시작부터 Roar 체크하는 게 아니라 "시야 감지"만 시작
    StartSightDetection();
}

void AMHMonsterCharacterBase::SetCombatTarget(AActor* NewTarget)
{
    CombatTarget = NewTarget;
}

bool AMHMonsterCharacterBase::IsUnaware() const
{
    if (!AbilitySystemComponent)
    {
        return false;
    }

    return !AbilitySystemComponent->HasMatchingGameplayTag(MHGameplayTags::State_Monster_Alert)
        && !AbilitySystemComponent->HasMatchingGameplayTag(MHGameplayTags::State_Monster_Roaring)
        && !AbilitySystemComponent->HasMatchingGameplayTag(MHGameplayTags::State_Monster_Combat);
}

void AMHMonsterCharacterBase::StartSightDetection()
{
    if (!GetWorldTimerManager().IsTimerActive(SightDetectTimer))
    {
        GetWorldTimerManager().SetTimer(
            SightDetectTimer,
            this,
            &AMHMonsterCharacterBase::CheckSightDetection,
            SightDetectInterval,
            true
        );

        UE_LOG(MonsterCharacter, Warning, TEXT("StartSightDetection | timer started"));
    }
}

void AMHMonsterCharacterBase::StopSightDetection()
{
    GetWorldTimerManager().ClearTimer(SightDetectTimer);
}

void AMHMonsterCharacterBase::CheckSightDetection()
{
    if (!IsUnaware())
    {
        return;
    }

    ACharacter* PlayerCharacter = UGameplayStatics::GetPlayerCharacter(this, 0);
    if (!PlayerCharacter)
    {
        return;
    }

    if (CanSeeTargetFromHead(PlayerCharacter))
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("CheckSightDetection | player detected by sight"));
        HandleSightDetected(PlayerCharacter);
    }
}

bool AMHMonsterCharacterBase::CanSeeTargetFromHead(AActor* Target) const
{
    if (!Target)
    {
        return false;
    }

    const USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        return false;
    }

    if (!MeshComp->DoesSocketExist(HeadLookSocketName))
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("CanSeeTargetFromHead | missing socket: %s"), *HeadLookSocketName.ToString());
        return false;
    }

    const FVector SocketLocation = MeshComp->GetSocketLocation(HeadLookSocketName);
    const FRotator SocketRotation = MeshComp->GetSocketRotation(HeadLookSocketName);
    const FVector SocketForward = SocketRotation.Vector();

    FVector TargetLocation = Target->GetActorLocation();

    if (const ACharacter* TargetCharacter = Cast<ACharacter>(Target))
    {
        if (const UCapsuleComponent* Capsule = TargetCharacter->GetCapsuleComponent())
        {
            TargetLocation.Z += Capsule->GetScaledCapsuleHalfHeight() * 0.8f;
        }
        else
        {
            TargetLocation.Z += SightTargetHeightOffset;
        }
    }
    else
    {
        TargetLocation.Z += SightTargetHeightOffset;
    }

    FVector ToTarget = TargetLocation - SocketLocation;
    const float Distance = ToTarget.Size();

    if (Distance > SightDetectRange)
    {
        return false;
    }

    // 너무 가까우면 각도 불안정하니 바로 통과
    if (Distance < AutoPassCloseRange)
    {
        return true;
    }

    ToTarget.Normalize();

    // 머리 소켓 기준 forward
    const FVector LocalDir = SocketRotation.UnrotateVector(ToTarget);

    if (LocalDir.X <= 0.f)
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("Sight Fail | target behind head socket"));
        return false;
    }

    const float YawDeg =
        FMath::RadiansToDegrees(FMath::Atan2(LocalDir.Y, LocalDir.X));

    const float PitchDeg =
        FMath::RadiansToDegrees(FMath::Atan2(LocalDir.Z, LocalDir.X));

    UE_LOG(MonsterCharacter, Warning,
        TEXT("Sight Debug | Dist=%.2f Yaw=%.2f Pitch=%.2f | LimitYaw=%.2f LimitPitch=%.2f"),
        Distance, YawDeg, PitchDeg, SightHorizontalHalfAngleDeg, SightVerticalHalfAngleDeg);

    if (FMath::Abs(YawDeg) > SightHorizontalHalfAngleDeg)
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("Sight Fail | yaw out of range"));
        return false;
    }

    if (FMath::Abs(PitchDeg) > SightVerticalHalfAngleDeg)
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("Sight Fail | pitch out of range"));
        return false;
    }

    if (bSightRequireLineOfSight)
    {
        FHitResult HitResult;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterSightLOS), false, this);
        Params.AddIgnoredActor(this);

        const bool bHit = GetWorld()->LineTraceSingleByChannel(
            HitResult,
            SocketLocation + SocketForward * 20.f,
            TargetLocation,
            ECC_Visibility,
            Params
        );

        if (!bHit)
        {
            // 플레이어가 Visibility를 막지 않으면 no hit가 날 수 있음
            // 여기선 "장애물 확인" 용이 아니라 "타겟 직접 적중" 방식
            UE_LOG(MonsterCharacter, Warning, TEXT("Sight Fail | LOS no hit"));
            return false;
        }

        if (HitResult.GetActor() != Target)
        {
            UE_LOG(MonsterCharacter, Warning, TEXT("Sight Fail | LOS blocked by %s"), *GetNameSafe(HitResult.GetActor()));
            return false;
        }
    }

    UE_LOG(MonsterCharacter, Warning, TEXT("Sight Success"));
    return true;
}

void AMHMonsterCharacterBase::HandleSightDetected(AActor* Target)
{
    if (!AbilitySystemComponent || !Target)
    {
        return;
    }

    if (!IsUnaware())
    {
        return;
    }

    SetCombatTarget(Target);

    AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Unaware);
    AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::State_Monster_Alert);
    AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::Event_Monster_DetectedBySight);

    UE_LOG(MonsterCharacter, Warning, TEXT("HandleSightDetected | Alert -> StartRoar"));

    StartRoar();
}

void AMHMonsterCharacterBase::HandleDamagedFromUnaware(AActor* InstigatorActor)
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    if (!IsUnaware())
    {
        return;
    }

    SetCombatTarget(InstigatorActor);

    AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Unaware);
    AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::Event_Monster_AttackedFromUnaware);

    UE_LOG(MonsterCharacter, Warning, TEXT("HandleDamagedFromUnaware | skip roar -> EnterCombatPhase"));

    EnterCombatPhase();
}

void AMHMonsterCharacterBase::StartRoar()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::State_Monster_Roaring);

    if (!RoarMontage)
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("StartRoar | RoarMontage null -> fallback combat"));
        OnRoarFinished();
        return;
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("StartRoar | MeshComp null -> fallback combat"));
        OnRoarFinished();
        return;
    }

    UAnimInstance* Anim = MeshComp->GetAnimInstance();
    if (!Anim)
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("StartRoar | AnimInstance null -> fallback combat"));
        OnRoarFinished();
        return;
    }

    if (Anim->Montage_IsPlaying(RoarMontage))
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("StartRoar | already playing"));
        return;
    }

    const float PlayedLen = Anim->Montage_Play(RoarMontage, 1.0f);
    UE_LOG(MonsterCharacter, Warning, TEXT("StartRoar | Montage_Play = %f"), PlayedLen);

    if (PlayedLen > 0.f)
    {
        bHasRoared = true;
        StopSightDetection();
    }
    else
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("StartRoar | play failed -> fallback combat"));
        OnRoarFinished();
    }
}

void AMHMonsterCharacterBase::OnRoarFinished()
{
    UE_LOG(MonsterCharacter, Warning, TEXT("OnRoarFinished | roar end -> combat phase"));

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Roaring);
    }

    EnterCombatPhase();
}

void AMHMonsterCharacterBase::EnterCombatPhase()
{
    if (bInCombat)
    {
        return;
    }

    bInCombat = true;

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Alert);
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Roaring);
        AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::State_Monster_Combat);
    }

    StopSightDetection();
    EnterCombat();

    UE_LOG(MonsterCharacter, Warning, TEXT("EnterCombatPhase | combat started"));
}

void AMHMonsterCharacterBase::EnterCombat()
{
    UE_LOG(MonsterCharacter, Warning, TEXT("EnterCombat | base combat entered"));
}

void AMHMonsterCharacterBase::ExitCombat()
{
    bInCombat = false;
    CombatTarget = nullptr;

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Alert);
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Roaring);
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Combat);
        AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::State_Monster_Unaware);
    }

    StartSightDetection();

    UE_LOG(MonsterCharacter, Warning, TEXT("ExitCombat | return to unaware"));
}

void AMHMonsterCharacterBase::NotifyDamagedFrom(AActor* InstigatorActor)
{
    if (!InstigatorActor)
    {
        return;
    }

    UE_LOG(MonsterCharacter, Warning, TEXT("NotifyDamagedFrom | damaged by %s"), *GetNameSafe(InstigatorActor));

    if (IsUnaware())
    {
        HandleDamagedFromUnaware(InstigatorActor);
    }
}

void AMHMonsterCharacterBase::InitMonsterGAS()
{
    if (bMonsterGASInitialized)
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("InitMonsterGAS | already initialized"));
        return;
    }

    UE_LOG(MonsterCharacter, Warning, TEXT("InitMonsterGAS | start"));

    if (!AbilitySystemComponent)
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("InitMonsterGAS | AbilitySystemComponent null"));
        return;
    }

    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    ApplyStartupLooseTags();
    GrantStartupAbilities();
    ApplyStartupEffects();

    bMonsterGASInitialized = true;
    bGASInitialized = true;

    if (MonsterAttributes)
    {
        UE_LOG(LogTemp, Warning, TEXT("[MonsterGAS] %s HP=%f/%f Poise=%f/%f Atk=%f Def=%f"),
            *GetName(),
            MonsterAttributes->GetHealth(), MonsterAttributes->GetMaxHealth(),
            MonsterAttributes->GetPoise(), MonsterAttributes->GetMaxPoise(),
            MonsterAttributes->GetAttackPower(), MonsterAttributes->GetDefense()
        );
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[MonsterGAS] %s MonsterAttributes is NULL"), *GetName());
    }
}

void AMHMonsterCharacterBase::ApplyStartupLooseTags()
{
    UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
    if (!MonsterDataAsset || !AbilitySystemComponent)
    {
        return;
    }

    for (const FGameplayTag& Tag : MonsterDataAsset->MonsterTags)
    {
        AbilitySystemComponent->AddLooseGameplayTag(Tag);
    }
}

void AMHMonsterCharacterBase::GrantStartupAbilities()
{
    UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
    if (!MonsterDataAsset || !AbilitySystemComponent)
    {
        return;
    }

    for (const TSubclassOf<UGameplayAbility>& AbilityClass : MonsterDataAsset->StartupAbilities)
    {
        if (!AbilityClass)
        {
            continue;
        }

        AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
    }
}

void AMHMonsterCharacterBase::ApplyStartupEffects()
{
    UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
    if (!MonsterDataAsset || !AbilitySystemComponent)
    {
        return;
    }

    for (const TSubclassOf<UGameplayEffect>& EffectClass : MonsterDataAsset->StartupEffects)
    {
        if (!EffectClass)
        {
            continue;
        }

        FGameplayEffectContextHandle ContextHandle = AbilitySystemComponent->MakeEffectContext();
        ContextHandle.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle =
            AbilitySystemComponent->MakeOutgoingSpec(EffectClass, 1.f, ContextHandle);

        if (SpecHandle.IsValid())
        {
            AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
            UE_LOG(MonsterCharacter, Warning, TEXT("ApplyStartupEffects | applied %s"), *GetNameSafe(EffectClass));
        }
    }
}
