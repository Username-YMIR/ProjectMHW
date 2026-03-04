#include "DataAsset/Input/DataAsset_InputConfig.h"

DEFINE_LOG_CATEGORY(LogDataAsset_InputConfig)

UInputAction* UDataAsset_InputConfig::FindNativeInputActionByTag(const FGameplayTag& InInputTag) const
{
    for (const FMHInputActionConfig& ActionConfig : NativeInputActions)
    {
        if (ActionConfig.InputTag == InInputTag && ActionConfig.InputAction)
        {
            return ActionConfig.InputAction;
        }
    }

    return nullptr;
}
