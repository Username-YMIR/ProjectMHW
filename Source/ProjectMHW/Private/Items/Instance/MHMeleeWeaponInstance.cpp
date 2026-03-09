// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHMeleeWeaponInstance.h"

#include "Components/BoxComponent.h"
#include "Interfaces/MHDamageableInterface.h"

DEFINE_LOG_CATEGORY(LogMeleeWeaponInstance);

// Sets default values
AMHMeleeWeaponInstance::AMHMeleeWeaponInstance()
{

	//WeaponCollisionBox
	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(WeaponMesh); // 손승우 수정
	WeaponCollisionBox->SetBoxExtent(FVector(20.0f));
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	//Overlap Connect
	WeaponCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::OnCollisionBoxBeginOverlap);
	WeaponCollisionBox->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::OnCollisionBoxEndOverlap);
}

void AMHMeleeWeaponInstance::OnCollisionBoxBeginOverlap(
		UPrimitiveComponent*	OverlappedComponent, 
		AActor*					OtherActor,
		UPrimitiveComponent*	OtherComp, 
		int32					OtherBodyIndex, 
		bool					bFromSweep, 
		const FHitResult&		SweepResult)
{	
	UE_LOG(LogMeleeWeaponInstance, Log, TEXT("[Weapon] Overlap Begin - Self: %s, Other: %s"),
		*GetNameSafe(this),
		*GetNameSafe(OtherActor));

	// 검사 단계
	if (!OtherActor)
	{
		UE_LOG(LogMeleeWeaponInstance, Warning, TEXT("[Weapon] OtherActor is nullptr"));
		return;
	}

	if (OtherActor == this)
	{
		UE_LOG(LogMeleeWeaponInstance, Warning, TEXT("[Weapon] Ignore self overlap"));
		return;
	}

	if (OtherActor == CachedOwnerActor.Get())
	{
		UE_LOG(LogMeleeWeaponInstance, Warning, TEXT("[Weapon] Ignore owner overlap"));
		return;
	}

	if (!OtherActor->GetClass()->ImplementsInterface(UMHDamageableInterface::StaticClass()))
	{
		UE_LOG(LogMeleeWeaponInstance, Warning, TEXT("[Weapon] %s does not implement DamageableInterface"), *GetNameSafe(OtherActor));
		return;
	}
	
	// 이번 콤보 액터 리스트 체크
	bool HasAlreadyHitActor = OverlappedActorsThisCombo.Contains(OtherActor);
	if (HasAlreadyHitActor)
	{
		UE_LOG(LogMeleeWeaponInstance, Log, TEXT("[Weapon] Already hit in this combo: %s"), *GetNameSafe(OtherActor));
		return;
	}

	// 이번 콤보 오버랩 액터 리스트에 추가
	OverlappedActorsThisCombo.Add(OtherActor);

	UE_LOG(LogTemp, Log, TEXT("[Weapon] Register hit actor: %s"), *GetNameSafe(OtherActor));
	
	// 전달용 대미지 컨텍스트 생성
	const FMHDamageContext DamageContext = BuildDamageContext(OtherActor, SweepResult);

	UE_LOG(LogTemp, Log, TEXT("[Weapon] Send DamageContext -> Target: %s, Damage: %.1f, ComboIndex: %d, Bone: %s"),
		*GetNameSafe(OtherActor),
		DamageContext.BaseDamage,
		DamageContext.ComboIndex,
		*DamageContext.HitBoneName.ToString());

	// 대미지 전달
	IMHDamageableInterface::Execute_ApplyDamageContext(OtherActor, DamageContext);
}

void AMHMeleeWeaponInstance::OnCollisionBoxEndOverlap(
		UPrimitiveComponent*	OverlappedComponent, 
		AActor*					OtherActor,
		UPrimitiveComponent*	OtherComp, 
		int32					OtherBodyIndex)
{

}

FMHDamageContext AMHMeleeWeaponInstance::BuildDamageContext(AActor* TargetActor, const FHitResult& HitResult) const
{
	FMHDamageContext Context;
	Context.SourceActor = CachedOwnerActor;
	Context.TargetActor = TargetActor;
	Context.CauserActor = const_cast<AMHMeleeWeaponInstance*>(this);
	Context.BaseDamage = TestBaseDamage;
	Context.ComboIndex = CurrentComboIndex;
	Context.HitBoneName = HitResult.BoneName;
	Context.HitLocation = HitResult.ImpactPoint;

	return Context;
}