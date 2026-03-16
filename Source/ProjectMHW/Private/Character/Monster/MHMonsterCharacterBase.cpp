// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-09


#include "Character/Monster/MHMonsterCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "MHGameplayTags.h"
#include "NavigationSystem.h"
#include "Character/Monster/AI/MHMonsterAIController.h"
#include "Character/Monster/AI/MHMonsterBlackboardKeys.h"
#include "Character/Monster/Attribute/MHMonsterAttributeSet.h"
#include "Combat/Data/MHCombatDataLibrary.h"
#include "Components/CapsuleComponent.h"
#include "DataAsset/MHMonsterDataAsset.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

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
    
    //ambient 시작 
    SetMonsterMoveSpeed(AmbientWalkSpeed);
    StartAmbientBehavior();
    // 시작부터 Roar 체크하는 게 아니라 "시야 감지"만 시작
    StartSightDetection();
}

bool AMHMonsterCharacterBase::TryActivateMonsterAbilityByTag(FGameplayTag AbilityTag)
{
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

    FGameplayTagContainer TagContainer;
    TagContainer.AddTag(AbilityTag);

    const bool bActivated = AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);

    UE_LOG(MHMonsterCharacterBase, Warning,
        TEXT("TryActivateMonsterAbilityByTag | Tag=%s Result=%s"),
        *AbilityTag.ToString(),
        bActivated ? TEXT("Success") : TEXT("Fail"));

    return bActivated;
}

float AMHMonsterCharacterBase::GetDistanceToCombatTarget() const
{
    if (!CombatTarget)
    {
        return TNumericLimits<float>::Max();
    }

    return FVector::Dist(GetActorLocation(), CombatTarget->GetActorLocation());
}

bool AMHMonsterCharacterBase::IsCombatTargetInRange(float Range) const
{
    if (!CombatTarget)
    {
        return false;
    }

    return GetDistanceToCombatTarget() <= Range;
}


FMHHitAcknowledge AMHMonsterCharacterBase::ReceiveDamageSpec_Implementation(
    AActor* SourceActor,
    AActor* SourceWeapon,
    FGameplayTag AttackTag,
    const FGameplayEffectSpecHandle& DamageSpecHandle,
    const FHitResult& HitResult
)
{
    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ReceiveDamageSpec_Implementation"))
    UE_LOG(MHMonsterCharacterBase, Log, TEXT("SourceActor : %s, SourceWeapon : %s, AttackTag : %s")
        , *SourceActor->GetName()
        , *SourceActor->GetName()
        , *AttackTag.GetTagName().ToString())
    
    FMHHitAcknowledge Result;

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

        if (IsDead())
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

	// TODO:
	// - 사망 애니메이션
	// - AI 정지
	// - 드랍 처리
	// - 전투 종료 처리
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

    if (AAIController* AICon = Cast<AAIController>(GetController()))
    {
        AICon->StopMovement();
    }

    SetMonsterMoveSpeed(AmbientWalkSpeed);

    const float RandValue = FMath::FRand();

    if (RandValue < AmbientIdleChance)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("[Ambient] Look Around"));
        ScheduleNextAmbientDecision();
        return;
    }

    MoveToRandomAmbientLocation();
    ScheduleNextAmbientDecision();
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

    FNavLocation RandomLocation;
    const bool bFound = NavSys->GetRandomReachablePointInRadius(
        GetActorLocation(),
        AmbientMoveRadius,
        RandomLocation
    );

    if (!bFound)
    {
        UE_LOG(MHMonsterCharacterBase, Warning, TEXT("[Ambient] Random reachable point not found"));
        return;
    }

    AICon->MoveToLocation(RandomLocation.Location, 20.f);

    UE_LOG(MHMonsterCharacterBase, Warning, TEXT("[Ambient] Move To Random Location"));

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
            UE_LOG(MHMonsterCharacterBase, Warning, TEXT("ApplyStartupEffects | applied %s"), *GetNameSafe(EffectClass));
        }
    }
}
