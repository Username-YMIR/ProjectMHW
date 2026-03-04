#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "DataAsset_InputConfig.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDataAsset_InputConfig, Log, All)

class UInputAction;
class UInputMappingContext;

USTRUCT(BlueprintType)
struct FMHInputActionConfig
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
    FGameplayTag InputTag; // 입력 태그

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputAction> InputAction; // 입력 액션
};

UCLASS()
class PROJECTMHW_API UDataAsset_InputConfig : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<UInputMappingContext> DefaultMappingContext; // 기본 IMC

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (TitleProperty = "InputTag"))
    TArray<FMHInputActionConfig> NativeInputActions; // 태그-액션 목록

    // 태그로 입력 액션 반환
    UInputAction* FindNativeInputActionByTag(const FGameplayTag& InInputTag) const;
};
