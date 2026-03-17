#include "Animation/mh_player_anim_instance_base.h"

#include "Character/Player/MHPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayTags/MHCombatStateGameplayTags.h"

DEFINE_LOG_CATEGORY(LogMHPlayerAnimInstanceBase);

void UMHPlayerAnimInstanceBase::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    CachedPlayerCharacter = Cast<AMHPlayerCharacter>(TryGetPawnOwner());
}

void UMHPlayerAnimInstanceBase::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    AMHPlayerCharacter* PlayerCharacter = GetMHPlayerCharacter();
    if (!PlayerCharacter)
    {
        GroundSpeed = 0.0f;
        bHasAcceleration = false;
        bIsInAir = false;
        CurrentEquippedWeaponTag = FGameplayTag();
        CurrentWeaponTypeTag = FGameplayTag();
        CurrentWeaponSheathStateTag = FGameplayTag();
        CurrentActionTag = FGameplayTag();
        CurrentMoveTag = FGameplayTag();
        bUseArmedLocomotion = false;
        return;
    }

    // XY 평면 이동 속도만 추출하여 Locomotion BlendSpace 입력값으로 사용한다.
    const FVector HorizontalVelocity(PlayerCharacter->GetVelocity().X, PlayerCharacter->GetVelocity().Y, 0.0f);
    GroundSpeed = HorizontalVelocity.Size();

    if (const UCharacterMovementComponent* MoveComp = PlayerCharacter->GetCharacterMovement())
    {
        bHasAcceleration = MoveComp->GetCurrentAcceleration().SizeSquared2D() > KINDA_SMALL_NUMBER;
        bIsInAir = MoveComp->IsFalling();
    }
    else
    {
        bHasAcceleration = false;
        bIsInAir = false;
    }

    CurrentWeaponTypeTag = PlayerCharacter->GetCurrentWeaponTypeGameplayTag();
    CurrentWeaponSheathStateTag = PlayerCharacter->GetCurrentWeaponSheathGameplayTag();
    CurrentActionTag = PlayerCharacter->GetCurrentCombatStateGameplayTag();
    CurrentEquippedWeaponTag = CurrentWeaponTypeTag;

    // 발도 상태에서만 무기 전용 Locomotion Layer를 사용한다.
    bUseArmedLocomotion = CurrentEquippedWeaponTag.IsValid()
        && CurrentWeaponSheathStateTag == MHCombatStateGameplayTags::WeaponSheath_Unsheathed;
}

AMHPlayerCharacter* UMHPlayerAnimInstanceBase::GetMHPlayerCharacter() const
{
    if (CachedPlayerCharacter.IsValid())
    {
        return CachedPlayerCharacter.Get();
    }

    return Cast<AMHPlayerCharacter>(TryGetPawnOwner());
}
