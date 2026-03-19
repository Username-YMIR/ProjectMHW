// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHWeaponInstance.h"

#include "Components/SceneComponent.h" // 손승우 추가
#include "AbilitySystemComponent.h" // 손승우 추가
#include "Abilities/GameplayAbility.h" // 손승우 추가
#include "Items/Data/MHItemDataBase.h"
#include "Items/Data/MHWeaponItemData.h"
#include "Items/Data/ItemDataRegistry.h"



// Sets default values
AMHWeaponInstance::AMHWeaponInstance()
{
	WeaponRoot = CreateDefaultSubobject<USceneComponent>(TEXT("WeaponRoot")); // 손승우 수정
	RootComponent = WeaponRoot; // 손승우 수정

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(WeaponRoot); // 손승우 수정
	
	// 무기 메시 자체는 판정용이 아니라 표시용이므로 기본 충돌 비활성화_이건주
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetGenerateOverlapEvents(false);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);

	// 물리/시뮬레이션 관련 충돌도 사용하지 않음_이건주
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	
}

void AMHWeaponInstance::BeginPlay()
{
	Super::BeginPlay();
}

void AMHWeaponInstance::ApplyItemData()
{
	Super::ApplyItemData();

	const UMHWeaponItemData* WeaponData = GetWeaponData();
	if (!WeaponData) return;

	if (!WeaponMesh) return;

	if (WeaponData->WeaponMeshData.IsNull())
	{
		WeaponMesh->SetSkeletalMesh(nullptr);
		return;
	}

	USkeletalMesh* WeaponMeshData = WeaponData->WeaponMeshData.LoadSynchronous();
	WeaponMesh->SetSkeletalMesh(WeaponMeshData);
}


const FMHAttackStats& AMHWeaponInstance::GetAttackStats() const
{
	static FMHAttackStats EmptyStat;

	return GetWeaponData()
		? GetWeaponData()->AttackStats
		: EmptyStat;
}

void AMHWeaponInstance::GrantWeaponAbilities(UAbilitySystemComponent* ASC) // 손승우 추가
{
	if (!ASC)
	{
		return;
	}

	if (!PrimaryAttackAbilityClass)
	{
		return;
	}

	if (PrimaryAttackAbilityHandle.IsValid())
	{
		return;
	}

	const FGameplayAbilitySpec Spec(PrimaryAttackAbilityClass, 1, INDEX_NONE, this);
	PrimaryAttackAbilityHandle = ASC->GiveAbility(Spec);
}

void AMHWeaponInstance::ClearWeaponAbilities(UAbilitySystemComponent* ASC) // 손승우 추가
{
	if (!ASC)
	{
		return;
	}

	if (!PrimaryAttackAbilityHandle.IsValid())
	{
		return;
	}

	ASC->ClearAbility(PrimaryAttackAbilityHandle);
	PrimaryAttackAbilityHandle = FGameplayAbilitySpecHandle();
}
