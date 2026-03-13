#include "Character/MHCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Combat/Attributes/MHPlayerAttributeSet.h"

DEFINE_LOG_CATEGORY(LogMHCharacterBase)


AMHCharacterBase::AMHCharacterBase()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
    
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    
}

UAbilitySystemComponent* AMHCharacterBase::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AMHCharacterBase::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogMHCharacterBase, Verbose, TEXT("%s : BeginPlay"), *GetName());
}

void AMHCharacterBase::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    InitializeAbilitySystem();
}

void AMHCharacterBase::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    InitializeAbilitySystem();
}

void AMHCharacterBase::InitializeAbilitySystem()
{
    if (bGASInitialized)
    {
        return;
    }

    if (!AbilitySystemComponent)
    {
        UE_LOG(LogMHCharacterBase, Error, TEXT("%s : AbilitySystemComponent is null"), *GetName());
        return;
    }

    // GAS ActorInfo 초기화
    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    bGASInitialized = true;

    UE_LOG(LogMHCharacterBase, Log, TEXT("%s : GAS Initialized"), *GetName());
    
}


#pragma region DamageSystem_GJ
FMHHitAcknowledge AMHCharacterBase::ReceiveDamageSpec_Implementation(
	AActor* SourceActor,
	AActor* SourceWeapon,
	FGameplayTag AttackTag,
	const FGameplayEffectSpecHandle& DamageSpecHandle,
	const FHitResult& HitResult
)
{
	UE_LOG(LogMHCharacterBase, Warning, TEXT("ReceiveDamageSpec_Implementation"));
	// 1. DamageSpec 유효성 검사
	if (!ValidateDamageSpec(DamageSpecHandle))
	{
		HandleDamageRejected(SourceActor, SourceWeapon, AttackTag, HitResult);
		return BuildRejectedHitAcknowledge();
	}

	// 2. 현재 피격 가능한 상태인지 검사
	if (!CanReceiveDamage(SourceActor, AttackTag, DamageSpecHandle, HitResult))
	{
		HandleDamageRejected(SourceActor, SourceWeapon, AttackTag, HitResult);
		return BuildRejectedHitAcknowledge();
	}

	// 3. 자기 자신에게 DamageSpec 적용
	if (!ApplyIncomingDamageSpec(DamageSpecHandle))
	{
		HandleDamageRejected(SourceActor, SourceWeapon, AttackTag, HitResult);
		return BuildRejectedHitAcknowledge();
	}

	// 4. 적용 성공 후 후처리
	HandleDamageAccepted(SourceActor, SourceWeapon, AttackTag, HitResult);

	// 5. 사망 여부 검사
	if (IsDead())
	{
		HandleDeath();
	}

	return BuildAcceptedHitAcknowledge();
}

bool AMHCharacterBase::ValidateDamageSpec(
	const FGameplayEffectSpecHandle& DamageSpecHandle
) const
{
	return DamageSpecHandle.IsValid() && DamageSpecHandle.Data.IsValid();
}

bool AMHCharacterBase::CanReceiveDamage(
	AActor* SourceActor,
	FGameplayTag AttackTag,
	const FGameplayEffectSpecHandle& DamageSpecHandle,
	const FHitResult& HitResult
) const
{
	// 기본 구현:
	// - 살아있는 상태라면 피격 가능
	// - 필요 시 파생 클래스에서 무적, 가드, 연출 상태 등을 검사하도록 오버라이드
	return !IsDead();
}

UAbilitySystemComponent* AMHCharacterBase::GetCharacterASC() const
{
	return UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(
		const_cast<AMHCharacterBase*>(this));
}

bool AMHCharacterBase::ApplyIncomingDamageSpec(
	const FGameplayEffectSpecHandle& DamageSpecHandle
)
{
	UAbilitySystemComponent* TargetASC = GetCharacterASC();
	if (!IsValid(TargetASC))
	{
		return false;
	}

	if (!DamageSpecHandle.IsValid() || !DamageSpecHandle.Data.IsValid())
	{
		return false;
	}

	const FActiveGameplayEffectHandle ActiveHandle =
		TargetASC->ApplyGameplayEffectSpecToSelf(*DamageSpecHandle.Data.Get());

	return ActiveHandle.WasSuccessfullyApplied();
}

void AMHCharacterBase::HandleDamageAccepted(
	AActor* SourceActor,
	AActor* SourceWeapon,
	FGameplayTag AttackTag,
	const FHitResult& HitResult
)
{
	// 공통 기본 처리
	// 필요 시 파생 클래스에서 오버라이드
	// 예:
	// - 피격 리액션
	// - UI 갱신
	// - AI 반응
	// - 카메라 셰이크
}

void AMHCharacterBase::HandleDamageRejected(
	AActor* SourceActor,
	AActor* SourceWeapon,
	FGameplayTag AttackTag,
	const FHitResult& HitResult
)
{
	// 공통 기본 처리
	// 예:
	// - 무적 피드백
	// - 가드 이펙트
	// - 튕김 이펙트
}

bool AMHCharacterBase::IsDead() const
{
	// TODO:
	// Health AttributeSet과 연동해서 현재 체력이 0 이하인지 검사
	// 예시:
	// return CurrentHealth <= 0.0f;

	return false;
}

void AMHCharacterBase::HandleDeath()
{
	// 공통 사망 진입점
	// 실제 사망 처리 방식은 파생 클래스에서 오버라이드 권장
}

FMHHitAcknowledge AMHCharacterBase::BuildAcceptedHitAcknowledge() const
{
	FMHHitAcknowledge Result;
	Result.bAcceptedHit = true;
	Result.bConsumeHitOnce = true;
	Result.bShouldStopAttackWindow = false;
	Result.ResultType = EMHHitResultType::NormalHit;
	return Result;
}

FMHHitAcknowledge AMHCharacterBase::BuildRejectedHitAcknowledge() const
{
	FMHHitAcknowledge Result;
	Result.bAcceptedHit = false;
	Result.bConsumeHitOnce = false;
	Result.bShouldStopAttackWindow = false;
	Result.ResultType = EMHHitResultType::None;
	return Result;
}

#pragma endregion