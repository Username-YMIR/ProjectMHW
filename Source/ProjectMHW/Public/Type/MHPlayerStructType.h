#pragma once

#include "CoreMinimal.h"
#include "MHPlayerStructType.generated.h"

// 플레이어 로코모션 상태
UENUM(BlueprintType)
enum class EMHPlayerLocomotionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Move        UMETA(DisplayName = "Move"),
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
    Walk    UMETA(DisplayName = "Walk"),
    Run     UMETA(DisplayName = "Run"),
    Sprint  UMETA(DisplayName = "Sprint"),
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
    float RollCost = 18.0f; // 구르기 소모

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float PanicDiveCost = 22.0f; // 긴급회피 소모

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float RecoveryPerSecond = 15.0f; // 초당 회복

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina")
    float LowThreshold = 10.0f; // 저스태미나 기준
};

// 플레이어 이동 설정
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHPlayerMovementConfig
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float WalkSpeed = 220.0f; // 걷기 속도

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float RunSpeed = 420.0f; // 달리기 속도

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float SprintSpeed = 560.0f; // 질주 속도

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float RotationRateYaw = 520.0f; // 회전 속도(Yaw)

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
    float BrakingDecelWalking = 2200.0f; // 감속
};
