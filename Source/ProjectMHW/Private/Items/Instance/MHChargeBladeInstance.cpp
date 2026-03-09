// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHChargeBladeInstance.h"


AMHChargeBladeInstance::AMHChargeBladeInstance()
{
	// 무기 타입 설정
	WeaponType = EMHWeaponType::ChargeBlade; // 손승우 추가

	// 방패 메쉬 컴포넌트 생성
	ShieldMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShieldMesh")); // 손승우 추가
	ShieldMesh->SetupAttachment(RootComponent); // 손승우 추가

	// 방패는 비주얼 전용
	ShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ShieldMesh->SetGenerateOverlapEvents(false);
}
