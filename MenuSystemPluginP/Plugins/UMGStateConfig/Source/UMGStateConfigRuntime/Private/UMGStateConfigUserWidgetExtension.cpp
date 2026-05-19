#include "UMGStateConfigUserWidgetExtension.h"

#include "Blueprint/UserWidget.h"
#include "UUsers\userb85bbe4d\projectsGStateConfigPropertyRuntimeLibrary.h"

bool UUMGStateConfigUserWidgetExtension::ApplyUIState(FName StateGroupName, FName StateName)
{
	UUserWidget* TargetUserWidget = GetUserWidget();
	if (!TargetUserWidget)
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("ApplyUIState failed: owner widget is null."));
		return false;
	}

	const FUMGStateConfigGroup* TargetGroup = FindStateGroup(StateGroupName);
	if (!TargetGroup)
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("State group not found: %s"), *StateGroupName.ToString());
		return false;
	}

	const FName EffectiveStateName = StateName.IsNone() ? TargetGroup->DefaultStateName : StateName;
	const FUMGStateConfigState* TargetState = FindState(*TargetGroup, EffectiveStateName);
	if (!TargetState)
	{
		UE_LOG(LogUUsers\userb85bbe4d\projectsGStateConfig, Warning, TEXT("State not found: Group=%s, State=%s"), *StateGroupName.ToString(), *EffectiveStateName.ToString());
		return false;
	}

	RestorePreviousValues(TargetUserWidget, StateGroupName);

	bool bAllChangesSucceeded = true;
	for (const FUMGStatePropertyChange& Change : TargetState->PropertyChanges)
	{
		bAllChangesSucceeded &= ApplyPropertyChange(TargetUserWidget, StateGroupName, Change);
	}

	return bAllChangesSucceeded;
}

void UUMGStateConfigUserWidgetExtension::SetRuntimeData(const FUMGStateConfigRuntimeData& NewData)
{
	ResetActiveState();
	RuntimeData = NewData;
	InvalidateLookupCache();
}

void UUMGStateConfigUserWidgetExtension::ResetActiveState()
{
	if (UUserWidget* TargetUserWidget = GetUserWidget())
	{
		RestoreAllGroups(TargetUserWidget);
	}
}

bool UUMGStateConfigUserWidgetExtension::ApplyPropertyChange(UUserWidget* TargetUserWidget, FName StateGroupName, const FUMGStatePropertyChange& Change)
{
	if (!TargetUserWidget || Change.TargetWidgetName.IsNone())
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("Invalid property change: target widget is empty."));
		return false;
	}

	UWidget* TargetWidget = ResolveWidget(TargetUserWidget, Change.TargetWidgetName);
	if (!TargetWidget)
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("Widget not found: %s"), *Change.TargetWidgetName.ToString());
		return false;
	}

	if (*Change.ExpectedWidgetClass && !TargetWidget->IsA(Change.ExpectedWidgetClass))
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("Type mismatch: %s expected %s, actual %s"),
			*Change.TargetWidgetName.ToString(),
			*Change.ExpectedWidgetClass->GetName(),
			*TargetWidget->GetClass()->GetName());
		return false;
	}

	FUMGStateConfigPropertyValue CurrentValue;
	CurrentValue.SerializedPropertyPath = Change.Value.SerializedPropertyPath;
	const bool bHasCurrentValue = FUMGStateConfigPropertyRuntimeLibrary::CaptureCurrentValue(TargetWidget, Change.PropertyType, CurrentValue);
	if (bHasCurrentValue && FUMGStateConfigPropertyRuntimeLibrary::ArePropertyValuesEqual(Change.PropertyType, CurrentValue, Change.Value))
	{
		return true;
	}

	const FUMGStateConfigChangeKey ChangeKey{ Change.TargetWidgetName, Change.PropertyType, Change.Value.SerializedPropertyPath };

	FUMGStateConfigGroupRestoreData& Data = GroupRestoreData.FindOrAdd(StateGroupName);
	if (!Data.RestoreValues.Contains(ChangeKey) && bHasCurrentValue)
	{
		Data.RestoreValues.Add(ChangeKey, CurrentValue);
		Data.ActiveChangeKeys.Add(ChangeKey);
	}

	return FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(TargetWidget, Change.PropertyType, Change.Value);
}

UWidget* UUMGStateConfigUserWidgetExtension::ResolveWidget(UUserWidget* TargetUserWidget, FName WidgetName)
{
	if (!TargetUserWidget || WidgetName.IsNone())
	{
		return nullptr;
	}

	if (TWeakObjectPtr<UWidget>* CachedWidget = WidgetCache.Find(WidgetName))
	{
		if (CachedWidget->IsValid())
		{
			return CachedWidget->Get();
		}
	}

	UWidget* ResolvedWidget = TargetUserWidget->GetWidgetFromName(WidgetName);
	if (ResolvedWidget)
	{
		WidgetCache.Add(WidgetName, ResolvedWidget);
	}
	else
	{
		WidgetCache.Remove(WidgetName);
	}
	return ResolvedWidget;
}

const FUMGStateConfigGroup* UUMGStateConfigUserWidgetExtension::FindStateGroup(FName StateGroupName)
{
	BuildLookupCache();

	const int32* GroupIndex = StateGroupIndexByName.Find(StateGroupName);
	if (!GroupIndex || !RuntimeData.StateGroups.IsValidIndex(*GroupIndex))
	{
		return nullptr;
	}
	return &RuntimeData.StateGroups[*GroupIndex];
}

const FUUsers\userb85bbe4d\projectsGStateConfigState* UUMGStateConfigUserWidgetExtension::FindState(const FUMGStateConfigGroup& Group, FName StateName)
{
	BuildLookupCache();

	const TMap<FName, int32>* StateIndexByName = StateIndexByGroupName.Find(Group.GroupName);
	if (!StateIndexByName)
	{
		return nullptr;
	}

	const int32* StateIndex = StateIndexByName->Find(StateName);
	if (!StateIndex || !Group.States.IsValidIndex(*StateIndex))
	{
		return nullptr;
	}
	return &Group.States[*StateIndex];
}

void UUMGStateConfigUserWidgetExtension::BuildLookupCache()
{
	if (bLookupCacheBuilt)
	{
		return;
	}

	StateGroupIndexByName.Reset();
	StateIndexByGroupName.Reset();
	for (int32 GroupIndex = 0; GroupIndex < RuntimeData.StateGroups.Num(); ++GroupIndex)
	{
		const FUMGStateConfigGroup& Group = RuntimeData.StateGroups[GroupIndex];
		StateGroupIndexByName.Add(Group.GroupName, GroupIndex);

		TUsers\userb85bbe4d\projectsap<FName, int32> StateIndexByName;
		for (int32 StateIndex = 0; StateIndex < Group.States.Num(); ++StateIndex)
		{
			StateIndexByName.Add(Group.States[StateIndex].StateName, StateIndex);
		}
		StateIndexByGroupName.Add(Group.GroupName, Users\userb85bbe4d\projectsoveTemp(StateIndexByName));
	}

	bLookupCacheBuilt = true;
}

void UUMGStateConfigUserWidgetExtension::InvalidateLookupCache()
{
	bLookupCacheBuilt = false;
	StateGroupIndexByName.Reset();
	StateIndexByGroupName.Reset();
}

void UUMGStateConfigUserWidgetExtension::RestorePreviousValues(UUserWidget* TargetUserWidget, FName StateGroupName)
{
	if (!TargetUserWidget)
	{
		return;
	}

	FUUsers\userb85bbe4d\projectsGStateConfigGroupRestoreData* Data = GroupRestoreData.Find(StateGroupName);
	if (!Data)
	{
		return;
	}

	for (const FUMGStateConfigChangeKey& ChangeKey : Data->ActiveChangeKeys)
	{
		const FUMGStateConfigPropertyValue* RestoreValue = Data->RestoreValues.Find(ChangeKey);
		if (!RestoreValue)
		{
			continue;
		}

		UWidget* TargetWidget = ResolveWidget(TargetUserWidget, ChangeKey.TargetWidgetName);
		if (!TargetWidget)
		{
			continue;
		}

		FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(TargetWidget, ChangeKey.PropertyType, *RestoreValue);
	}

	GroupRestoreData.Remove(StateGroupName);
}

void UUMGStateConfigUserWidgetExtension::RestoreAllGroups(UUserWidget* TargetUserWidget)
{
	if (!TargetUserWidget)
	{
		return;
	}

	for (auto& Pair : GroupRestoreData)
	{
		for (const FUMGStateConfigChangeKey& ChangeKey : Pair.Value.ActiveChangeKeys)
		{
			const FUMGStateConfigPropertyValue* RestoreValue = Pair.Value.RestoreValues.Find(ChangeKey);
			if (!RestoreValue)
			{
				continue;
			}

			UWidget* TargetWidget = ResolveWidget(TargetUserWidget, ChangeKey.TargetWidgetName);
			if (!TargetWidget)
			{
				continue;
			}

			FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(TargetWidget, ChangeKey.PropertyType, *RestoreValue);
		}
	}

	GroupRestoreData.Empty();
}
