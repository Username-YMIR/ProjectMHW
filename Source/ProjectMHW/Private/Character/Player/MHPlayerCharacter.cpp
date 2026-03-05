#include "Character/Player/MHPlayerCharacter.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

#include "Components/Input/MHInputComponent.h"
#include "DataAsset/Input/DataAsset_InputConfig.h"
#include "MHGameplayTags.h"

#include "Animation/AnimInstance.h"

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

    ApplyMovementConfig();

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
}

void AMHPlayerCharacter::ApplyPlayerVisuals()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!MeshComp)
    {
        return;
    }

    if (DefaultSkeletalMesh.IsNull() == false)
    {
        if (USkeletalMesh* MeshAsset = DefaultSkeletalMesh.LoadSynchronous())
        {
            MeshComp->SetSkeletalMesh(MeshAsset);
        }
    }

    if (DefaultAnimClass.IsNull() == false)
    {
        if (UClass* AnimClass = DefaultAnimClass.LoadSynchronous())
        {
            MeshComp->SetAnimInstanceClass(AnimClass);
        }
    }
}

void AMHPlayerCharacter::ApplyMovementConfig()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!MoveComp)
    {
        return;
    }

    MoveComp->bOrientRotationToMovement = true;
    MoveComp->RotationRate = FRotator(0.f, MovementConfig.RotationRateYaw, 0.f);
    MoveComp->MaxWalkSpeed = MovementConfig.RunSpeed;
    MoveComp->BrakingDecelerationWalking = MovementConfig.BrakingDecelWalking;
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

void AMHPlayerCharacter::Interact()
{
}

void AMHPlayerCharacter::UsePrimaryAction()
{
}
