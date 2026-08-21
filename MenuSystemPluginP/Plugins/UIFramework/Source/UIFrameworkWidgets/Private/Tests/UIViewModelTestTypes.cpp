// Copyright TikiStar. All Rights Reserved.

#include "UIViewModelTestTypes.h"

void UUIViewModelTestSubject::StartCountingBroadcasts()
{
	BroadcastCounts.Reset();

	GetFieldNotificationDescriptor().ForEachField(GetClass(),
		[this](const UE::FieldNotification::FFieldId FieldId)
		{
			const FName FieldName = FieldId.GetName();
			AddFieldValueChangedDelegate(
				FieldId,
				FFieldValueChangedDelegate::CreateWeakLambda(
					this,
					[this, FieldName](UObject*, UE::FieldNotification::FFieldId)
					{
						BroadcastCounts.FindOrAdd(FieldName)++;
					}));
			return true;
		});
}

int32 UUIViewModelTestSubject::GetBroadcastCount(FName FieldName) const
{
	const int32* Found = BroadcastCounts.Find(FieldName);
	return Found ? *Found : 0;
}
