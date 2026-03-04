#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Type/MHCombatStatStructType.h" // FMHAttackStats, (앞에서 만든) 방어 Attribute 기반이면 데이터용 struct 따로 권장
#include "MHItemStructType.generated.h"

class UTexture2D;

/** 아이템 등급 */
UENUM(BlueprintType)
enum class EMHItemRarity : uint8
{
	Common		UMETA(DisplayName="Common"),
	Uncommon	UMETA(DisplayName="Uncommon"),
	Rare		UMETA(DisplayName="Rare"),
	Epic		UMETA(DisplayName="Epic"),
	Legendary	UMETA(DisplayName="Legendary"),
};

/** 아이템 대분류 */
UENUM(BlueprintType)
enum class EMHItemType : uint8
{
	None		UMETA(DisplayName="None"),
	Consumable	UMETA(DisplayName="Consumable"),
	Material	UMETA(DisplayName="Material"),
	Weapon		UMETA(DisplayName="Weapon"),
	Armor		UMETA(DisplayName="Armor"),
};

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

/**
 * 방어구 "데이터"용 스탯
 * - 런타임은 AttributeSet + GE로 적용할 거라, DT에는 단순 수치만 둠.
 */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHArmorStatsData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float DefenseAdd = 0.f;

	// 속성 내성(고정 5종 권장)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Resist_Fire_Add = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Resist_Water_Add = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Resist_Thunder_Add = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Resist_Ice_Add = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Resist_Dragon_Add = 0.f;

	// 확장 여지(상태이상/특수 내성 등)를 태그로 추가하고 싶으면 여기에 옵션으로:
	// UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Resist"))
	// TMap<FGameplayTag, float> ExtraResistByTag;
};

/** 아이템 공통(베이스) */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHItemBaseData
{
	GENERATED_BODY()

	// DataTable RowName을 “아이템ID”로 쓸 거면 Name/ID는 별도 저장 안 해도 됨.
	// 그래도 내부에서 안정적으로 참조하려면 ItemId를 두는 걸 추천.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName ItemId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	// 아이콘은 Soft로 두는 게 DT에서 안전함(참조로 인한 패키징/로딩 이슈 감소)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine="true"))
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMHItemRarity Rarity = EMHItemRarity::Common;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0"))
	int32 SellPrice = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMHItemType ItemType = EMHItemType::None;
};

/** 일반 아이템 공통 - (요구사항이 "보류"라 최소만) */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHCommonItemData
{
	GENERATED_BODY()

	// 보류: 예) MaxStack, UseAction, CraftMaterialTags 등
};

/** 무기 아이템 공통 */
USTRUCT(BlueprintType)
struct PROJECTMHW_API FMHWeaponItemData
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
struct PROJECTMHW_API FMHArmorItemData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EMHArmorType ArmorType = EMHArmorType::None;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMHArmorStatsData ArmorStats;
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
	FMHItemBaseData Base;

	// 일반 아이템(보류지만 슬롯은 마련)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMHCommonItemData Common;

	// 무기
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMHWeaponItemData Weapon;

	// 방어구
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FMHArmorItemData Armor;
};