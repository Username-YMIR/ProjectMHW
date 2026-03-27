#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "MHWorldSettings.generated.h"

class AMHMonsterCharacterBase;

DECLARE_LOG_CATEGORY_EXTERN(LogMHWorldSettings, Log, All);

UCLASS()
class PROJECTMHW_API AMHWorldSettings : public AWorldSettings
{
    GENERATED_BODY()

public:
    AMHWorldSettings();

    const TArray<TObjectPtr<AMHMonsterCharacterBase>>& GetBGMClearTargetMonsters() const
    {
        return BGMClearTargetMonsters;
    }

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Audio|BGM")
    TArray<TObjectPtr<AMHMonsterCharacterBase>> BGMClearTargetMonsters;
};
