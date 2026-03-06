#include "Character/Player/MHPlayerCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Components/Input/MHInputComponent.h"
#include "DataAsset/Input/DataAsset_InputConfig.h"
#include "MHGameplayTags.h"

DEFINE_LOG_CATEGORY(LogMHPlayerCharacter)

AMHPlayerCharacter::AMHPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    // 회전 사용 비활성화
    bUseControllerRotationPitch = false;
    bUseControllerRotationRoll = false;
    bUseControllerRotationYaw = false;

    // 스프링암
    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(GetRootComponent());
    CameraBoom->TargetArmLength = 260.0f;
    CameraBoom->SocketOffset = FVector(0.f, 55.f, 65.f);
    CameraBoom->bUsePawnControlRotation = true;

    // 카메라
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    // 이동 세팅
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->bOrientRotationToMovement = true;
        MoveComp->RotationRate = FRotator(0.f, MovementConfig.RotationRateYaw, 0.f);
        MoveComp->BrakingDecelerationWalking = MovementConfig.BrakingDecelerationWalking;
        MoveComp->MaxWalkSpeed = MovementConfig.RunSpeed;
    }

    CurrentStamina = StaminaConfig.MaxStamina;

    // 데칼 영향 제거
    if (USkeletalMeshComponent* MeshComp = GetMesh())
    {
        MeshComp->bReceivesDecals = false;
    }
}

void AMHPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogMHPlayerCharacter, Log, TEXT("%s : BeginPlay"), *GetName());

    ApplyPlayerVisuals();

    // 무브먼트 업데이트 델리게이트 바인딩
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->OnMovementUpdated.AddDynamic(this, &AMHPlayerCharacter::HandleMovementUpdated);
    }

}

void AMHPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    checkf(InputConfigDataAsset, TEXT("InputConfigDataAsset is null"));

    APlayerController* PC = GetController<APlayerController>();
    check(PC);

    ULocalPlayer* LocalPlayer = PC->GetLocalPlayer();
    check(LocalPlayer);

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
    check(Subsystem);

    if (InputConfigDataAsset->DefaultMappingContext)
    {
        Subsystem->AddMappingContext(InputConfigDataAsset->DefaultMappingContext, 0);
    }

    UMHInputComponent* MHInputComponent = CastChecked<UMHInputComponent>(PlayerInputComponent);
    MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Move, ETriggerEvent::Triggered, this, &AMHPlayerCharacter::Input_Move);
    MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Look, ETriggerEvent::Triggered, this, &AMHPlayerCharacter::Input_Look);

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_Sprint))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Sprint, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_SprintStarted);
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Sprint, ETriggerEvent::Completed, this, &AMHPlayerCharacter::Input_SprintCompleted);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_Dodge))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Dodge, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_Dodge);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_Interact))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_Interact, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_Interact);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AttackPrimary))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackPrimary, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AttackPrimary);
    }

    if (InputConfigDataAsset->FindNativeInputActionByTag(MHGameplayTags::Input_AttackSecondary))
    {
        MHInputComponent->BindNativeInputAction(InputConfigDataAsset, MHGameplayTags::Input_AttackSecondary, ETriggerEvent::Started, this, &AMHPlayerCharacter::Input_AttackSecondary);
    }
}


void AMHPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 무브먼트 업데이트 델리게이트 해제
    if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
    {
        MoveComp->OnMovementUpdated.RemoveDynamic(this, &AMHPlayerCharacter::HandleMovementUpdated);
    }

    Super::EndPlay(EndPlayReason);
}

void AMHPlayerCharacter::HandleMovementUpdated(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
    UpdateStamina(DeltaSeconds);
    EvaluateSprintState();
    UpdateLocomotionState();
}


void AMHPlayerCharacter::Input_Move(const FInputActionValue& InputActionValue)
{
    const FVector2D MovementVector = InputActionValue.Get<FVector2D>();
    if (!Controller)
    {
        return;
    }

    const FRotator MovementRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);

    if (MovementVector.Y != 0.f)
    {
        const FVector ForwardDir = MovementRotation.RotateVector(FVector::ForwardVector);
        AddMovementInput(ForwardDir, MovementVector.Y);
    }

    if (MovementVector.X != 0.f)
    {
        const FVector RightDir = MovementRotation.RotateVector(FVector::RightVector);
        AddMovementInput(RightDir, MovementVector.X);
    }
}

void AMHPlayerCharacter::Input_Look(const FInputActionValue& InputActionValue)
{
    const FVector2D LookAxisVector = InputActionValue.Get<FVector2D>();

    if (LookAxisVector.X != 0.f)
    {
        AddControllerYawInput(LookAxisVector.X);
    }

    if (LookAxisVector.Y != 0.f)
    {
        AddControllerPitchInput(LookAxisVector.Y);
    }
}

void AMHPlayerCharacter::Input_SprintStarted(const FInputActionValue& InputActionValue)
{
    bSprintHeld = true;
    EvaluateSprintState();
}

void AMHPlayerCharacter::Input_SprintCompleted(const FInputActionValue& InputActionValue)
{
    bSprintHeld = false;
    EvaluateSprintState();
}

void AMHPlayerCharacter::Input_Dodge(const FInputActionValue& InputActionValue)
{
    UE_LOG(LogMHPlayerCharacter, Verbose, TEXT("%s : Dodge input"), *GetName());
}

void AMHPlayerCharacter::Input_Interact(const FInputActionValue& InputActionValue)
{
    Interact();
}

void AMHPlayerCharacter::Input_AttackPrimary(const FInputActionValue& InputActionValue)
{
    UsePrimaryAction();
}

void AMHPlayerCharacter::Input_AttackSecondary(const FInputActionValue& InputActionValue)
{
    UE_LOG(LogMHPlayerCharacter, Verbose, TEXT("%s : Secondary attack input"), *GetName());
}

void AMHPlayerCharacter::Interact()
{
}

void AMHPlayerCharacter::UsePrimaryAction()
{
}

void AMHPlayerCharacter::ApplyPlayerVisuals()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        return;
    }

    MeshComp->SetRelativeLocation(DefaultMeshRelativeLocation);
    MeshComp->SetRelativeRotation(DefaultMeshRelativeRotation);

    if (USkeletalMesh* LoadedMesh = DefaultSkeletalMesh.LoadSynchronous())
    {
        MeshComp->SetSkeletalMesh(LoadedMesh);
    }

    if (UClass* LoadedAnimClass = DefaultAnimClass.LoadSynchronous())
    {
        MeshComp->SetAnimInstanceClass(LoadedAnimClass);
    }
}

void AMHPlayerCharacter::ApplyMovementProfile(EMHPlayerMoveProfile InProfile)
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    MoveComp->RotationRate = FRotator(0.f, MovementConfig.RotationRateYaw, 0.f);
    MoveComp->BrakingDecelerationWalking = MovementConfig.BrakingDecelerationWalking;

    switch (InProfile)
    {
    case EMHPlayerMoveProfile::Run:
        MoveComp->MaxWalkSpeed = MovementConfig.RunSpeed;
        break;
    case EMHPlayerMoveProfile::Sprint:
        MoveComp->MaxWalkSpeed = MovementConfig.SprintSpeed;
        break;
    default:
        MoveComp->MaxWalkSpeed = MovementConfig.RunSpeed;
        break;
    }
}

void AMHPlayerCharacter::UpdateLocomotionState()
{
    const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    const float Speed2D = GetVelocity().Size2D();

    if (MoveComp && MoveComp->IsFalling())
    {
        // 공중 상태는 추후 확장
        return;
    }

    if (bIsSprinting)
    {
        LocomotionState = EMHPlayerLocomotionState::Sprint;
        return;
    }

    LocomotionState = (Speed2D < 3.0f) ? EMHPlayerLocomotionState::Idle : EMHPlayerLocomotionState::Move;
}

void AMHPlayerCharacter::UpdateStamina(float DeltaSeconds)
{
    if (DeltaSeconds <= 0.0f)
    {
        return;
    }

    if (bIsSprinting)
    {
        CurrentStamina -= StaminaConfig.SprintCostPerSecond * DeltaSeconds;
    }
    else
    {
        CurrentStamina += StaminaConfig.RecoveryPerSecond * DeltaSeconds;
    }

    CurrentStamina = FMath::Clamp(CurrentStamina, 0.0f, StaminaConfig.MaxStamina);

    if (bIsSprinting && CurrentStamina <= 0.0f)
    {
        bIsSprinting = false;
        ApplyMovementProfile(EMHPlayerMoveProfile::Run);
    }
}

void AMHPlayerCharacter::EvaluateSprintState()
{
    const bool bHasMoveInput = GetLastMovementInputVector().Size2D() > KINDA_SMALL_NUMBER;
    const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    const bool bInAir = MoveComp ? MoveComp->IsFalling() : false;

    const bool bWantsSprint = bSprintHeld && bHasMoveInput && !bInAir;

    if (bWantsSprint && CanStartSprint())
    {
        if (!bIsSprinting)
        {
            bIsSprinting = true;
            ApplyMovementProfile(EMHPlayerMoveProfile::Sprint);
        }
        return;
    }

    if (bIsSprinting)
    {
        bIsSprinting = false;
        ApplyMovementProfile(EMHPlayerMoveProfile::Run);
    }
}

bool AMHPlayerCharacter::CanStartSprint() const
{
    return CurrentStamina > StaminaConfig.LowStaminaThreshold;
}

bool AMHPlayerCharacter::TryEnterSlide()
{
    return false;
}

bool AMHPlayerCharacter::TryStartClimb()
{
    return false;
}

bool AMHPlayerCharacter::TryJumpOffLedge()
{
    return false;
}

void AMHPlayerCharacter::HandleLandingState()
{
}
