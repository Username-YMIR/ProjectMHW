#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "DataAsset/Input/DataAsset_InputConfig.h"
#include "MHInputComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHInputComponent, Log, All)

UCLASS()
class PROJECTMHW_API UMHInputComponent : public UEnhancedInputComponent
{
    GENERATED_BODY()

public:
    // 태그로 입력 바인딩
    template<class UserObject, typename CallbackFunc>
    void BindNativeInputAction(
        const UDataAsset_InputConfig* InInputConfig,
        const FGameplayTag& InInputTag,
        ETriggerEvent TriggerEvent,
        UserObject* ContextObject,
        CallbackFunc Func);
};

template<class UserObject, typename CallbackFunc>
inline void UMHInputComponent::BindNativeInputAction(
    const UDataAsset_InputConfig* InInputConfig,
    const FGameplayTag& InInputTag,
    ETriggerEvent TriggerEvent,
    UserObject* ContextObject,
    CallbackFunc Func)
{
    checkf(InInputConfig, TEXT("Input config data asset is null"));

    if (UInputAction* FoundAction = InInputConfig->FindNativeInputActionByTag(InInputTag))
    {
        BindAction(FoundAction, TriggerEvent, ContextObject, Func);
    }
}
