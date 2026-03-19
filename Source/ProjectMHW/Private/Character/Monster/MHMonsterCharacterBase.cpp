// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-13


#include "Character/Monster/MHMonsterCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "MHGameplayTags.h"
#include "NavigationSystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Character/Monster/AI/MHMonsterAIController.h"
#include "Character/Monster/AI/MHMonsterBlackboardKeys.h"
#include "Character/Monster/Attribute/MHMonsterAttributeSet.h"
#include "Combat/Data/MHCombatDataLibrary.h"
#include "Combat/Effects/MHGameplayEffect_Damage.h"
#include "Interfaces/MHDamageSpecReceiverInterface.h"
#include "Components/CapsuleComponent.h"
#include "DataAsset/MHMonsterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Combat/Attributes/MHCombatAttributeSet.h"

DEFINE_LOG_CATEGORY(MHMonsterCharacterBase)

AMHMonsterCharacterBase::AMHMonsterCharacterBase()
{

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = 180.f;   //속도  테스트용
        GetCharacterMovement()->bOrientRotationToMovement = true;
        GetCharacterMovement()->RotationRate = FRotator(0.f, 360.f, 0.f);
    }

    bUseControllerRotationYaw = false;

    MonsterAttributes = CreateDefaultSubobject<UMHMonsterAttributeSet>(TEXT("MonsterAttributeSet"));
    CombatAttributeSets = CreateDefaultSubobject<UMHCombatAttributeSet>(TEXT("CombatAttributeSet"));
    HealthAttributeSets = CreateDefaultSubobject<UMHHealthAttributeSet>(TEXT("HealthAttributeSet"));
    
    MonsterDamageEffectClass = UMHGameplayEffect_Damage::StaticClass();
}

void AMHMonsterCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("BeginPlay | Enter"));

    InitMonsterGAS();

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("BeginPlay | After InitMonsterGAS"));


    // 시작 상태: Unaware
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::State_Monster_Unaware);
    }
    
    //ambient 시작 
    SetMonsterMoveSpeed(AmbientWalkSpeed);
    StartAmbientBehavior();
    // 시작부터 Roar 체크하는 게 아니라 "시야 감지"만 시작
    StartSightDetection();
    SetMonsterAttacking(false);
    
}

bool AMHMonsterCharacterBase::TryActivateMonsterAbilityByTag(FGameplayTag AbilityTag)
{
    
    if (HasDeadTag())
    {
        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("TryActivateMonsterAbilityByTag | blocked, dead | Tag=%s"),
            *AbilityTag.ToString());
        return false;
    }

    if (!AbilitySystemComponent)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("TryActivateMonsterAbilityByTag | ASC is null"));
        return false;
    }

    if (!AbilityTag.IsValid())
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("TryActivateMonsterAbilityByTag | AbilityTag invalid"));
        return false;
    }

    if (IsMonsterAbilityOnCooldown(AbilityTag))
    {
        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("TryActivateMonsterAbilityByTag | Tag=%s is on cooldown (Remaining=%.2f)"),
            *AbilityTag.ToString(),
            GetMonsterAbilityCooldownRemaining(AbilityTag));
        return false;
    }

    for (const FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
    {
        if (Spec.Ability)
        {
            UE_LOG(MHMonsterCharacterBase, Warning, TEXT("  Owned Ability = %s"),
                *Spec.Ability->GetClass()->GetName());

            UE_LOG(MHMonsterCharacterBase, Warning, TEXT("  Ability Tags = %s"),
                *Spec.Ability->AbilityTags.ToStringSimple());

            UE_LOG(MHMonsterCharacterBase, Warning, TEXT("  Dynamic Tags = %s"),
                *Spec.DynamicAbilityTags.ToStringSimple());
        }
    }

    FaceCombatTargetInstant();

    bool bActivated = false;

    for (FGameplayAbilitySpec& Spec : AbilitySystemComponent->GetActivatableAbilities())
    {
        if (!Spec.Ability)
        {
            continue;
        }

        const bool bMatchAssetTag = Spec.Ability->AbilityTags.HasTagExact(AbilityTag);
        const bool bMatchDynamicTag = Spec.DynamicAbilityTags.HasTagExact(AbilityTag);

        if (!bMatchAssetTag && !bMatchDynamicTag)
        {
            continue;
        }

        bActivated = AbilitySystemComponent->TryActivateAbility(Spec.Handle);

        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("TryActivateMonsterAbilityByTag | Try Spec=%s | DynamicTags=%s | Result=%s"),
            *GetNameSafe(Spec.Ability),
            *Spec.DynamicAbilityTags.ToStringSimple(),
            bActivated ? TEXT("Success") : TEXT("Fail"));

        if (bActivated)
        {
            break;
        }
    }

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("TryActivateMonsterAbilityByTag | Tag=%s Result=%s"),
        *AbilityTag.ToString(),
        bActivated ? TEXT("Success") : TEXT("Fail"));

    // 성공시 쿨타임 시작 
    if (bActivated)
    {
        StartMonsterAbilityCooldown(AbilityTag);
    }

    return bActivated;
}

bool AMHMonsterCharacterBase::GetMonsterAbilityEntryByTag(FGameplayTag AbilityTag, FMonsterAbilityEntry& OutEntry) const
{
    return FindMonsterAbilityEntryByTag(AbilityTag, OutEntry);
}

float AMHMonsterCharacterBase::GetDistanceToCombatTarget() const
{
    if (!CombatTarget)
    {
        return TNumericLimits<float>::Max();
    }

    const float CenterDist = FVector::Dist2D(GetActorLocation(), CombatTarget->GetActorLocation());

    float SelfRadius = 0.f;
    if (const UCapsuleComponent* MyCapsule = GetCapsuleComponent())
    {
        SelfRadius = MyCapsule->GetScaledCapsuleRadius();
    }

    float TargetRadius = 0.f;
    if (const ACharacter* TargetCharacter = Cast<ACharacter>(CombatTarget))
    {
        if (const UCapsuleComponent* TargetCapsule = TargetCharacter->GetCapsuleComponent())
        {
            TargetRadius = TargetCapsule->GetScaledCapsuleRadius();
        }
    }
    else if (const UCapsuleComponent* TargetCapsule = CombatTarget->FindComponentByClass<UCapsuleComponent>())
    {
        TargetRadius = TargetCapsule->GetScaledCapsuleRadius();
    }

    const float AdjustedDist = CenterDist - SelfRadius - TargetRadius - CombatDistanceOffset;
    return FMath::Max(0.f, AdjustedDist);
}

bool AMHMonsterCharacterBase::IsCombatTargetInRange(float Range) const
{
    if (!CombatTarget)
    {
        return false;
    }

    return GetDistanceToCombatTarget() <= Range;
}

bool AMHMonsterCharacterBase::IsMonsterAttacking() const
{
    /*
    if (!AbilitySystemComponent)
    {
        UE_LOG(MonsterCharacter, Warning, TEXT("IsMonsterAttacking | !AbilitySystemComponent "));
        
        return false;
    }
    return AbilitySystemComponent->HasMatchingGameplayTag(MHGameplayTags::State_Monster_Attacking);
    */
    
    return bMonsterAttacking;
    
    
}

void AMHMonsterCharacterBase::SetMonsterAttacking(bool bNewAttacking)
{
    if (bMonsterAttacking == bNewAttacking)
    {
        return;
    }

    bMonsterAttacking = bNewAttacking;

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("SetMonsterAttacking | %s"),
        bMonsterAttacking ? TEXT("true") : TEXT("false"));
    // 블랙보드 
    if (AMHMonsterAIController* MonsterAI = GetMonsterAIController())
    {
        // BBKeys 로 이동 
        /*if (UBlackboardComponent* BB = MonsterAI->GetBlackboardComponent())
        {
            BB->SetValueAsBool(MHMonsterBBKeys::bAttacking, bMonsterAttacking);
        }*/
        
        MonsterAI->SetAttacking(bNewAttacking);
        
    }
}

void AMHMonsterCharacterBase::BeginMonsterAttackWindow()
{
    bMonsterAttackWindowOpen = true;
    bMonsterAttackHitConsumed = false;

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("BeginMonsterAttackWindow | Open=true Consumed=false"));
}

void AMHMonsterCharacterBase::EndMonsterAttackWindow()
{
    bMonsterAttackWindowOpen = false;
    bMonsterAttackHitConsumed = false;

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("EndMonsterAttackWindow | Open=false Consumed=false"));
}

bool AMHMonsterCharacterBase::CanMonsterAttackHitNow() const
{
    return bMonsterAttackWindowOpen && !bMonsterAttackHitConsumed;
    
}



void AMHMonsterCharacterBase::FaceCombatTargetInstant()
{
    if (!CombatTarget)
    {
        return;
    }

    FVector ToTarget = CombatTarget->GetActorLocation() - GetActorLocation();
    ToTarget.Z = 0.f;

    if (ToTarget.IsNearlyZero())
    {
        return;
    }

    const FRotator TargetRot = ToTarget.Rotation();
    SetActorRotation(FRotator(0.f, TargetRot.Yaw, 0.f));
}

void AMHMonsterCharacterBase::FaceCombatTargetInterp(float DeltaSeconds, float TurnSpeedDeg)
{
    if (!CombatTarget)
    {
        return;
    }

    FVector ToTarget = CombatTarget->GetActorLocation() - GetActorLocation();
    ToTarget.Z = 0.f;

    if (ToTarget.IsNearlyZero())
    {
        return;
    }

    const FRotator CurrentRot = GetActorRotation();
    const FRotator TargetRot(0.f, ToTarget.Rotation().Yaw, 0.f);

    const FRotator NewRot = FMath::RInterpConstantTo(
        CurrentRot,
        TargetRot,
        DeltaSeconds,
        TurnSpeedDeg
    );

    SetActorRotation(NewRot);
}


bool AMHMonsterCharacterBase::ConsumeMonsterAttackHitOnce(FGameplayTag AttackTag)
{
    if (!bMonsterAttackWindowOpen)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ConsumeMonsterAttackHitOnce | Window closed"));
        return false;
    }

    if (bMonsterAttackHitConsumed)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ConsumeMonsterAttackHitOnce | Already consumed"));
        return false;
    }

    if (!CombatTarget)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ConsumeMonsterAttackHitOnce | CombatTarget null"));
        return false;
    }

    if (!CombatTarget->GetClass()->ImplementsInterface(UMHDamageSpecReceiverInterface::StaticClass()))
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ConsumeMonsterAttackHitOnce | Target has no damage receiver interface"));
        return false;
    }

    FMonsterAbilityEntry AbilityEntry;

    float FinalPhysicalDamage = MonsterBasicPhysicalDamage;
    float FinalAttackRange = 220.f;

    if (FindMonsterAbilityEntryByTag(AttackTag, AbilityEntry))
    {
        FinalPhysicalDamage = AbilityEntry.PhysicalDamage;
        FinalAttackRange = AbilityEntry.AttackRange;

        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("ConsumeMonsterAttackHitOnce | Entry Found | Tag=%s Damage=%.1f Range=%.1f Cooldown=%.1f"),
            *AbilityEntry.AbilityTag.ToString(),
            FinalPhysicalDamage,
            FinalAttackRange,
            AbilityEntry.CooldownSeconds);
    }
    else
    {
        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("ConsumeMonsterAttackHitOnce | Entry Not Found | Tag=%s Fallback Damage=%.1f Range=%.1f"),
            *AttackTag.ToString(),
            FinalPhysicalDamage,
            FinalAttackRange);
    }

    const float Dist = FVector::Dist(GetActorLocation(), CombatTarget->GetActorLocation());
    if (Dist > FinalAttackRange)
    {
        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("ConsumeMonsterAttackHitOnce | Out of range | Dist=%.1f Range=%.1f"),
            Dist,
            FinalAttackRange);
        return false;
    }

    FGameplayEffectSpecHandle DamageSpecHandle;
    if (!BuildMonsterDamageSpec(FinalPhysicalDamage, DamageSpecHandle))
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ConsumeMonsterAttackHitOnce | BuildMonsterDamageSpec failed"));
        return false;
    }

    FHitResult HitResult;
    HitResult.Location = CombatTarget->GetActorLocation();
    HitResult.ImpactPoint = CombatTarget->GetActorLocation();
    HitResult.ImpactNormal = (CombatTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    HitResult.Normal = HitResult.ImpactNormal;

    const FMHHitAcknowledge HitAck =
        IMHDamageSpecReceiverInterface::Execute_ReceiveDamageSpec(
            CombatTarget,
            this,
            this,
            AttackTag,
            DamageSpecHandle,
            HitResult
        );

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("ConsumeMonsterAttackHitOnce | Accepted=%d Consume=%d StopWindow=%d ResultType=%d"),
        HitAck.bAcceptedHit ? 1 : 0,
        HitAck.bConsumeHitOnce ? 1 : 0,
        HitAck.bShouldStopAttackWindow ? 1 : 0,
        static_cast<int32>(HitAck.ResultType));

    if (HitAck.bConsumeHitOnce)
    {
        bMonsterAttackHitConsumed = true;
    }

    if (HitAck.bShouldStopAttackWindow)
    {
        EndMonsterAttackWindow();
    }

    return HitAck.bAcceptedHit;
}

bool AMHMonsterCharacterBase::BuildMonsterDamageSpec(float PhysicalDamage,
    FGameplayEffectSpecHandle& OutSpecHandle) const
{
    OutSpecHandle = FGameplayEffectSpecHandle();

    if (!AbilitySystemComponent)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("BuildMonsterDamageSpec | ASC null"));
        return false;
    }

    if (!MonsterDamageEffectClass)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("BuildMonsterDamageSpec | MonsterDamageEffectClass null"));
        return false;
    }

    FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
    EffectContext.AddInstigator(const_cast<AMHMonsterCharacterBase*>(this), const_cast<AMHMonsterCharacterBase*>(this));
    EffectContext.AddSourceObject(const_cast<AMHMonsterCharacterBase*>(this));

    FGameplayEffectSpecHandle SpecHandle =
        AbilitySystemComponent->MakeOutgoingSpec(MonsterDamageEffectClass, 1.f, EffectContext);

    if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("BuildMonsterDamageSpec | SpecHandle invalid"));
        return false;
    }

    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Physical, PhysicalDamage);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Fire, 0.f);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Water, 0.f);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Thunder, 0.f);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Ice, 0.f);
    SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Dragon, 0.f);

    OutSpecHandle = SpecHandle;
    return true;
}



FMHHitAcknowledge AMHMonsterCharacterBase::ReceiveDamageSpec_Implementation(
    AActor* SourceActor,
    AActor* SourceWeapon,
    FGameplayTag AttackTag,
    const FGameplayEffectSpecHandle& DamageSpecHandle,
    const FHitResult& HitResult
)
{
    FMHHitAcknowledge Result;
    if (HasDeadTag())
    {
        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("ReceiveDamageSpec_Implementation | ignored, already dead"));
       return Result;
    }
    
    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ReceiveDamageSpec_Implementation"));
    UE_LOG(MHMonsterCharacterBase, Log, TEXT("SourceActor : %s, SourceWeapon : %s, AttackTag : %s")
        , *SourceActor->GetName()
        , *SourceActor->GetName()
        , *AttackTag.GetTagName().ToString());
    
   

    // 1. 전달받은 DamageSpec 유효성 검사
    if (!DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid())
    {
        return Result;
    }

    // 2. 현재 피격 가능한 상태인지 검사
    if (!CanReceiveDamage(SourceActor, AttackTag, DamageSpecHandle, HitResult))
    {
        return Result;
    }

    // 3. 몬스터 자신의 ASC 조회
    UAbilitySystemComponent* TargetASC = GetCharacterASC();
    if (!IsValid(TargetASC))
    {
        return Result;
    }

    // 4. 전달받은 Spec을 자기 자신에게 적용
    const FActiveGameplayEffectHandle ActiveHandle =
        TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());

    // 5. 적용 성공 시 후처리 및 응답 작성
    if (ActiveHandle.WasSuccessfullyApplied())
    {
        Result.bAcceptedHit = true;
        Result.bConsumeHitOnce = true;
        Result.bShouldStopAttackWindow = false;
        Result.ResultType = EMHHitResultType::NormalHit;

        HandleDamageAccepted(SourceActor, SourceWeapon, AttackTag, HitResult);

        if (IsMonsterDead())
        {
            HandleDeath();
        }
    }

    return Result;
}

void AMHMonsterCharacterBase::HandleDamageAccepted(
	AActor* SourceActor,
	AActor* SourceWeapon,
	FGameplayTag AttackTag,
	const FHitResult& HitResult
)
{
	Super::HandleDamageAccepted(SourceActor, SourceWeapon, AttackTag, HitResult);

	// 피격 VFX
	PlayHitImpactFXByAttackTag(AttackTag, HitResult);

	// 피격 SFX
	PlayHitSoundByAttackTag(AttackTag, HitResult);

	// 이후 확장 예시
	// - 피격 리액션 재생
	// - AI에게 공격자 전달
	// - 경직치 누적
	// - 부위 파괴 누적
}

void AMHMonsterCharacterBase::HandleDeath()
{
	Super::HandleDeath();

    if (HasDeadTag())
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("HandleDeath | already dead"));
        return;
    }

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("HandleDeath | start"));

    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::State_Monster_Dead);

        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Alert);
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Roaring);
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Combat);
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Unaware);
    }

    StopAmbientBehavior();
    StopSightDetection();
    SetMonsterAttacking(false);

    bInCombat = false;
    CombatTarget = nullptr;

    if (AMHMonsterAIController* MonsterAI = GetMonsterAIController())
    {
        MonsterAI->SetAttacking(false);
        MonsterAI->SetInCombat(false);
        MonsterAI->SetIsRoaring(false);
        MonsterAI->SetCombatTarget(nullptr);
        MonsterAI->StopMovement();
    }

    if (GetCharacterMovement())
    {
        GetCharacterMovement()->DisableMovement();
        GetCharacterMovement()->StopMovementImmediately();
    }

    if (UCapsuleComponent* Capsule = GetCapsuleComponent())
    {
        Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("HandleDeath | Dead tag applied"));

}

void AMHMonsterCharacterBase::PlayHitImpactFXByAttackTag(
	FGameplayTag AttackTag,
	const FHitResult& HitResult
)
{
	if (!IsValid(AttackMetaTable))
	{
		UE_LOG(MHMonsterCharacterBase, Warning, TEXT("PlayHitImpactFXByAttackTag : AttackMetaTable is invalid"));
		return;
	}

	FMHAttackMetaRow AttackMetaRow;
	if (!UMHCombatDataLibrary::FindAttackMetaRowByTag(AttackMetaTable, AttackTag, AttackMetaRow))
	{
		UE_LOG(MHMonsterCharacterBase, Verbose, TEXT("PlayHitImpactFXByAttackTag : AttackMetaRow not found -> %s"), *AttackTag.ToString());
		return;
	}

	if (AttackMetaRow.HitEffectNiagara.IsNull())
	{
		return;
	}

	UNiagaraSystem* HitEffectNiagara = AttackMetaRow.HitEffectNiagara.LoadSynchronous();
	if (!IsValid(HitEffectNiagara))
	{
		UE_LOG(MHMonsterCharacterBase, Warning, TEXT("PlayHitImpactFXByAttackTag : Failed to load HitEffectNiagara -> %s"), *AttackTag.ToString());
		return;
	}

	const FVector SpawnLocation = HitResult.ImpactPoint;
	const FRotator SpawnRotation = AttackMetaRow.bUseDirectionalHitFX
		? HitResult.ImpactNormal.Rotation()
		: FRotator::ZeroRotator;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(
		GetWorld(),
		HitEffectNiagara,
		SpawnLocation,
		SpawnRotation
	);
    
    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("SpawnSystemAtLocation"))
}

void AMHMonsterCharacterBase::PlayHitSoundByAttackTag(
	FGameplayTag AttackTag,
	const FHitResult& HitResult
)
{
	if (!IsValid(AttackMetaTable))
	{
		UE_LOG(MHMonsterCharacterBase, Warning, TEXT("PlayHitSoundByAttackTag : AttackMetaTable is invalid"));
		return;
	}

	FMHAttackMetaRow AttackMetaRow;
	if (!UMHCombatDataLibrary::FindAttackMetaRowByTag(AttackMetaTable, AttackTag, AttackMetaRow))
	{
		UE_LOG(MHMonsterCharacterBase, Verbose, TEXT("PlayHitSoundByAttackTag : AttackMetaRow not found -> %s"), *AttackTag.ToString());
		return;
	}

	if (AttackMetaRow.HitSound.IsNull())
	{
		return;
	}

	USoundBase* HitSound = AttackMetaRow.HitSound.LoadSynchronous();
	if (!IsValid(HitSound))
	{
		UE_LOG(MHMonsterCharacterBase, Warning, TEXT("PlayHitSoundByAttackTag : Failed to load HitSound -> %s"), *AttackTag.ToString());
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(
		this,
		HitSound,
		HitResult.ImpactPoint
	);
    
    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("PlaySoundAtLocation"))

}


bool AMHMonsterCharacterBase::HasDeadTag() const
{
    if (!AbilitySystemComponent)
    {
        return false;
    }

    return AbilitySystemComponent->HasMatchingGameplayTag(MHGameplayTags::State_Monster_Dead);

}

bool AMHMonsterCharacterBase::IsMonsterDead() const
{
    if (HasDeadTag())
    {
        return true;
        
    }
    return HealthAttributeSets && HealthAttributeSets->GetHealth() <= 0.f;

    
}

void AMHMonsterCharacterBase::PlayDeathAnimation()
{
    if (!DeathMontage)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("PlayDeathAnimation | DeathMontage null"));
        return;
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("PlayDeathAnimation | MeshComp null"));
        return;
    }

    UAnimInstance* Anim = MeshComp->GetAnimInstance();
    if (!Anim)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("PlayDeathAnimation | AnimInstance null"));
        return;
    }

    const float PlayedLen = Anim->Montage_Play(DeathMontage, 1.0f);

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("PlayDeathAnimation | PlayedLen=%.2f Montage=%s"),
        PlayedLen,
        *GetNameSafe(DeathMontage));
    
}

AMHMonsterAIController* AMHMonsterCharacterBase::GetMonsterAIController() const
{
    return Cast<AMHMonsterAIController>(GetController());
}

#pragma region Ambient


// ambient 시작
void AMHMonsterCharacterBase::StartAmbientBehavior()
{
    if (bInCombat || bHasRoared)
    {
        return;
    }

    SetMonsterMoveSpeed(AmbientWalkSpeed);
    ScheduleNextAmbientDecision();
    
}
// ambient 종료 
void AMHMonsterCharacterBase::StopAmbientBehavior()
{
    GetWorldTimerManager().ClearTimer(AmbientBehaviorTimer);

    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->StopMovement();
    }
}
// 다음 결정 예약 
void AMHMonsterCharacterBase::ScheduleNextAmbientDecision()
{
    if (bInCombat || bHasRoared)
    {
        return;
    }

    const float NextDelay = FMath::FRandRange(AmbientDecisionIntervalMin, AmbientDecisionIntervalMax);

    GetWorldTimerManager().SetTimer(
        AmbientBehaviorTimer,
        this,
        &AMHMonsterCharacterBase::DecideNextAmbientAction,
        NextDelay,
        false
    );
}

// 멈출지 걸을지 결정 
void AMHMonsterCharacterBase::DecideNextAmbientAction()
{
    if (bInCombat || bHasRoared || CombatTarget)
    {
        return;
    }

    /*if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->StopMovement();
    }*/

    SetMonsterMoveSpeed(AmbientWalkSpeed);

    const float RandValue = FMath::FRand();

    if (RandValue < AmbientIdleChance)
    {
        if (AAIController* AICon = Cast<AAIController>(GetController()))
        {
            AICon->StopMovement();
        }
        
        
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("[Ambient] Look Around"));
        ScheduleNextAmbientDecision();
        return;
    }

    MoveToRandomAmbientLocation();
    //ScheduleNextAmbientDecision();
}

// 랜덤 이동 
void AMHMonsterCharacterBase::MoveToRandomAmbientLocation()
{
    if (bInCombat || bHasRoared)
    {
        return;
    }

    AAIController* AICon = Cast<AAIController>(GetController());
    if (!AICon)
    {
        return;
    }

    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys)
    {
        return;
    }

    const FVector Origin = GetActorLocation();
    FNavLocation RandomLocation;
    bool bFoundValidLocation = false;

    for (int32 TryIndex = 0; TryIndex < AmbientMoveLocationTryCount; ++TryIndex)
    {
        const bool bFound = NavSys->GetRandomReachablePointInRadius(
            Origin,
            AmbientMoveRadius,
            RandomLocation
        );

        if (!bFound)
        {
            continue;
        }

        const float MoveDist2D = FVector::Dist2D(Origin, RandomLocation.Location);
        if (MoveDist2D >= AmbientMinMoveDistance)
        {
            bFoundValidLocation = true;
            break;
        }
    }

    if (!bFoundValidLocation)
    {
        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("[Ambient] Valid random location not found (MinDist=%.1f Radius=%.1f)"),
            AmbientMinMoveDistance, AmbientMoveRadius);
        return;
    }

    AICon->MoveToLocation(RandomLocation.Location, 50.f);
    // 타이머 추가 ?
    const float MoveDist = FVector::Dist2D(Origin, RandomLocation.Location);
    const float Speed = FMath::Max(1.f, AmbientWalkSpeed);
    const float ExpectedMoveTime = MoveDist / Speed;

    GetWorldTimerManager().SetTimer(
        AmbientBehaviorTimer,
        this,
        &AMHMonsterCharacterBase::DecideNextAmbientAction,
        ExpectedMoveTime + 0.5f,
        false
    );
    
    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("[Ambient] Move To Random Location | Dist=%.1f"),
        FVector::Dist2D(Origin, RandomLocation.Location));

}

// 이동속도 함수 
void AMHMonsterCharacterBase::SetMonsterMoveSpeed(float NewSpeed)
{
    if (GetCharacterMovement())
    {
        GetCharacterMovement()->MaxWalkSpeed = NewSpeed;
    }
}



#pragma endregion

#pragma region Cooldown
bool AMHMonsterCharacterBase::IsMonsterAbilityOnCooldown(FGameplayTag AbilityTag) const
{
    if (!AbilityTag.IsValid() || !GetWorld())
    {
        return false;
    }

    const float* EndTime = AbilityCooldownEndTimes.Find(AbilityTag);
    if (!EndTime)
    {
        return false;
    }

    return GetWorld()->GetTimeSeconds() < *EndTime;
}

float AMHMonsterCharacterBase::GetMonsterAbilityCooldownRemaining(FGameplayTag AbilityTag) const
{
    if (!AbilityTag.IsValid() || !GetWorld())
    {
        return 0.f;
    }

    const float* EndTime = AbilityCooldownEndTimes.Find(AbilityTag);
    if (!EndTime)
    {
        return 0.f;
    }

    return FMath::Max(0.f, *EndTime - GetWorld()->GetTimeSeconds());
}

float AMHMonsterCharacterBase::FindMonsterAbilityCooldownSeconds(FGameplayTag AbilityTag) const
{
    /*const UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
    if (!MonsterDataAsset || !AbilityTag.IsValid())
    {
        return 0.f;
    }

    for (const FMonsterAbilityEntry& Entry : MonsterDataAsset->AbilityEntries)
    {
        if (Entry.AbilityTag == AbilityTag)
        {
            return Entry.CooldownSeconds;
        }
    }

    for (const FPhaseAbilitySet& PhaseEntry : MonsterDataAsset->PhaseSet)
    {
        for (const FMonsterAbilityEntry& Entry : PhaseEntry.AbilityEntries)
        {
            if (Entry.AbilityTag == AbilityTag)
            {
                return Entry.CooldownSeconds;
            }
        }
    }*/
    FMonsterAbilityEntry Entry;
    
    if (FindMonsterAbilityEntryByTag(AbilityTag, Entry))
    {
        return Entry.CooldownSeconds;
    }
    
    return 0.f;
}

void AMHMonsterCharacterBase::StartMonsterAbilityCooldown(FGameplayTag AbilityTag)
{
    if (!AbilityTag.IsValid() || !GetWorld())
    {
        return;
    }

    const float CooldownSeconds = FindMonsterAbilityCooldownSeconds(AbilityTag);
    if (CooldownSeconds <= 0.f)
    {
        return;
    }

    AbilityCooldownEndTimes.FindOrAdd(AbilityTag) = GetWorld()->GetTimeSeconds() + CooldownSeconds;

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("StartMonsterAbilityCooldown | Tag=%s Cooldown=%.2f"),
        *AbilityTag.ToString(),
        CooldownSeconds);
    
    
}

bool AMHMonsterCharacterBase::FindMonsterAbilityEntryByTag(FGameplayTag AbilityTag,
    FMonsterAbilityEntry& OutEntry) const
{
    OutEntry = FMonsterAbilityEntry();

    const UMHMonsterDataAsset* MonsterDataAsset = Cast<UMHMonsterDataAsset>(GASAsset);
    if (!MonsterDataAsset)
    {
        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("FindMonsterAbilityEntryByTag | MonsterDataAsset null | GASAsset=%s"),
            *GetNameSafe(GASAsset));
        return false;
    }
    
    UE_LOG(MHMonsterCharacterBase, Warning,
    TEXT("FindMonsterAbilityEntryByTag | GASAsset=%s"),
    *GetNameSafe(GASAsset));
    
    

    if (!AbilityTag.IsValid())
    {
        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("FindMonsterAbilityEntryByTag | AbilityTag invalid"));
        return false;
    }

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("FindMonsterAbilityEntryByTag | Search Tag=%s"),
        *AbilityTag.ToString());

    for (const FMonsterAbilityEntry& Entry : MonsterDataAsset->AbilityEntries)
    {
        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("  Check AbilityEntries Tag=%s Cooldown=%.2f"),
            *Entry.AbilityTag.ToString(),
            Entry.CooldownSeconds);

        if (Entry.AbilityTag == AbilityTag)
        {
            OutEntry = Entry;

            UE_LOG(MHMonsterCharacterBase, Warning,
                TEXT("FindMonsterAbilityEntryByTag | Found in AbilityEntries | Tag=%s Cooldown=%.2f"),
                *OutEntry.AbilityTag.ToString(),
                OutEntry.CooldownSeconds);

            return true;
        }
    }

    for (const FPhaseAbilitySet& PhaseEntry : MonsterDataAsset->PhaseSet)
    {
        for (const FMonsterAbilityEntry& Entry : PhaseEntry.AbilityEntries)
        {
            UE_LOG(MHMonsterCharacterBase, Warning,
                TEXT("  Check PhaseSet Phase=%s Tag=%s Cooldown=%.2f"),
                *PhaseEntry.PhaseTag.ToString(),
                *Entry.AbilityTag.ToString(),
                Entry.CooldownSeconds);

            if (Entry.AbilityTag == AbilityTag)
            {
                OutEntry = Entry;

                UE_LOG(MHMonsterCharacterBase, Warning,
                    TEXT("FindMonsterAbilityEntryByTag | Found in PhaseSet | Tag=%s Cooldown=%.2f"),
                    *OutEntry.AbilityTag.ToString(),
                    OutEntry.CooldownSeconds);

                return true;
            }
        }
    }

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("FindMonsterAbilityEntryByTag | Not Found | Tag=%s"),
        *AbilityTag.ToString());

    return false;
}


#pragma endregion

// 디버그용 함수 TODO: 추후 삭제 
void AMHMonsterCharacterBase::DrawSightConeDebug(const FVector& SocketLocation, const FRotator& SocketRotation,
    const FVector& TargetLocation, bool bCanSee) const
{
    if (!GetWorld())
    {
        return;
    }

    const FColor MainColor   = bCanSee ? FColor::Green : FColor::Red;
    const FColor YawColor    = FColor::Yellow;
    const FColor PitchColor  = FColor::Cyan;
    const FColor TargetColor = FColor::Magenta;

    const FVector Forward = SocketRotation.Vector();
    const FVector Right   = FRotationMatrix(SocketRotation).GetUnitAxis(EAxis::Y);
    const FVector Up      = FRotationMatrix(SocketRotation).GetUnitAxis(EAxis::Z);

    // 소켓 위치
    DrawDebugSphere(GetWorld(), SocketLocation, 8.f, 8, MainColor, false, 0.12f, 0, 1.2f);

    // 정면 방향
    DrawDebugLine(
        GetWorld(),
        SocketLocation,
        SocketLocation + Forward * SightDetectRange,
        MainColor,
        false,
        0.12f,
        0,
        2.0f
    );

    // 플레이어 방향
    DrawDebugLine(
        GetWorld(),
        SocketLocation,
        TargetLocation,
        TargetColor,
        false,
        0.12f,
        0,
        1.5f
    );

    DrawDebugSphere(GetWorld(), TargetLocation, 10.f, 8, TargetColor, false, 0.12f, 0, 1.0f);

    // 좌우 부채꼴
    const int32 YawSteps = 16;
    for (int32 i = 0; i <= YawSteps; ++i)
    {
        const float Alpha = (float)i / (float)YawSteps;
        const float YawAngle = FMath::Lerp(-SightHorizontalHalfAngleDeg, SightHorizontalHalfAngleDeg, Alpha);

        const FVector Dir = FRotator(0.f, YawAngle, 0.f).RotateVector(Forward);

        DrawDebugLine(
            GetWorld(),
            SocketLocation,
            SocketLocation + Dir * SightDetectRange,
            YawColor,
            false,
            0.12f,
            0,
            0.8f
        );
    }

    // 위아래 경계선
    const FVector PitchUpDir =
        FRotationMatrix(SocketRotation + FRotator(SightVerticalHalfAngleDeg, 0.f, 0.f)).GetUnitAxis(EAxis::X);

    const FVector PitchDownDir =
        FRotationMatrix(SocketRotation + FRotator(-SightVerticalHalfAngleDeg, 0.f, 0.f)).GetUnitAxis(EAxis::X);

    DrawDebugLine(
        GetWorld(),
        SocketLocation,
        SocketLocation + PitchUpDir * SightDetectRange,
        PitchColor,
        false,
        0.12f,
        0,
        1.2f
    );

    DrawDebugLine(
        GetWorld(),
        SocketLocation,
        SocketLocation + PitchDownDir * SightDetectRange,
        PitchColor,
        false,
        0.12f,
        0,
        1.2f
    );

    // 인식 거리 구체
    DrawDebugSphere(
        GetWorld(),
        SocketLocation,
        SightDetectRange,
        24,
        FColor::Silver,
        false,
        0.12f,
        0,
        0.8f
    );
    
}

void AMHMonsterCharacterBase::SetCombatTarget(AActor* NewTarget)
{
    CombatTarget = NewTarget;

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("SetCombatTarget | NewTarget=%s"),
        *GetNameSafe(NewTarget));

    if (AMHMonsterAIController* MonsterAI = GetMonsterAIController())
    {
        MonsterAI->SetCombatTarget(NewTarget);
    }
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

        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("StartSightDetection | timer started"));
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
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("CheckSightDetection | player detected by sight"));
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
        // UE_LOG(MHMonsterCharacterBase, Warning, TEXT("CanSeeTargetFromHead | missing socket: %s"), *HeadLookSocketName.ToString());
        return false;
    }

    const FVector SocketLocation = MeshComp->GetSocketLocation(HeadLookSocketName);
    const FRotator SocketRotation = MeshComp->GetSocketRotation(HeadLookSocketName);
    const FVector SocketForward = SocketRotation.Vector();

    FVector TargetLocation = Target->GetActorLocation();
    // 디버그용 
    /*DrawDebugLine(
            GetWorld(),
            SocketLocation,
            SocketLocation + SocketForward * 200.f,
            FColor::Green,
            false,
            0.1f,
            0,
            2.f
                );*/
    DrawSightConeDebug(SocketLocation, SocketRotation, TargetLocation, true);
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
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("Sight Fail | target behind head socket"));
        return false;
    }

    const float YawDeg =
        FMath::RadiansToDegrees(FMath::Atan2(LocalDir.Y, LocalDir.X));

    const float PitchDeg =
        FMath::RadiansToDegrees(FMath::Atan2(LocalDir.Z, LocalDir.X));

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("Sight Debug | Dist=%.2f Yaw=%.2f Pitch=%.2f | LimitYaw=%.2f LimitPitch=%.2f"),
        Distance, YawDeg, PitchDeg, SightHorizontalHalfAngleDeg, SightVerticalHalfAngleDeg);

    if (FMath::Abs(YawDeg) > SightHorizontalHalfAngleDeg)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("Sight Fail | yaw out of range"));
        return false;
    }

    if (FMath::Abs(PitchDeg) > SightVerticalHalfAngleDeg)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("Sight Fail | pitch out of range"));
        return false;
    }

    if (bSightRequireLineOfSight)
    {
        const FVector TraceStart = SocketLocation + SocketForward * 20.f;
        const FVector TraceEnd = TargetLocation;

        FHitResult HitResult;
        FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterSightLOS), false, this);
        Params.AddIgnoredActor(this);
        Params.AddIgnoredActor(Target);

        FCollisionObjectQueryParams ObjParams;
        ObjParams.AddObjectTypesToQuery(ECC_WorldStatic);
        ObjParams.AddObjectTypesToQuery(ECC_WorldDynamic);

        const bool bBlocked = GetWorld()->LineTraceSingleByObjectType(
            HitResult,
            TraceStart,
            TraceEnd,
            ObjParams,
            Params
        );

        if (bBlocked)
        {
            UE_LOG(MHMonsterCharacterBase, Warning,
                TEXT("Sight Fail | LOS blocked by %s"),
                *GetNameSafe(HitResult.GetActor()));
            return false;
        }

        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("Sight LOS Success | no obstacle"));
    }

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("Sight Success"));
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

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("HandleSightDetected | Alert -> StartRoar"));

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

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("HandleDamagedFromUnaware | skip roar -> EnterCombatPhase"));

    EnterCombatPhase();
}

void AMHMonsterCharacterBase::StartRoar()
{
    
    // 앰비언트 종료 
    StopAmbientBehavior();

    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->StopMovement();
    }
    if (!AbilitySystemComponent)
    {
        return;
    }

    AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::State_Monster_Roaring);

    if (AMHMonsterAIController* MonsterAI = GetMonsterAIController())
    {
        MonsterAI->SetIsRoaring(true);
    }
    
    if (!RoarMontage)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("StartRoar | RoarMontage null -> fallback combat"));
        OnRoarFinished();
        return;
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("StartRoar | MeshComp null -> fallback combat"));
        OnRoarFinished();
        return;
    }

    UAnimInstance* Anim = MeshComp->GetAnimInstance();
    if (!Anim)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("StartRoar | AnimInstance null -> fallback combat"));
        OnRoarFinished();
        return;
    }

    if (Anim->Montage_IsPlaying(RoarMontage))
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("StartRoar | already playing"));
        return;
    }

    const float PlayedLen = Anim->Montage_Play(RoarMontage, 1.0f);
    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("StartRoar | Montage_Play = %f"), PlayedLen);

    if (PlayedLen > 0.f)
    {
        bHasRoared = true;
        StopSightDetection();
    }
    else
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("StartRoar | play failed -> fallback combat"));
        OnRoarFinished();
    }
}

void AMHMonsterCharacterBase::OnRoarFinished()
{
    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("OnRoarFinished | roar end -> combat phase"));
    
    //BT 용 BOOL 값
    if (AMHMonsterAIController* MonsterAI = GetMonsterAIController())
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("OnRoarFinished | SetIsRoaring(false)"));
        
        MonsterAI->SetIsRoaring(false);
    }
    else
    {
        // todo 추후 제거 로그 
        UE_LOG(MHMonsterCharacterBase, Error, TEXT("OnRoarFinished | MonsterAI is null"));
        
    }
    
    
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Roaring);
    }

    EnterCombatPhase();
}

void AMHMonsterCharacterBase::EnterCombatPhase()
{
    
    SetMonsterMoveSpeed(CombatWalkSpeed);
    if (bInCombat)
    {
        return;
    }
    //BT 용 BOOL  값
    if (AMHMonsterAIController* MonsterAI = GetMonsterAIController())
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("EnterCombatPhase | SetInCombat(true)"));
        MonsterAI->SetInCombat(true);
    }
    else
    {
        UE_LOG(MHMonsterCharacterBase, Error, TEXT("EnterCombatPhase | MonsterAI is null"));
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

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("EnterCombatPhase | combat started"));
}

void AMHMonsterCharacterBase::EnterCombat()
{
    
}

void AMHMonsterCharacterBase::ExitCombat()
{
    bInCombat = false;
    CombatTarget = nullptr;
    
    // 종룔시 BT 값 수정 
    if (AMHMonsterAIController* MonsterAI = GetMonsterAIController())
    {
        MonsterAI->SetInCombat(false);
        MonsterAI->SetIsRoaring(false);
        MonsterAI->SetCombatTarget(nullptr);
    }
    
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Alert);
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Roaring);
        AbilitySystemComponent->RemoveLooseGameplayTag(MHGameplayTags::State_Monster_Combat);
        AbilitySystemComponent->AddLooseGameplayTag(MHGameplayTags::State_Monster_Unaware);
    }

    StartSightDetection();

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ExitCombat | return to unaware"));
}

void AMHMonsterCharacterBase::NotifyDamagedFrom(AActor* InstigatorActor)
{
    if (!InstigatorActor)
    {
        return;
    }

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("NotifyDamagedFrom | damaged by %s"), *GetNameSafe(InstigatorActor));

    if (IsUnaware())
    {
        HandleDamagedFromUnaware(InstigatorActor);
    }
}

void AMHMonsterCharacterBase::InitMonsterGAS()
{
    if (bMonsterGASInitialized)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("InitMonsterGAS | already initialized"));
        return;
    }

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("InitMonsterGAS | start"));

    if (!AbilitySystemComponent)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("InitMonsterGAS | AbilitySystemComponent null"));
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
        // UE_LOG(MHMonsterCharacterBase, Warning, TEXT("[MonsterGAS] %s HP=%f/%f Poise=%f/%f Atk=%f Def=%f"),
        //     *GetName(),
        //     MonsterAttributes->GetHealth(), MonsterAttributes->GetMaxHealth(),
        //     MonsterAttributes->GetPoise(), MonsterAttributes->GetMaxPoise(),
        //     MonsterAttributes->GetAttackPower(), MonsterAttributes->GetDefense()
        // );
    }
    else
    {
        UE_LOG(MHMonsterCharacterBase, Error, TEXT("[MonsterGAS] %s MonsterAttributes is NULL"), *GetName());
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
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("GrantStartupAbilities | !MonsterDataAsset"));
        return;
    }

    // 1) 기존 StartupAbilities 부여
    for (const TSubclassOf<UGameplayAbility>& AbilityClass : MonsterDataAsset->StartupAbilities)
    {
        if (!AbilityClass)
        {
            UE_LOG(MHMonsterCharacterBase, Warning, TEXT("GrantStartupAbilities | AbilityClass = NULL"));
            continue;
        }

        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("GrantStartupAbilities | Give StartupAbility = %s"),
            *AbilityClass->GetName());

        AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
    }

    // 2) AbilityEntries 기반 Ability Spec 부여
    auto GiveEntryAbility = [this](const FMonsterAbilityEntry& Entry)
    {
        if (!AbilitySystemComponent)
        {
            return;
        }

        if (!Entry.AbilityClass)
        {
            UE_LOG(MHMonsterCharacterBase, Warning,
                TEXT("GrantStartupAbilities | Entry AbilityClass null | Tag=%s"),
                *Entry.AbilityTag.ToString());
            return;
        }

        if (!Entry.AbilityTag.IsValid())
        {
            UE_LOG(MHMonsterCharacterBase, Warning,
                TEXT("GrantStartupAbilities | Entry AbilityTag invalid | Class=%s"),
                *GetNameSafe(Entry.AbilityClass));
            return;
        }

        FGameplayAbilitySpec Spec(Entry.AbilityClass, 1, INDEX_NONE, this);
        Spec.DynamicAbilityTags.AddTag(Entry.AbilityTag);

        UE_LOG(MHMonsterCharacterBase, Warning,
            TEXT("GrantStartupAbilities | Give EntryAbility = %s | DynamicTag=%s"),
            *GetNameSafe(Entry.AbilityClass),
            *Entry.AbilityTag.ToString());

        AbilitySystemComponent->GiveAbility(Spec);
    };

    for (const FMonsterAbilityEntry& Entry : MonsterDataAsset->AbilityEntries)
    {
        GiveEntryAbility(Entry);
    }

    for (const FPhaseAbilitySet& PhaseEntry : MonsterDataAsset->PhaseSet)
    {
        for (const FMonsterAbilityEntry& Entry : PhaseEntry.AbilityEntries)
        {
            GiveEntryAbility(Entry);
        }
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
            UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ApplyStartupEffects | applied %s"), *GetNameSafe(EffectClass));
        }
    }
}
