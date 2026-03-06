#pragma once
// 제작자 : 이건주
// 제작일 : 2026-03-05
// 수정일 : 2026-03-05 
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Type/MHCombatStatStructType.h" // FMHAttackStats, (앞에서 만든) 방어 Attribute 기반이면 데이터용 struct 따로 권장
#include "MHItemStructType.generated.h"

class UTexture2D;


/** 무기 종류(필요한 것만 확장) */
UENUM(BlueprintType)
enum class EMHWeaponType : uint8
{
	None			UMETA(DisplayName="None"),
	GreatSword		UMETA(DisplayName="Great Sword"),
	LongSword		UMETA(DisplayName="Long Sword"),
	SwordAndShield	UMETA(DisplayName="Sword & Shield"),
	DualBlades		UMETA(DisplayName="Dual Blades"),
	Hammer			UMETA(DisplayName="Hammer"),
	HuntingHorn		UMETA(DisplayName="Hunting Horn"),
	Lance			UMETA(DisplayName="Lance"),
	Gunlance		UMETA(DisplayName="Gunlance"),
	SwitchAxe		UMETA(DisplayName="Switch Axe"),
	ChargeBlade		UMETA(DisplayName="Charge Blade"),
	InsectGlaive	UMETA(DisplayName="Insect Glaive"),
	LightBowgun		UMETA(DisplayName="Light Bowgun"),
	HeavyBowgun		UMETA(DisplayName="Heavy Bowgun"),
	Bow				UMETA(DisplayName="Bow"),
};

/** 방어구 부위 */
UENUM(BlueprintType)
enum class EMHArmorType : uint8
{
	None	UMETA(DisplayName="None"),
	Head	UMETA(DisplayName="Head"),
	Chest	UMETA(DisplayName="Chest"),
	Arms	UMETA(DisplayName="Arms"),
	Waist	UMETA(DisplayName="Waist"),
	Legs	UMETA(DisplayName="Legs"),
};


/** 아이템 공통(베이스) */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FItemBaseData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	// 아이콘은 Soft로 두는 게 DT에서 안전함(참조로 인한 패키징/로딩 이슈 감소)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine="true"))
	FText Description;

	// 희귀도 등급 1~12
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1", ClampMax="12"))
	int Rarity = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0"))
	int32 SellPrice = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0"))
	int32 BuyPrice = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Item"))
	FGameplayTag ItemTag;
};

/** 일반 아이템 공통 - (요구사항이 "보류"라 최소만) */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FCommonItemData
{
	GENERATED_BODY()

	// 보류: 예) MaxStack, UseAction, CraftMaterialTags 등
};

/** 무기 아이템 공통 */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FWeaponItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMHWeaponType WeaponType = EMHWeaponType::None;

	// 공격 스테이터스(네가 이미 만든 FMHAttackStats 사용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMHAttackStats AttackStats;
};

/** 방어구 공통 */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FArmorItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMHArmorType ArmorType = EMHArmorType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMHDefenseStats ArmorStats;
};

/**
 * 최종 DataTable Row
 * - ItemType에 따라 Weapon/Armor 세부 데이터 사용
 * - 블루프린트/DT 편집 편의상 중첩 struct로 들고 가는 형태
 */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHItemDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FItemBaseData Base;

	// 일반 아이템(보류지만 슬롯은 마련)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FCommonItemData Common;

	// 무기
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FWeaponItemData Weapon;

	// 방어구
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FArmorItemData Armor;
};