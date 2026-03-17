#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "MHDamageSpecReceiverInterface.generated.h"

/**
 * 피격 결과 타입
 * - 이후 가드, 튕김, 무적 등으로 확장 가능
 */
UENUM(BlueprintType)
enum class EMHHitResultType : uint8
{
	None			UMETA(DisplayName="None"),
	NormalHit		UMETA(DisplayName="NormalHit"),
	Guarded			UMETA(DisplayName="Guarded"),
	Bounced			UMETA(DisplayName="Bounced"),
	Invulnerable	UMETA(DisplayName="Invulnerable")
};

/**
 * 공격 대상이 공격자에게 돌려주는 피격 응답값
 */
USTRUCT(BlueprintType)
struct FMHHitAcknowledge
{
	GENERATED_BODY()

public:
	/** 이번 타격을 유효 히트로 인정했는가 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit")
	bool bAcceptedHit = false;

	/** 이번 히트로 1회 타격 기회를 소비할 것인가 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit")
	bool bConsumeHitOnce = false;

	/** 이번 결과로 공격 윈도우를 즉시 종료할 것인가 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit")
	bool bShouldStopAttackWindow = false;

	/** 상세 피격 결과 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Hit")
	EMHHitResultType ResultType = EMHHitResultType::None;
};

UINTERFACE(MinimalAPI, Blueprintable)
class UMHDamageSpecReceiverInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * DamageSpec 수신 인터페이스
 * - 무기/공격자는 대상의 구체 클래스에 의존하지 않고 이 인터페이스만 호출한다.
 * - 피격자는 전달받은 DamageSpec을 ASC에 적용하거나, 가드/무적 판정 등을 처리한 뒤 결과를 반환한다.
 */
class PROJECTMHW_API IMHDamageSpecReceiverInterface
{
	GENERATED_BODY()

public:
	/**
	 * DamageSpec을 수신하고 처리한다.
	 *
	 * @param SourceActor			공격 주체 액터
	 * @param SourceWeapon			공격에 사용된 무기 액터
	 * @param AttackTag				공격 식별 태그
	 * @param DamageSpecHandle		적용할 GameplayEffectSpec 핸들
	 * @param HitResult				이번 피격의 충돌 정보
	 *
	 * @return						피격 결과 응답값
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Combat|Damage")
	FMHHitAcknowledge ReceiveDamageSpec(
		AActor* SourceActor,
		AActor* SourceWeapon,
		FGameplayTag AttackTag,
		const FGameplayEffectSpecHandle& DamageSpecHandle,
		const FHitResult& HitResult
	);
};