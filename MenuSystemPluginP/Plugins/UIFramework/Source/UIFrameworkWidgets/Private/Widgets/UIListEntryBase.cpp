// Copyright TikiStar. All Rights Reserved.

#include "Widgets/UIListEntryBase.h"

void UUIListEntryBase::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	Item = ListItemObject;
	OnItemSet(ListItemObject);
}
