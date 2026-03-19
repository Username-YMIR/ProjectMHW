// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHLongSwordInstance.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Weapons/LongSword/MHLongSwordComboGraph.h" // 손승우 추가
#include "Weapons/LongSword/MHLongSwordComboComponent.h" // 손승우 추가


// Sets default values
AMHLongSwordInstance::AMHLongSwordInstance()
{
	// 무기 타입 설정
	WeaponType = EMHWeaponType::LongSword; // 손승우 추가

	// 검집 메쉬 컴포넌트 생성
	SayaMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SayaMesh"));
	SayaMesh->SetupAttachment(RootComponent); // 손승우 추가

	// 검집은 비주얼 전용
	SayaMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SayaMesh->SetGenerateOverlapEvents(false); // 손승우 추가

	ComboComponent = CreateDefaultSubobject<UMHLongSwordComboComponent>(TEXT("ComboComponent")); // 손승우 수정
}

void AMHLongSwordInstance::ApplyItemData()
{
	Super::ApplyItemData();

	const UMHLongSwordItemData* LongSwordData = GetLongSwordData();
	if (!LongSwordData) return;

	if (!SayaMesh) return;
	
	if (LongSwordData->SayaMeshData.IsNull())
	{
		SayaMesh->SetSkeletalMesh(nullptr);
		return;
	}
	

	USkeletalMesh* SayaMeshData = LongSwordData->SayaMeshData.LoadSynchronous();
	SayaMesh->SetSkeletalMesh(SayaMeshData);
}

void AMHLongSwordInstance::BeginPlay() // 손승우 추가
{
	Super::BeginPlay();

	if (ComboComponent)
	{
		UMHLongSwordComboGraph* Graph = GetComboGraph();
		ComboComponent->SetComboGraph(Graph);
	}
}

UMHLongSwordComboGraph* AMHLongSwordInstance::GetComboGraph() const // 손승우 추가
{
	return ComboGraphAsset.IsNull() 
			? nullptr 
			: ComboGraphAsset.LoadSynchronous(); // 손승우 추가
}
