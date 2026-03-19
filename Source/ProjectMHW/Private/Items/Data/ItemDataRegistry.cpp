// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Data/ItemDataRegistry.h"

const UMHItemDataBase* UItemDataRegistry::GetItemData(const FName KeyName) const
{
	if (const UMHItemDataBase* const* Found = ItemDataMap.Find(KeyName))
	{
		return *Found;
	}

	return nullptr;
}