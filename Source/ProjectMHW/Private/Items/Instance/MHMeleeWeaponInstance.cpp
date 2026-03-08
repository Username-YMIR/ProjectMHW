// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHMeleeWeaponInstance.h"

#include "Components/BoxComponent.h"


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
	//GetInstigator 이벤트를 발생시킨 포인터 누가 대미지를 입혔는가 T 클래스 반환 (주체대상)
	//무기의 주체(Hero)
	APawn* WeaponOwningPawn = GetInstigator<APawn>();

	checkf(WeaponOwningPawn, TEXT("Forget to Assign an instigator as the owning pawn of the weapon: %s"), *GetName());

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (WeaponOwningPawn != HitPawn)
		{
			OnWeaponHitTarget.ExecuteIfBound(OtherActor);
		}
	}
}

void AMHMeleeWeaponInstance::OnCollisionBoxEndOverlap(
		UPrimitiveComponent*	OverlappedComponent, 
		AActor*					OtherActor,
		UPrimitiveComponent*	OtherComp, 
		int32					OtherBodyIndex)
{
	//GetInstigator 이벤트를 발생시킨 포인터 누가 대미지를 입혔는가 T 클래스 반환 (주체대상)
	APawn* WeaponOwningPawn = GetInstigator<APawn>();

	checkf(WeaponOwningPawn, TEXT("Forget to Assign an instigator as the owning pawn of the weapon: %s"), *GetName());

	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (WeaponOwningPawn != HitPawn)
		{
			OnWeaponPulledFromTarget.ExecuteIfBound(OtherActor);
		}
	}
}

