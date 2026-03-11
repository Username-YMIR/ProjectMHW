// Fill out your copyright notice in the Description page of Project Settings.

#include "Items/Instance/MHMeleeWeaponInstance.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/BoxComponent.h"
#include "Combat/Effects/MHGameplayEffect_Damage.h"
#include "MHGameplayTags.h"

// 이 클래스 전용 로그 카테고리 선언
DEFINE_LOG_CATEGORY(LogMHMeleeWeaponInstance);

AMHMeleeWeaponInstance::AMHMeleeWeaponInstance()
{
	// 액터 틱은 사용하지 않으므로 비활성화
	PrimaryActorTick.bCanEverTick = false;

	// 무기 타격 판정용 박스 콜리전 생성
	HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));

	// 무기 메시를 기준으로 콜리전 부착
	HitBox->SetupAttachment(WeaponMesh);

	// 물리 충돌은 하지 않고, 오버랩 쿼리만 사용
	HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	// 기본 박스 크기 설정
	HitBox->SetBoxExtent(FVector(20.0f));

	// 오버랩 이벤트 발생 활성화
	HitBox->SetGenerateOverlapEvents(true);
	
	// 기본 대미지 GameplayEffect 클래스를 사용하도록 설정
	DamageEffectClass = UMHGameplayEffect_Damage::StaticClass();
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

void AMHMeleeWeaponInstance::ClearDamagedActors()
{
	// 한 번의 공격 판정 동안 중복 타격을 막기 위해 기록한 액터 목록 초기화
	// 보통 콤보 시작 시점, 공격 시작 시점, 혹은 다음 공격 판정 전에 호출
	DamagedActors.Reset();
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
	// 무기 오버랩 발생 로그
	UE_LOG(LogMHMeleeWeaponInstance, Error, TEXT("OnWeaponBeginOverlap()"));

	// 상대 액터가 유효하지 않으면 처리 중단
	if (!IsValid(OtherActor))
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("Overlaped Actor is not Valid"));
		return;
	}
	
	// 무기의 소유자(공격 주체) 가져오기
	AActor* OwnerActor = GetOwner();
	if (!IsValid(OwnerActor))
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("Owner is not Valid"));
		return;
	}

	// 자기 자신과의 충돌은 무시
	if (OtherActor == OwnerActor)
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("Overlaped Actor is Owner"));
		return;
	}

	// 이미 이번 공격 판정에서 타격한 대상이면 중복 대미지 방지
	if (DamagedActors.Contains(OtherActor))
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("Overlaped Actor is already overlaped before"));
		return;
	}
	
	// 공격자와 피격자의 AbilitySystemComponent 조회
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor);

	// 둘 중 하나라도 ASC가 없으면 GAS 기반 대미지 처리를 할 수 없으므로 중단
	if (!ensure(IsValid(SourceASC)) || !ensure(IsValid(TargetASC)))
	{
		return;
	}

	// 적용할 대미지 GameplayEffect 클래스가 없으면 처리 중단
	if (!DamageEffectClass)
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("DamageEffectClass is null."));
		return;
	}

	// EffectContext 생성
	// 누가, 무엇으로 대미지를 발생시켰는지 같은 부가 정보를 담는 컨텍스트
	FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();

	// 이 무기 액터를 SourceObject로 등록
	// 추후 ExecutionCalculation이나 로그 추적 시 출처 식별에 활용 가능
	EffectContext.AddSourceObject(this);

	// 대미지 GameplayEffectSpec 생성
	// 레벨은 현재 1.0f로 고정
	FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(DamageEffectClass, 1.f, EffectContext);

	// 스펙 생성 실패 시 처리 중단
	if (!SpecHandle.IsValid() || !SpecHandle.Data.IsValid())
	{
		UE_LOG(LogMHMeleeWeaponInstance, Warning, TEXT("Failed to create DamageEffect Spec."));
		return;
	}

	// SetByCaller로 각 속성별 대미지 값을 Spec에 주입
	// 실제 계산은 GE / ExecutionCalculation 쪽에서 이 값을 읽어 수행
	SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Physical, BasePhysicalDamage);
	SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Fire, BaseFireDamage);
	SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Water, BaseWaterDamage);
	SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Thunder, BaseThunderDamage);
	SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Ice, BaseIceDamage);
	SpecHandle.Data->SetSetByCallerMagnitude(MHGameplayTags::Data_Damage_Dragon, BaseDragonDamage);

	// 생성한 대미지 스펙을 대상 ASC에 적용
	const FActiveGameplayEffectHandle ActiveHandle =
		SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);

	// 정상 적용되었다면 이번 공격에서 이미 맞은 대상으로 등록
	if (ActiveHandle.WasSuccessfullyApplied())
	{
		DamagedActors.Add(OtherActor);

		// 디버그 로그 출력
		UE_LOG(
			LogMHMeleeWeaponInstance,
			Log,
			TEXT("Damage Applied. Source=%s Target=%s Physical=%.2f Fire=%.2f"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(OtherActor),
			BasePhysicalDamage,
			BaseFireDamage
		);
	}
	else
	{
		// 적용 실패 로그 출력
		UE_LOG(
			LogMHMeleeWeaponInstance,
			Warning,
			TEXT("Failed to apply damage effect. Source=%s Target=%s"),
			*GetNameSafe(GetOwner()),
			*GetNameSafe(OtherActor)
		);
	}
}