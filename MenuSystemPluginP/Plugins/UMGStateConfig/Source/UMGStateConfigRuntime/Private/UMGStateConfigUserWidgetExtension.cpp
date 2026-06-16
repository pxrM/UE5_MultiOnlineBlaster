#include "UMGStateConfigUserWidgetExtension.h"

#include "Blueprint/UserWidget.h"
#include "UMGStateConfigPropertyRuntimeLibrary.h"

namespace
{
constexpr int32 GUMGStateConfigCurrentSchemaVersion = 1;
}

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
		UE_LOG(LogUMGStateConfig, Warning, TEXT("State not found: Group=%s, State=%s"), *StateGroupName.ToString(), *EffectiveStateName.ToString());
		return false;
	}

	// 收集目标状态引用的资产；未加载完先异步加载，避免应用时同步加载导致卡帧
	TArray<FSoftObjectPath> StateReferencedAssets;
	for (const FUMGStatePropertyChange& Change : TargetState->PropertyChanges)
	{
		StateReferencedAssets.Append(Change.Value.SerializedReferencedAssets);
	}

	const FName ResolvedGroupName = TargetGroup->GroupName;

	// 资产已全部就绪：直接同步应用，保留真实返回值
	if (FUMGStateConfigPropertyRuntimeLibrary::AreReferencedAssetsLoaded(StateReferencedAssets))
	{
		PendingApplyHandles.Remove(ResolvedGroupName);
		return ApplyResolvedState(ResolvedGroupName, EffectiveStateName);
	}

	// 有未加载资产：异步加载完成后再应用，避免阻塞主线程
	TWeakObjectPtr<UUMGStateConfigUserWidgetExtension> WeakThis(this);
	PendingApplyHandles.Add(ResolvedGroupName, FUMGStateConfigPropertyRuntimeLibrary::RequestPreloadReferencedAssetsAsync(
		StateReferencedAssets,
		[WeakThis, ResolvedGroupName, EffectiveStateName]()
		{
			if (UUMGStateConfigUserWidgetExtension* StrongThis = WeakThis.Get())
			{
				StrongThis->ApplyResolvedState(ResolvedGroupName, EffectiveStateName);
			}
		}));

	// 资产异步加载中，应用已派发
	return true;
}

bool UUMGStateConfigUserWidgetExtension::ApplyResolvedState(FName StateGroupName, FName StateName)
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

	const FUMGStateConfigState* TargetState = FindState(*TargetGroup, StateName);
	if (!TargetState)
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("State not found: Group=%s, State=%s"), *StateGroupName.ToString(), *StateName.ToString());
		return false;
	}

	// 收集新状态将要覆盖的 ChangeKey，避免先恢复再覆盖的无效操作
	TSet<FUMGStateConfigChangeKey> NewStateChangeKeys;
	for (const FUMGStatePropertyChange& Change : TargetState->PropertyChanges)
	{
		NewStateChangeKeys.Add(FUMGStateConfigChangeKey{ Change.TargetWidgetName, Change.PropertyType, Change.Value.SerializedPropertyPath });
	}

	// 只恢复目标组中不会被新状态覆盖的属性值
	TSet<FUMGStateConfigChangeKey> TargetTouchedKeys = NewStateChangeKeys;

	if (FUMGActiveStateGroupRuntime* ExistingActiveGroup = ActiveStateGroups.Find(TargetGroup->GroupName))
	{
		for (const TPair<FUMGStateConfigChangeKey, FUMGStateConfigPropertyValue>& RestorePair : ExistingActiveGroup->RestoreValues)
		{
			TargetTouchedKeys.Add(RestorePair.Key);
			if (NewStateChangeKeys.Contains(RestorePair.Key))
			{
				continue;
			}
			UWidget* RestoreTargetWidget = ResolveWidget(TargetUserWidget, RestorePair.Key.TargetWidgetName);
			if (RestoreTargetWidget)
			{
				FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(RestoreTargetWidget, RestorePair.Key.PropertyType, RestorePair.Value, false);
				QueueWidgetRefresh(RestoreTargetWidget);
			}
		}
		ExistingActiveGroup->ActiveChangeKeys.Empty();
		ExistingActiveGroup->RestoreValues.Empty();
	}

	FUMGActiveStateGroupRuntime& ActiveGroupRuntime = ActiveStateGroups.FindOrAdd(TargetGroup->GroupName);
	ActiveGroupRuntime.ActiveStateName = TargetState->StateName;

	const bool bResult = ReapplyActiveStates(TargetUserWidget, TargetGroup, TargetTouchedKeys);
	FlushPendingWidgetRefreshes();
	return bResult;
}

void UUMGStateConfigUserWidgetExtension::ResetActiveState()
{
	if (UUserWidget* TargetUserWidget = GetUserWidget())
	{
		RestoreGlobalValues(TargetUserWidget);
		FlushPendingWidgetRefreshes();
		ActiveStateGroups.Empty();
		PendingApplyHandles.Empty();
	}
}


void UUMGStateConfigUserWidgetExtension::SetRuntimeData(const FUMGStateConfigRuntimeData& InRuntimeData)
{
	ResetActiveState();
	RuntimeData = InRuntimeData;
	NormalizeRuntimeData();
	InvalidateLookupCache();
	PreloadReferencedAssets(/*bAsync=*/ true);
}


bool UUMGStateConfigUserWidgetExtension::ApplyPropertyChange(UUserWidget* TargetUserWidget, const FUMGStatePropertyChange& Change, FUMGActiveStateGroupRuntime& ActiveGroupRuntime)
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
	if (!bHasCurrentValue)
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("Capture property value failed: Widget=%s, PropertyPath=%s"),
			*Change.TargetWidgetName.ToString(),
			*Change.Value.SerializedPropertyPath);
	}
	if (bHasCurrentValue && FUMGStateConfigPropertyRuntimeLibrary::ArePropertyValuesEqual(Change.PropertyType, CurrentValue, Change.Value))
	{
		return true;
	}

	const FUMGStateConfigChangeKey ChangeKey{ Change.TargetWidgetName, Change.PropertyType, Change.Value.SerializedPropertyPath };

	if (!GlobalRestoreValues.Contains(ChangeKey) && bHasCurrentValue)
	{
		GlobalRestoreValues.Add(ChangeKey, CurrentValue);
	}
	if (!ActiveGroupRuntime.RestoreValues.Contains(ChangeKey) && bHasCurrentValue)
	{
		ActiveGroupRuntime.RestoreValues.Add(ChangeKey, CurrentValue);
	}
	ActiveGroupRuntime.ActiveChangeKeys.AddUnique(ChangeKey);

	const bool bApplied = FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(TargetWidget, Change.PropertyType, Change.Value, false);
	if (bApplied)
	{
		QueueWidgetRefresh(TargetWidget);
	}
	if (!bApplied)

	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("Apply property value failed: Widget=%s, PropertyPath=%s, Value=%s"),
			*Change.TargetWidgetName.ToString(),
			*Change.Value.SerializedPropertyPath,
			*Change.Value.SerializedPropertyValue);
	}
	return bApplied;
}

bool UUMGStateConfigUserWidgetExtension::ReapplyActiveStates(UUserWidget* TargetUserWidget, const FUMGStateConfigGroup* ChangedGroup, const TSet<FUMGStateConfigChangeKey>& TargetTouchedKeys)
{
	// Only reapply groups affected by this change: the changed group itself,
	// plus any active group sharing a ChangeKey with it. Disjoint groups keep
	// their applied values untouched, avoiding full-replay Export/Import cost.
	TArray<const FUMGStateConfigGroup*> AffectedGroups;
	for (const FUMGStateConfigGroup& Group : RuntimeData.StateGroups)
	{
		const FUMGActiveStateGroupRuntime* ActiveGroupRuntime = ActiveStateGroups.Find(Group.GroupName);
		if (!ActiveGroupRuntime || ActiveGroupRuntime->ActiveStateName.IsNone())
		{
			continue;
		}

		if (ChangedGroup && Group.GroupName == ChangedGroup->GroupName)
		{
			AffectedGroups.Add(&Group);
			continue;
		}

		const FUMGStateConfigState* State = FindState(Group, ActiveGroupRuntime->ActiveStateName);
		if (!State)
		{
			continue;
		}

		bool bConflicts = false;
		for (const FUMGStatePropertyChange& Change : State->PropertyChanges)
		{
			const FUMGStateConfigChangeKey ChangeKey{ Change.TargetWidgetName, Change.PropertyType, Change.Value.SerializedPropertyPath };
			if (TargetTouchedKeys.Contains(ChangeKey))
			{
				bConflicts = true;
				break;
			}
		}
		if (bConflicts)
		{
			AffectedGroups.Add(&Group);
		}
	}

	AffectedGroups.Sort([](const FUMGStateConfigGroup& A, const FUMGStateConfigGroup& B)
	{
		return A.Priority < B.Priority;
	});

	bool bAllChangesSucceeded = true;
	for (const FUMGStateConfigGroup* Group : AffectedGroups)
	{
		FUMGActiveStateGroupRuntime* ActiveGroupRuntime = ActiveStateGroups.Find(Group->GroupName);
		if (!ActiveGroupRuntime)
		{
			continue;
		}

		const FUMGStateConfigState* State = FindState(*Group, ActiveGroupRuntime->ActiveStateName);
		if (!State)
		{
			UE_LOG(LogUMGStateConfig, Warning, TEXT("Active state not found during reapply: Group=%s, State=%s"),
				*Group->GroupName.ToString(),
				*ActiveGroupRuntime->ActiveStateName.ToString());
			bAllChangesSucceeded = false;
			continue;
		}

		for (const FUMGStatePropertyChange& Change : State->PropertyChanges)
		{
			bAllChangesSucceeded &= ApplyPropertyChange(TargetUserWidget, Change, *ActiveGroupRuntime);
		}
	}
	return bAllChangesSucceeded;
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

const FUMGStateConfigState* UUMGStateConfigUserWidgetExtension::FindState(const FUMGStateConfigGroup& Group, FName StateName)
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

		TMap<FName, int32> StateIndexByName;
		for (int32 StateIndex = 0; StateIndex < Group.States.Num(); ++StateIndex)
		{
			StateIndexByName.Add(Group.States[StateIndex].StateName, StateIndex);
		}
		StateIndexByGroupName.Add(Group.GroupName, MoveTemp(StateIndexByName));
	}

	bLookupCacheBuilt = true;
}

void UUMGStateConfigUserWidgetExtension::InvalidateLookupCache()
{
	StateGroupIndexByName.Reset();
	StateIndexByGroupName.Reset();
	WidgetCache.Reset();
	bLookupCacheBuilt = false;
}

void UUMGStateConfigUserWidgetExtension::NormalizeRuntimeData()
{
	if (RuntimeData.SchemaVersion <= 0)
	{
		RuntimeData.SchemaVersion = 1;
	}
	if (RuntimeData.SchemaVersion < GUMGStateConfigCurrentSchemaVersion)
	{
		RuntimeData.SchemaVersion = GUMGStateConfigCurrentSchemaVersion;
	}
}

void UUMGStateConfigUserWidgetExtension::PreloadReferencedAssets(bool bAsync)
{
	// 收集所有引用资产，一次性批量加载
	TArray<FSoftObjectPath> AllAssets;
	for (const FUMGStateConfigGroup& Group : RuntimeData.StateGroups)
	{
		for (const FUMGStateConfigState& State : Group.States)
		{
			for (const FUMGStatePropertyChange& Change : State.PropertyChanges)
			{
				AllAssets.Append(Change.Value.SerializedReferencedAssets);
			}
		}
	}
	FUMGStateConfigPropertyRuntimeLibrary::PreloadReferencedAssets(AllAssets, bAsync);
}

void UUMGStateConfigUserWidgetExtension::RestoreGlobalValues(UUserWidget* TargetUserWidget)
{
	if (!TargetUserWidget)
	{
		return;
	}

	for (const TPair<FUMGStateConfigChangeKey, FUMGStateConfigPropertyValue>& RestorePair : GlobalRestoreValues)
	{
		UWidget* TargetWidget = ResolveWidget(TargetUserWidget, RestorePair.Key.TargetWidgetName);
		if (!TargetWidget)
		{
			continue;
		}

		if (FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(TargetWidget, RestorePair.Key.PropertyType, RestorePair.Value, false))
		{
			QueueWidgetRefresh(TargetWidget);
		}
		else
		{
			UE_LOG(LogUMGStateConfig, Warning, TEXT("Restore property value failed: Widget=%s, PropertyPath=%s"),
				*RestorePair.Key.TargetWidgetName.ToString(),
				*RestorePair.Key.SerializedPropertyPath);
		}

	}

	GlobalRestoreValues.Empty();
}

void UUMGStateConfigUserWidgetExtension::QueueWidgetRefresh(UWidget* Widget)
{
	if (!Widget)
	{
		return;
	}

	for (const TWeakObjectPtr<UWidget>& Existing : PendingRefreshWidgets)
	{
		if (Existing.Get() == Widget)
		{
			return;
		}
	}
	PendingRefreshWidgets.Add(Widget);
}

void UUMGStateConfigUserWidgetExtension::FlushPendingWidgetRefreshes()
{
	for (const TWeakObjectPtr<UWidget>& WidgetPtr : PendingRefreshWidgets)
	{
		if (UWidget* Widget = WidgetPtr.Get())
		{
			Widget->SynchronizeProperties();
			Widget->InvalidateLayoutAndVolatility();
		}
	}
	PendingRefreshWidgets.Reset();
}





