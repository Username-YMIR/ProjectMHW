// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Instance/MHMeleeWeaponInstance.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Components/BoxComponent.h"
#include "GameplayTags/MHLongSwordGameplayTags.h"
#include "Interfaces/MHDamageSpecReceiverInterface.h"
#include "TimerManager.h"

namespace
{
	constexpr float HelmbreakerDelayedHitInitialDelay = 1.5f;
	constexpr float HelmbreakerDelayedHitInterval = 0.08f;
	constexpr int32 HelmbreakerDelayedHitCount = 5;
}

DEFINE_LOG_CATEGORY(LogMHMeleeWeaponInstance);

AMHMeleeWeaponInstance::AMHMeleeWeaponInstance()
{
	// 틱이 필요 없으므로 기본 비활성화한다.
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 무기 타격용 박스 콜리전을 생성한다.
	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
	HitBox->SetupAttachment(WeaponMesh);
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitBox->SetGenerateOverlapEvents(false);
	HitBox->SetBoxExtent(FVector(20.0f));
	HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AMHMeleeWeaponInstance::BeginPlay()
{
	Super::BeginPlay();

	if (ensure(HitBox))
	{
		HitBox->OnComponentBeginOverlap.AddDynamic(this, &AMHMeleeWeaponInstance::OnWeaponBeginOverlap);
	}
}

void AMHMeleeWeaponInstance::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearHelmbreakerDelayedHitContexts();
	Super::EndPlay(EndPlayReason);
}

void AMHMeleeWeaponInstance::BeginAttackWindow()
{
	ClearHitActors();
	SetAttackCollisionEnabled(true);
	ProcessExistingOverlapsAtAttackWindowBegin();
}

void AMHMeleeWeaponInstance::EndAttackWindow()
{
	SetAttackCollisionEnabled(false);
	ClearHitActors();
}

void AMHMeleeWeaponInstance::ResetMeleeAttack()
{
	EndAttackWindow();
	ClearCurrentAttackData();
	bResolvedConfirmedHitForCurrentAttack = false;
}

void AMHMeleeWeaponInstance::SetAttackCollisionEnabled(bool bEnabled)
{
	if (!HitBox)
	{
		return;
	}

	if (bEnabled)
	{
		HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		HitBox->SetGenerateOverlapEvents(true);
	}
	else
	{
		HitBox->SetGenerateOverlapEvents(false);
		HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AMHMeleeWeaponInstance::ClearHitActors()
{
	// 동일 윈도우 내 중복 타격을 막기 위해 대상 목록을 초기화한다.
	HitActors.Reset();
}

void AMHMeleeWeaponInstance::SetCurrentDamageSpec(const FGameplayEffectSpecHandle& InDamageSpecHandle)
{
	CurrentDamageSpecHandle = InDamageSpecHandle;
	ResetPerAttackRuntimeState();
}

void AMHMeleeWeaponInstance::SetCurrentAttackTag(const FGameplayTag& InAttackTag)
{
	CurrentAttackTag = InAttackTag;
}

void AMHMeleeWeaponInstance::ClearCurrentAttackData()
{
	CurrentDamageSpecHandle = FGameplayEffectSpecHandle();
	CurrentAttackTag = FGameplayTag();
	bResolvedConfirmedHitForCurrentAttack = false;
}

void AMHMeleeWeaponInstance::ResetPerAttackRuntimeState()
{
	// 새 공격 데이터가 들어오면 1회성 히트 보상 상태를 초기화한다.
	bResolvedConfirmedHitForCurrentAttack = false;
	++CurrentAttackSequenceId;

	UE_LOG(
		LogMHMeleeWeaponInstance,
		Verbose,
		TEXT("새 공격 데이터로 교체. AttackSequence=%d AttackTag=%s"),
		CurrentAttackSequenceId,
		*CurrentAttackTag.ToString()
	);
}

void AMHMeleeWeaponInstance::ProcessExistingOverlapsAtAttackWindowBegin()
{
	if (!HitBox)
	{
		return;
	}

	HitBox->UpdateOverlaps();

	TArray<AActor*> OverlappingActors;
	HitBox->GetOverlappingActors(OverlappingActors);

	UE_LOG(
		LogMHMeleeWeaponInstance,
		Verbose,
		TEXT("공격 윈도우 시작 시 기존 겹침 검사. Count=%d AttackTag=%s"),
		OverlappingActors.Num(),
		*CurrentAttackTag.ToString()
	);

	for (AActor* OverlappingActor : OverlappingActors)
	{
		if (!IsValid(OverlappingActor))
		{
			continue;
		}

		UPrimitiveComponent* OverlapComp = Cast<UPrimitiveComponent>(OverlappingActor->GetRootComponent());
		FHitResult EmptySweepResult;
		OnWeaponBeginOverlap(HitBox, OverlappingActor, OverlapComp, INDEX_NONE, false, EmptySweepResult);
	}
}

bool AMHMeleeWeaponInstance::HasValidCurrentDamageSpec() const
{
	return CurrentDamageSpecHandle.IsValid() && CurrentDamageSpecHandle.Data.IsValid();
}

bool AMHMeleeWeaponInstance::TryDeliverDamageSpecToTarget(
	AActor* TargetActor,
	const FHitResult& HitResult,
	FMHHitAcknowledge& OutHitAcknowledge
)
{
	return TryDeliverDamageSpecToTargetWithAttackData(
		TargetActor,
		HitResult,
		CurrentAttackTag,
		CurrentDamageSpecHandle,
		OutHitAcknowledge
	);
}

bool AMHMeleeWeaponInstance::TryDeliverDamageSpecToTargetWithAttackData(
	AActor* TargetActor,
	const FHitResult& HitResult,
	const FGameplayTag& InAttackTag,
	const FGameplayEffectSpecHandle& InDamageSpecHandle,
	FMHHitAcknowledge& OutHitAcknowledge
)
{
	OutHitAcknowledge = FMHHitAcknowledge();

	if (!IsValid(TargetActor))
	{
		return false;
	}

	if (!TargetActor->GetClass()->ImplementsInterface(UMHDamageSpecReceiverInterface::StaticClass()))
	{
		return false;
	}

	if (!InDamageSpecHandle.IsValid() || !InDamageSpecHandle.Data.IsValid() || !InAttackTag.IsValid())
	{
		return false;
	}

	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		return false;
	}

	OutHitAcknowledge = IMHDamageSpecReceiverInterface::Execute_ReceiveDamageSpec(
		TargetActor,
		OwnerActor,
		this,
		InAttackTag,
		InDamageSpecHandle,
		HitResult
	);

	return true;
}

void AMHMeleeWeaponInstance::PlayAcceptedHitCameraShake(const FGameplayTag& InAttackTag)
{
	AMHPlayerCharacter* PlayerOwner = Cast<AMHPlayerCharacter>(GetOwner());
	if (!IsValid(PlayerOwner))
	{
		return;
	}

	PlayerOwner->PlayWeaponHitCameraShake(InAttackTag);
}

FVector AMHMeleeWeaponInstance::ResolveImpactPoint(
	UPrimitiveComponent* OtherComp,
	AActor* OtherActor,
	bool bFromSweep,
	const FHitResult& SweepResult
) const
{
	// 스윕 결과가 유효하면 해당 타격 지점을 그대로 사용한다.
	if (bFromSweep && !SweepResult.ImpactPoint.IsNearlyZero())
	{
		return SweepResult.ImpactPoint;
	}

	// 상대 콜리전의 최근접 지점을 우선 사용한다.
	if (IsValid(OtherComp) && IsValid(HitBox))
	{
		FVector ClosestPoint = FVector::ZeroVector;
		const FVector QueryPoint = HitBox->GetComponentLocation();
		const float Distance = OtherComp->GetClosestPointOnCollision(QueryPoint, ClosestPoint);
		if (Distance >= 0.0f)
		{
			return ClosestPoint;
		}
	}

	if (IsValid(OtherComp))
	{
		return OtherComp->GetComponentLocation();
	}

	if (IsValid(OtherActor))
	{
		return OtherActor->GetActorLocation();
	}

	return FVector::ZeroVector;
}

FHitResult AMHMeleeWeaponInstance::BuildResolvedHitResult(
	UPrimitiveComponent* OtherComp,
	AActor* OtherActor,
	bool bFromSweep,
	const FHitResult& SweepResult
) const
{
	FHitResult ResolvedHitResult = SweepResult;
	const FVector ResolvedImpactPoint = ResolveImpactPoint(OtherComp, OtherActor, bFromSweep, SweepResult);

	ResolvedHitResult.Location = ResolvedImpactPoint;
	ResolvedHitResult.ImpactPoint = ResolvedImpactPoint;
	ResolvedHitResult.Component = OtherComp;

	if (ResolvedHitResult.ImpactNormal.IsNearlyZero() && IsValid(OtherActor) && IsValid(HitBox))
	{
		const FVector Direction = (OtherActor->GetActorLocation() - HitBox->GetComponentLocation()).GetSafeNormal();
		ResolvedHitResult.ImpactNormal = Direction.IsNearlyZero() ? FVector::UpVector : Direction;
	}

	if (ResolvedHitResult.Normal.IsNearlyZero())
	{
		ResolvedHitResult.Normal = ResolvedHitResult.ImpactNormal;
	}

	return ResolvedHitResult;
}

bool AMHMeleeWeaponInstance::ShouldUseHelmbreakerDelayedMultiHit() const
{
	return CurrentAttackTag == MHLongSwordGameplayTags::Move_LS_SpiritHelmbreaker;
}

FMHHelmbreakerDelayedHitContext* AMHMeleeWeaponInstance::FindHelmbreakerDelayedHitContext(AActor* InTargetActor)
{
	if (!IsValid(InTargetActor))
	{
		return nullptr;
	}

	for (FMHHelmbreakerDelayedHitContext& Context : HelmbreakerDelayedHitContexts)
	{
		if (Context.TargetActor.Get() == InTargetActor)
		{
			return &Context;
		}
	}

	return nullptr;
}

bool AMHMeleeWeaponInstance::ScheduleHelmbreakerDelayedMultiHit(AActor* InTargetActor)
{
	if (!ShouldUseHelmbreakerDelayedMultiHit() || !IsValid(InTargetActor))
	{
		return false;
	}

	if (!HasValidCurrentDamageSpec() || !CurrentAttackTag.IsValid())
	{
		return false;
	}

	if (FindHelmbreakerDelayedHitContext(InTargetActor) != nullptr)
	{
		return true;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return false;
	}

	FMHHelmbreakerDelayedHitContext& NewContext = HelmbreakerDelayedHitContexts.AddDefaulted_GetRef();
	NewContext.TargetActor = InTargetActor;
	NewContext.DamageSpecHandle = CurrentDamageSpecHandle;
	NewContext.AttackTag = CurrentAttackTag;
	NewContext.RemainingHitCount = HelmbreakerDelayedHitCount;
	NewContext.AttackSequenceId = CurrentAttackSequenceId;

	FTimerDelegate DelayedHitDelegate;
	DelayedHitDelegate.BindUObject(
		this,
		&AMHMeleeWeaponInstance::ExecuteHelmbreakerDelayedMultiHit,
		TWeakObjectPtr<AActor>(InTargetActor)
	);

	World->GetTimerManager().SetTimer(
		NewContext.TimerHandle,
		DelayedHitDelegate,
		HelmbreakerDelayedHitInterval,
		true,
		HelmbreakerDelayedHitInitialDelay
	);

	UE_LOG(
		LogMHMeleeWeaponInstance,
		Log,
		TEXT("투구깨기 지연 다단히트 예약. Target=%s Delay=%.2f Interval=%.2f Count=%d AttackSequence=%d"),
		*GetNameSafe(InTargetActor),
		HelmbreakerDelayedHitInitialDelay,
		HelmbreakerDelayedHitInterval,
		HelmbreakerDelayedHitCount,
		CurrentAttackSequenceId
	);

	return true;
}

void AMHMeleeWeaponInstance::ExecuteHelmbreakerDelayedMultiHit(TWeakObjectPtr<AActor> InTargetActor)
{
	AActor* TargetActor = InTargetActor.Get();
	if (!IsValid(TargetActor))
	{
		// 이미 사라진 대상에 대한 예약만 정리한다.
		RemoveHelmbreakerDelayedHitContext(nullptr);
		return;
	}

	FMHHelmbreakerDelayedHitContext* DelayedHitContext = FindHelmbreakerDelayedHitContext(TargetActor);
	if (DelayedHitContext == nullptr)
	{
		return;
	}

	AMHPlayerCharacter* PlayerOwner = Cast<AMHPlayerCharacter>(GetOwner());
	if (!IsValid(PlayerOwner))
	{
		RemoveHelmbreakerDelayedHitContext(TargetActor);
		return;
	}

	if (!DelayedHitContext->DamageSpecHandle.IsValid()
		|| !DelayedHitContext->DamageSpecHandle.Data.IsValid()
		|| !DelayedHitContext->AttackTag.IsValid())
	{
		RemoveHelmbreakerDelayedHitContext(TargetActor);
		return;
	}

	// 실제 타격이 들어가는 시점에만 샤프니스와 바운스를 처리한다.
	const EMHHitResultType LocalHitResult = PlayerOwner->HandleWeaponAttackHit(TargetActor, this);
	if (LocalHitResult == EMHHitResultType::Bounced)
	{
		PlayerOwner->HandleSharpnessBounce();
		RemoveHelmbreakerDelayedHitContext(TargetActor);
		return;
	}

	UPrimitiveComponent* TargetPrimitive = Cast<UPrimitiveComponent>(TargetActor->GetRootComponent());
	FHitResult EmptySweepResult;
	const FHitResult ResolvedHitResult = BuildResolvedHitResult(
		TargetPrimitive,
		TargetActor,
		false,
		EmptySweepResult
	);

	FMHHitAcknowledge HitAcknowledge;
	if (!TryDeliverDamageSpecToTargetWithAttackData(
		TargetActor,
		ResolvedHitResult,
		DelayedHitContext->AttackTag,
		DelayedHitContext->DamageSpecHandle,
		HitAcknowledge))
	{
		RemoveHelmbreakerDelayedHitContext(TargetActor);
		return;
	}

	ResolveConfirmedHitForAttack(
		PlayerOwner,
		DelayedHitContext->AttackTag,
		DelayedHitContext->AttackSequenceId,
		HitAcknowledge
	);

	UE_LOG(
		LogMHMeleeWeaponInstance,
		Log,
		TEXT("투구깨기 지연 타격 실행. Target=%s RemainingBefore=%d Accepted=%d ResultType=%d"),
		*GetNameSafe(TargetActor),
		DelayedHitContext->RemainingHitCount,
		HitAcknowledge.bAcceptedHit ? 1 : 0,
		static_cast<int32>(HitAcknowledge.ResultType)
	);

	DelayedHitContext->RemainingHitCount--;
	if (DelayedHitContext->RemainingHitCount <= 0)
	{
		RemoveHelmbreakerDelayedHitContext(TargetActor);
	}
}

void AMHMeleeWeaponInstance::RemoveHelmbreakerDelayedHitContext(AActor* InTargetActor)
{
	for (int32 ContextIndex = HelmbreakerDelayedHitContexts.Num() - 1; ContextIndex >= 0; --ContextIndex)
	{
		FMHHelmbreakerDelayedHitContext& Context = HelmbreakerDelayedHitContexts[ContextIndex];
		if (InTargetActor != nullptr)
		{
			if (Context.TargetActor.Get() != InTargetActor)
			{
				continue;
			}
		}
		else if (Context.TargetActor.IsValid())
		{
			continue;
		}

		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(Context.TimerHandle);
		}

		HelmbreakerDelayedHitContexts.RemoveAtSwap(ContextIndex);
	}
}

void AMHMeleeWeaponInstance::ClearHelmbreakerDelayedHitContexts()
{
	if (UWorld* World = GetWorld())
	{
		for (FMHHelmbreakerDelayedHitContext& Context : HelmbreakerDelayedHitContexts)
		{
			World->GetTimerManager().ClearTimer(Context.TimerHandle);
		}
	}

	HelmbreakerDelayedHitContexts.Reset();
}

void AMHMeleeWeaponInstance::ResolveConfirmedHitForAttack(
	AMHPlayerCharacter* PlayerOwner,
	const FGameplayTag& InAttackTag,
	int32 InAttackSequenceId,
	const FMHHitAcknowledge& HitAcknowledge
)
{
	if (!HitAcknowledge.bAcceptedHit || HitAcknowledge.ResultType != EMHHitResultType::NormalHit)
	{
		return;
	}

	if (ResolvedConfirmedHitAttackSequenceIds.Contains(InAttackSequenceId))
	{
		return;
	}

	if (IsValid(PlayerOwner))
	{
		PlayerOwner->Notify_LongSwordAttackHitConfirmed(InAttackTag);
	}

	PlayAcceptedHitCameraShake(InAttackTag);
	ResolvedConfirmedHitAttackSequenceIds.Add(InAttackSequenceId);

	if (InAttackSequenceId == CurrentAttackSequenceId)
	{
		bResolvedConfirmedHitForCurrentAttack = true;
	}
}

void AMHMeleeWeaponInstance::OnWeaponBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	(void)OverlappedComponent;
	(void)OtherBodyIndex;

	if (!IsValid(OtherActor))
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("오버랩 대상 액터가 유효하지 않습니다."));
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("무기 소유자가 유효하지 않습니다."));
		return;
	}

	if (OtherActor == OwnerActor)
	{
		return;
	}

	if (HitActors.Contains(OtherActor))
	{
		return;
	}

	if (!HasValidCurrentDamageSpec())
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("CurrentDamageSpecHandle이 유효하지 않습니다."));
		return;
	}

	if (!CurrentAttackTag.IsValid())
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("CurrentAttackTag가 유효하지 않습니다."));
		return;
	}

	const FHitResult ResolvedHitResult = BuildResolvedHitResult(OtherComp, OtherActor, bFromSweep, SweepResult);

	UE_LOG(
		LogMHMeleeWeaponInstance,
		Log,
		TEXT("오버랩 감지. TargetActor=%s bFromSweep=%d ImpactPoint=(%.2f, %.2f, %.2f) AttackTag=%s"),
		*GetNameSafe(OtherActor),
		bFromSweep ? 1 : 0,
		ResolvedHitResult.ImpactPoint.X,
		ResolvedHitResult.ImpactPoint.Y,
		ResolvedHitResult.ImpactPoint.Z,
		*CurrentAttackTag.ToString()
	);

	AMHPlayerCharacter* PlayerOwner = Cast<AMHPlayerCharacter>(OwnerActor);

	// 투구깨기는 첫 오버랩 시 즉시 데미지를 주지 않고 지연 다단히트만 예약한다.
	if (ShouldUseHelmbreakerDelayedMultiHit()
		&& OtherActor->GetClass()->ImplementsInterface(UMHDamageSpecReceiverInterface::StaticClass())
		&& ScheduleHelmbreakerDelayedMultiHit(OtherActor))
	{
		HitActors.Add(OtherActor);
		return;
	}

	FMHHitAcknowledge HitAcknowledge;

	if (OtherActor->GetClass()->ImplementsInterface(UMHDamageSpecReceiverInterface::StaticClass()) && PlayerOwner)
	{
		const EMHHitResultType LocalHitResult = PlayerOwner->HandleWeaponAttackHit(OtherActor, this);
		if (LocalHitResult == EMHHitResultType::Bounced)
		{
			HitAcknowledge.bAcceptedHit = true;
			HitAcknowledge.bConsumeHitOnce = true;
			HitAcknowledge.bShouldStopAttackWindow = true;
			HitAcknowledge.ResultType = EMHHitResultType::Bounced;

			PlayerOwner->HandleSharpnessBounce();
			HitActors.Add(OtherActor);
			SetAttackCollisionEnabled(false);
			return;
		}
	}

	if (!TryDeliverDamageSpecToTarget(OtherActor, ResolvedHitResult, HitAcknowledge))
	{
		UE_LOG(
			LogMHMeleeWeaponInstance,
			Verbose,
			TEXT("대상이 DamageSpec 수신 인터페이스를 구현하지 않았습니다. Target=%s"),
			*GetNameSafe(OtherActor)
		);
		return;
	}

	ResolveConfirmedHitForAttack(
		PlayerOwner,
		CurrentAttackTag,
		CurrentAttackSequenceId,
		HitAcknowledge
	);

	if (HitAcknowledge.bConsumeHitOnce)
	{
		HitActors.Add(OtherActor);
	}

	if (HitAcknowledge.bShouldStopAttackWindow)
	{
		SetAttackCollisionEnabled(false);
	}

	UE_LOG(
		LogMHMeleeWeaponInstance,
		Log,
		TEXT("DamageSpec 전달 완료. Source=%s Target=%s AttackTag=%s Accepted=%d ConsumeOnce=%d StopWindow=%d"),
		*GetNameSafe(OwnerActor),
		*GetNameSafe(OtherActor),
		*CurrentAttackTag.ToString(),
		HitAcknowledge.bAcceptedHit ? 1 : 0,
		HitAcknowledge.bConsumeHitOnce ? 1 : 0,
		HitAcknowledge.bShouldStopAttackWindow ? 1 : 0
	);
}
