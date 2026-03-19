// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHItemInstanceBase.h"

#include "Items/Data/ItemDataRegistry.h"


// Sets default values
AMHItemInstanceBase::AMHItemInstanceBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AMHItemInstanceBase::BeginPlay()
{
	Super::BeginPlay();
	ApplyItemData();
}

void AMHItemInstanceBase::ApplyItemData()
{
	CachedItemData = nullptr;

	if (!ItemRegistry || ItemDataKey.IsNone())
	{
		return;
	}

	UMHItemDataBase** FoundData = ItemRegistry->ItemDataMap.Find(ItemDataKey);

	if (!FoundData || !(*FoundData))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid ItemDataKey: %s"), *ItemDataKey.ToString());
		return;
	}

	CachedItemData = *FoundData;

	// 파생 클래스에서 override하여 실제 적용
}

#if WITH_EDITOR
void AMHItemInstanceBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.Property)
	{
		return;
	}

	const FName ChangedPropName = PropertyChangedEvent.Property->GetFName();

	static const FName NAME_ItemDataKey =
		GET_MEMBER_NAME_CHECKED(AMHItemInstanceBase, ItemDataKey);

	static const FName NAME_ItemRegistry =
		GET_MEMBER_NAME_CHECKED(AMHItemInstanceBase, ItemRegistry);

	if (ChangedPropName == NAME_ItemDataKey ||
		ChangedPropName == NAME_ItemRegistry)
	{
		ApplyItemData();
	}
}
#endif