// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Instance/MHMeleeWeaponInstance.h"

#include "Components/BoxComponent.h"
#include "Character/Player/MHPlayerCharacter.h"
#include "Interfaces/MHDamageableInterface.h"
#include "Interfaces/MHDamageSpecReceiverInterface.h"

// 이 클래스 전용 로그 카테고리 선언
DEFINE_LOG_CATEGORY(LogMHMeleeWeaponInstance);

AMHMeleeWeaponInstance::AMHMeleeWeaponInstance()
{
	// 액터 틱은 사용하지 않으므로 비활성화
	PrimaryActorTick.bCanEverTick = false;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 무기 타격 판정용 박스 콜리전 생성
	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));

	// 무기 메시를 기준으로 콜리전 부착
	HitBox->SetupAttachment(WeaponMesh);

	// 기본 상태는 비활성화
	HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HitBox->SetGenerateOverlapEvents(false);

	// 기본 박스 크기 설정
	HitBox->SetBoxExtent(FVector(20.0f));

	// 기본 응답 설정
	HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void AMHMeleeWeaponInstance::BeginPlay()
{
	Super::BeginPlay();

	// HitBox가 정상적으로 생성되었으면 오버랩 시작 이벤트 바인딩
	if (ensure(HitBox))
	{
		HitBox->OnComponentBeginOverlap.AddDynamic(this, &AMHMeleeWeaponInstance::OnWeaponBeginOverlap);
	}
}

void AMHMeleeWeaponInstance::ResetMeleeAttack()
{
	// 콜리전을 끄고 공격 받은 액터 리스트, 현재 공격 데이터 초기화
	SetAttackCollisionEnabled(false);
	ClearHitActors();
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
	// 한 번의 공격 판정 동안 중복 타격을 막기 위해 기록한 액터 목록 초기화
	HitActors.Reset();
}

void AMHMeleeWeaponInstance::SetCurrentDamageSpec(const FGameplayEffectSpecHandle& InDamageSpecHandle)
{
	CurrentDamageSpecHandle = InDamageSpecHandle;
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
	OutHitAcknowledge = FMHHitAcknowledge();

	if (!IsValid(TargetActor))
	{
		return false;
	}

	if (!TargetActor->GetClass()->ImplementsInterface(UMHDamageSpecReceiverInterface::StaticClass()))
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
		CurrentAttackTag,
		CurrentDamageSpecHandle,
		HitResult
	);

	return true;
}

FVector AMHMeleeWeaponInstance::ResolveImpactPoint(
	UPrimitiveComponent* OtherComp,
	AActor* OtherActor,
	bool bFromSweep,
	const FHitResult& SweepResult
) const
{
	// 1. Sweep로 들어온 유효한 ImpactPoint가 있으면 그대로 사용
	if (bFromSweep && !SweepResult.ImpactPoint.IsNearlyZero())
	{
		return SweepResult.ImpactPoint;
	}

	// 2. 상대 콜리전에서 무기 박스 기준 최근접점 계산
	if (IsValid(OtherComp) && IsValid(HitBox))
	{
		FVector ClosestPoint = FVector::ZeroVector;
		const FVector QueryPoint = HitBox->GetComponentLocation();
		const float Distance = OtherComp->GetClosestPointOnCollision(QueryPoint, ClosestPoint);

		// 0 이상이면 유효한 최근접점 반환
		if (Distance >= 0.0f)
		{
			return ClosestPoint;
		}
	}

	// 3. 컴포넌트 중심 fallback
	if (IsValid(OtherComp))
	{
		return OtherComp->GetComponentLocation();
	}

	// 4. 액터 중심 fallback
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

	// Sweep 노멀이 유효하지 않으면 방향 보정
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

void AMHMeleeWeaponInstance::OnWeaponBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// 상대 액터가 유효하지 않으면 처리 중단
	if (!IsValid(OtherActor))
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("Overlapped Actor is not valid."));
		return;
	}

	// 무기의 소유자(공격 주체) 가져오기
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("Owner is not valid."));
		return;
	}

	// 자기 자신과의 충돌은 무시
	if (OtherActor == OwnerActor)
	{
		return;
	}

	// 이미 이번 공격 판정에서 타격한 대상이면 중복 처리 방지
	if (HitActors.Contains(OtherActor))
	{
		return;
	}

	// 현재 공격용 DamageSpec이 없으면 처리 중단
	if (!HasValidCurrentDamageSpec())
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("CurrentDamageSpecHandle is invalid."));
		return;
	}

	// 현재 공격 태그가 없으면 처리 중단
	if (!CurrentAttackTag.IsValid())
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("CurrentAttackTag is invalid."));
		return;
	}

	const FHitResult ResolvedHitResult = BuildResolvedHitResult(OtherComp, OtherActor, bFromSweep, SweepResult);

	UE_LOG(
		LogMHMeleeWeaponInstance,
		Log,
		TEXT("TargetActor=%s bFromSweep=%d ImpactPoint=(%.2f, %.2f, %.2f)"),
		*GetNameSafe(OtherActor),
		bFromSweep ? 1 : 0,
		ResolvedHitResult.ImpactPoint.X,
		ResolvedHitResult.ImpactPoint.Y,
		ResolvedHitResult.ImpactPoint.Z
	);

	FMHHitAcknowledge HitAcknowledge;
	if (!TryDeliverDamageSpecToTarget(OtherActor, ResolvedHitResult, HitAcknowledge))
	{
		UE_LOG(
			LogMHMeleeWeaponInstance,
			Verbose,
			TEXT("Target does not implement damage receiver interface. Target=%s"),
			*GetNameSafe(OtherActor)
		);
		return;
	}

	// 실제 유효 타격이 처음 성립한 순간에만 소유자 자원을 반영한다.
	if (!bResolvedConfirmedHitForCurrentAttack
		&& HitAcknowledge.bAcceptedHit
		&& HitAcknowledge.ResultType == EMHHitResultType::NormalHit)
	{
		if (AMHPlayerCharacter* PlayerOwner = Cast<AMHPlayerCharacter>(OwnerActor))
		{
			PlayerOwner->Notify_LongSwordAttackHitConfirmed(CurrentAttackTag);
		}

		bResolvedConfirmedHitForCurrentAttack = true;
	}

	// 피격자가 이번 판정을 1회 소비 대상으로 인정하면 목록에 등록
	if (HitAcknowledge.bConsumeHitOnce)
	{
		HitActors.Add(OtherActor);
	}

	// 특정 결과에서 공격 윈도우를 조기 종료해야 하면 콜리전 비활성화
	if (HitAcknowledge.bShouldStopAttackWindow)
	{
		SetAttackCollisionEnabled(false);
	}

	UE_LOG(
		LogMHMeleeWeaponInstance,
		Log,
		TEXT("DamageSpec delivered. Source=%s Target=%s AttackTag=%s Accepted=%d ConsumeOnce=%d StopWindow=%d"),
		*GetNameSafe(OwnerActor),
		*GetNameSafe(OtherActor),
		*CurrentAttackTag.ToString(),
		HitAcknowledge.bAcceptedHit ? 1 : 0,
		HitAcknowledge.bConsumeHitOnce ? 1 : 0,
		HitAcknowledge.bShouldStopAttackWindow ? 1 : 0
	);
}