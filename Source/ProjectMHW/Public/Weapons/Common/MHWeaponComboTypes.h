#pragma once

#include "CoreMinimal.h"
#include "MHWeaponComboTypes.generated.h"

// 콤보 입력 타입
UENUM(BlueprintType)
enum class EMHComboInputType : uint8
{
    None UMETA(DisplayName = "None"),
    Primary UMETA(DisplayName = "Primary"),
    Secondary UMETA(DisplayName = "Secondary"),
    Special UMETA(DisplayName = "Special"),
};
