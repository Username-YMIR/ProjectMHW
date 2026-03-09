#pragma once

//손승우 추가: 공격 메타 DataTable Row

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MHAttackMetaTypes.generated.h"

class UCameraShakeBase;
class UNiagaraSystem;
class USoundBase;

USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHAttackMetaRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	FGameplayTag MoveTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta", meta = (ClampMin = "0.0"))
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta", meta = (ClampMin = "0.0"))
	float PartBreakMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta", meta = (ClampMin = "0.0"))
	float HitStopSeconds = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	TSoftClassPtr<UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta", meta = (ClampMin = "0.0"))
	float CameraShakeScale = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	TSoftObjectPtr<UNiagaraSystem> HitEffectNiagara;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	TSoftObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	bool bUseDirectionalHitFX = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta", meta = (ClampMin = "0.0"))
	float StaminaCost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	float SpiritGaugeGain = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	float SpiritGaugeConsume = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	bool bCanSever = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	FName AttackType = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Attack Meta")
	FString Notes;
};
