#pragma once

#include "CoreMinimal.h"
#include "MHPlayerStructType.generated.h"

// 플레이어 로코모션 상태
UENUM(BlueprintType)
enum class EMHPlayerLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Move        UMETA(DisplayName = "Move"),       // 기본 달리기
    Sprint      UMETA(DisplayName = "Sprint"),
    Roll        UMETA(DisplayName = "Roll"),
    PanicDive   UMETA(DisplayName = "PanicDive"),
    Slide       UMETA(DisplayName = "Slide"),
    Climb       UMETA(DisplayName = "Climb"),
    JumpOff     UMETA(DisplayName = "JumpOff"),
    Landing     UMETA(DisplayName = "Landing"),
    Disabled    UMETA(DisplayName = "Disabled"),
};

// 플레이어 이동 프로파일
UENUM(BlueprintType)
enum class EMHPlayerMoveProfile : uint8
{
    Run     UMETA(DisplayName = "Run"),
    Sprint  UMETA(DisplayName = "Sprint"),
};

// 무기 납도/발도 상태
UENUM(BlueprintType)
enum class EMHWeaponSheathState : uint8
{
    Sheathed     UMETA(DisplayName = "Sheathed"),     // 납도(등에 있음)
    Unsheathing  UMETA(DisplayName = "Unsheathing"),  // 발도 진행 중
    Unsheathed   UMETA(DisplayName = "Unsheathed"),   // 발도(손에 있음)
    Sheathing    UMETA(DisplayName = "Sheathing"),    // 납도 진행 중
};

// 플레이어 스태미나 설정
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHPlayerStaminaConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float MaxStamina = 100.0f; // 최대 스태미나

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float SprintCostPerSecond = 12.0f; // 질주 초당 소모

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float RecoveryPerSecond = 15.0f; // 초당 회복

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float LowStaminaThreshold = 10.0f; // 질주 불가 기준

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float RollCost = 18.0f; // 구르기 소모(예비)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float PanicDiveCost = 22.0f; // 긴급회피 소모(예비)
};

// 플레이어 이동 설정
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHPlayerMovementConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float RunSpeed = 420.0f; // 기본 달리기 속도

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float SprintSpeed = 560.0f; // 질주 속도

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float RotationRateYaw = 520.0f; // 회전 속도(Yaw)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float BrakingDecelerationWalking = 2200.0f; // 감속
};

// 무기 소켓 설정
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHWeaponSocketConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName BackSocketName = TEXT("Weapon_Back"); // 등에 붙는 소켓

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
    FName HandSocketName = TEXT("Weapon_Hand"); // 손에 붙는 소켓
};
