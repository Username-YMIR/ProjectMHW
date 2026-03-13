// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/MHAnimNotifyState_MeleeAttackHitWindow.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Items/Instance/MHItemInstanceBase.h"
#include "Items/Instance/MHMeleeWeaponInstance.h"

DEFINE_LOG_CATEGORY(MHANS_MeleeAttackHitWindow);


FString UMHAnimNotifyState_MeleeAttackHitWindow::GetNotifyName_Implementation() const
{
    return TEXT("MH Melee Attack Hit Window");
}

void UMHAnimNotifyState_MeleeAttackHitWindow::NotifyBegin(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    float TotalDuration,
    const FAnimNotifyEventReference& EventReference)
{
    UE_LOG(MHANS_MeleeAttackHitWindow, Warning, TEXT("NotifyBegin"))
    
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

    AActor* OwnerActor = ResolveOwnerActor(MeshComp);
    if (!OwnerActor)
    {
        UE_LOG(MHANS_MeleeAttackHitWindow, Warning, TEXT("OwnerActor is not Valid"))
        return;
    }

    AMHMeleeWeaponInstance* MeleeWeapon = ResolveMeleeWeapon(OwnerActor);
    if (!MeleeWeapon)
    {
        UE_LOG(MHANS_MeleeAttackHitWindow, Warning, TEXT("MeleeWeapon is not Valid"))

        return;
    }

    // 새로운 타격 판정 구간 시작 시, 이번 공격에서 이미 맞은 액터 목록 초기화
    MeleeWeapon->ClearHitActors();

    // 공격 판정 콜리전 활성화
    MeleeWeapon->SetAttackCollisionEnabled(true);
}

void UMHAnimNotifyState_MeleeAttackHitWindow::NotifyEnd(
    USkeletalMeshComponent* MeshComp,
    UAnimSequenceBase* Animation,
    const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

    AActor* OwnerActor = ResolveOwnerActor(MeshComp);
    if (!OwnerActor)
    {
        return;
    }

    AMHMeleeWeaponInstance* MeleeWeapon = ResolveMeleeWeapon(OwnerActor);
    if (!MeleeWeapon)
    {
        return;
    }

    // 공격 판정 콜리전 비활성화
    MeleeWeapon->ResetMeleeAttack();
}

AActor* UMHAnimNotifyState_MeleeAttackHitWindow::ResolveOwnerActor(USkeletalMeshComponent* MeshComp) const
{
    if (!MeshComp)
    {
        return nullptr;
    }

    return MeshComp->GetOwner();
}

AMHMeleeWeaponInstance* UMHAnimNotifyState_MeleeAttackHitWindow::ResolveMeleeWeapon(AActor* OwnerActor) const
{
    if (!OwnerActor)
    {
        return nullptr;
    }

    const AMHPlayerCharacter* Player = Cast<AMHPlayerCharacter>(OwnerActor);
    if (!Player)
    {
        return nullptr;
    }

    return Cast<AMHMeleeWeaponInstance>(Player->GetEquippedWeapon());
}