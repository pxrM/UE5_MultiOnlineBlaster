// Copyright TikiStar. All Rights Reserved.

#include "Widgets/UIListBase.h"
#include "Components/ListView.h"

void UUIListBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ListView)
	{
		ListView->OnItemClicked().AddUObject(this, &UUIListBase::HandleItemClicked);
	}
}

void UUIListBase::SetItems(const TArray<UObject*>& InItems)
{
	if (!ListView)
	{
		return;
	}
	ListView->ClearListItems();
	for (UObject* It : InItems)
	{
		if (It)
		{
			ListView->AddItem(It);
		}
	}
}

void UUIListBase::AddItem(UObject* InItem)
{
	if (ListView && InItem)
	{
		ListView->AddItem(InItem);
	}
}

void UUIListBase::ClearItems()
{
	if (ListView)
	{
		ListView->ClearListItems();
	}
}

UObject* UUIListBase::GetSelectedItem() const
{
	return ListView ? ListView->GetSelectedItem<UObject>() : nullptr;
}

void UUIListBase::HandleItemClicked(UObject* ClickedItem)
{
	OnItemClicked.Broadcast(ClickedItem);
}
