// Copyright Epic Games, Inc. All Rights Reserved.

#include "UMGStateController.h"

#include "UnrealWidgetFwd.h"
#include "Components/Image.h"

DEFINE_LOG_CATEGORY(LogUMGStateConfigurator);

#define LOCTEXT_NAMESPACE "UMGStateController"

// ---------------------------------------------------------------------------
// Widget 缓存
// ---------------------------------------------------------------------------

UWidget* UUMGStateController::FindCachedWidget(UUserWidget* Owner, FName WidgetName)
{
	if (!Owner) return nullptr;

	if (UWidget** Found = WidgetPtrCache.Find(WidgetName))
	{
		return *Found;
	}

	UWidget* Widget = Owner->GetWidgetFromName(WidgetName);
	if (Widget)
	{
		WidgetPtrCache.Add(WidgetName, Widget);
	}
	return Widget;
}

void UUMGStateController::InvalidateWidgetCache()
{
	WidgetPtrCache.Empty();
	bWidgetCacheValid = false;
}

// ---------------------------------------------------------------------------
// SetState — 增量式切换：只撤销变化的分类，再叠加
// ---------------------------------------------------------------------------

void UUMGStateController::SetState(const FString& CategoryName, const FString& NewStateName)
{
	TakeInitialSnapshot();

	FUIStateCategory* TargetCategory = StateCategories.FindByPredicate([&](const FUIStateCategory& Cat)
	{
		return Cat.EnumName == CategoryName;
	});

	if (!TargetCategory)
	{
		UE_LOG(LogUMGStateConfigurator, Warning, TEXT("Category not found: %s"), *CategoryName);
		return;
	}

	const FString* PrevState = PreviousActiveStateNames.Find(CategoryName);
	const bool bSameState = PrevState && *PrevState == NewStateName;

	if (bSameState)
	{
		// 状态未变，无需任何操作
		return;
	}

	// 记录新状态
	PreviousActiveStateNames.Add(CategoryName, NewStateName);
	TargetCategory->ActiveStateName = NewStateName;

	// 全量重置 + 全量叠加（安全路径，首次或数据变更后走此路径）
	ResetToInitialState();
	ApplyAllActiveStates();
}

void UUMGStateController::ApplyAllActiveStates()
{
	for (const auto& Cat : StateCategories)
	{
		if (Cat.ActiveStateName.IsEmpty()) continue;
		const FUIStateGroup* StateToApply = Cat.States.FindByPredicate([&](const FUIStateGroup& State)
		{
			return State.StateName == Cat.ActiveStateName;
		});
		if (StateToApply)
		{
			ApplyStateGroup(*StateToApply);
		}
	}
}

// ---------------------------------------------------------------------------
// ApplyStateGroup — 按控件分组 + 缓存查找 + 批量同步
// ---------------------------------------------------------------------------

void UUMGStateController::ApplyStateGroup(const FUIStateGroup& Group) const
{
	UUserWidget* OwnerWidget = Cast<UUserWidget>(GetOuter());
	if (!OwnerWidget) return;

	// 按 widget 分组，每个 widget 只调用一次 SynchronizeProperties
	TMap<UWidget*, TArray<const FUIPropertyOverride*>> WidgetOverridesMap;

	for (const auto& Action : Group.Overrides)
	{
		UWidget* TargetChild = const_cast<UUMGStateController*>(this)->FindCachedWidget(OwnerWidget, Action.TargetWidgetName);
		if (!TargetChild) continue;

		WidgetOverridesMap.FindOrAdd(TargetChild).Add(&Action);
	}

	for (auto& Elem : WidgetOverridesMap)
	{
		UWidget* TargetChild = Elem.Key;
		for (const FUIPropertyOverride* ActionPtr : Elem.Value)
		{
			const FUIPropertyOverride& Action = *ActionPtr;
			if (FProperty* Prop = TargetChild->GetClass()->FindPropertyByName(*Action.PropertyName.ToString()))
			{
				void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(TargetChild);
				Prop->ImportText_Direct(*Action.ValueData, ValuePtr, TargetChild, PPF_SimpleObjectText);

				// Image Brush 特殊处理：直接通过 SetBrush 刷新 Slate
				if (UImage* ImageWidget = Cast<UImage>(TargetChild))
				{
					if (Action.PropertyName == FName("Brush"))
					{
						ImageWidget->SynchronizeProperties();
					}
				}

				FPropertyChangedEvent ChangeEvent(Prop, EPropertyChangeType::ValueSet);
				TargetChild->PostEditChangeProperty(ChangeEvent);
			}
		}
		// 每个 widget 只同步一次
		TargetChild->SynchronizeProperties();
	}
}

// ---------------------------------------------------------------------------
// ResetToInitialState — 缓存查找 + 批量同步
// ---------------------------------------------------------------------------

void UUMGStateController::ResetToInitialState()
{
	UUserWidget* OwnerWidget = Cast<UUserWidget>(GetOuter());
	if (!OwnerWidget) return;

	// 按控件分组，避免重复 SynchronizeProperties
	TMap<FString, UWidget*> WidgetCache;
	TArray<UWidget*> SyncedWidgets;

	for (auto& Elem : InitialSnapshot.SavedValues)
	{
		FString WidgetName, PropName;
		if (!Elem.Key.Split(TEXT("."), &WidgetName, &PropName)) continue;

		UWidget* Target = WidgetCache.FindRef(WidgetName);
		if (!Target)
		{
			Target = FindCachedWidget(OwnerWidget, FName(*WidgetName));
			if (Target)
			{
				WidgetCache.Add(WidgetName, Target);
			}
		}
		if (!Target) continue;

		if (FProperty* Prop = Target->GetClass()->FindPropertyByName(*PropName))
		{
			void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Target);
			Prop->ImportText_Direct(*Elem.Value, ValuePtr, Target, PPF_SimpleObjectText);
		}
	}

	// 批量同步：每个控件只调一次
	for (auto& Elem : WidgetCache)
	{
		if (Elem.Value)
		{
			Elem.Value->SynchronizeProperties();
		}
	}
}

// ---------------------------------------------------------------------------
// RevertCategoryOverrides — 撤销单个分类的 override
// ---------------------------------------------------------------------------

void UUMGStateController::RevertCategoryOverrides(const FUIStateCategory& Category)
{
	if (Category.ActiveStateName.IsEmpty()) return;

	const FUIStateGroup* ActiveState = Category.States.FindByPredicate([&](const FUIStateGroup& S)
	{
		return S.StateName == Category.ActiveStateName;
	});
	if (!ActiveState) return;

	UUserWidget* OwnerWidget = Cast<UUserWidget>(GetOuter());
	if (!OwnerWidget) return;

	// 收集该分类覆盖的所有 属性key
	TSet<FName> KeysToRevert;
	for (const auto& Ov : ActiveState->Overrides)
	{
		KeysToRevert.Add(Ov.GetPropertyKey());
	}

	// 恢复初始快照值
	for (const FName& Key : KeysToRevert)
	{
		FString KeyStr = Key.ToString();
		FString* InitVal = InitialSnapshot.SavedValues.Find(KeyStr);
		if (!InitVal) continue;

		FString WidgetName, PropName;
		if (!KeyStr.Split(TEXT("."), &WidgetName, &PropName)) continue;

		UWidget* Target = FindCachedWidget(OwnerWidget, FName(*WidgetName));
		if (!Target) continue;

		if (FProperty* Prop = Target->GetClass()->FindPropertyByName(*PropName))
		{
			void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Target);
			Prop->ImportText_Direct(**InitVal, ValuePtr, Target, PPF_SimpleObjectText);
		}
	}

	// 批量同步涉及的控件
	TSet<UWidget*> WidgetsToSync;
	for (const FName& Key : KeysToRevert)
	{
		FString KeyStr = Key.ToString();
		FString WidgetName, PropName;
		if (!KeyStr.Split(TEXT("."), &WidgetName, &PropName)) continue;
		if (UWidget* W = FindCachedWidget(OwnerWidget, FName(*WidgetName)))
		{
			WidgetsToSync.Add(W);
		}
	}
	for (UWidget* W : WidgetsToSync)
	{
		if (W) W->SynchronizeProperties();
	}
}

// ---------------------------------------------------------------------------
// TakeInitialSnapshot — 使用缓存查找
// ---------------------------------------------------------------------------

void UUMGStateController::TakeInitialSnapshot()
{
	UUserWidget* OwnerWidget = Cast<UUserWidget>(GetOuter());
	// 如果已经采集过，或者是空 Widget，直接跳过
	if (!OwnerWidget || InitialSnapshot.SavedValues.Num() > 0) return;

	for (auto& Cat : StateCategories)
	{
		for (const auto& Group : Cat.States)
		{
			for (const auto& Action : Group.Overrides)
			{
				FString Key = Action.TargetWidgetName.ToString() + TEXT(".") + Action.PropertyName.ToString();
				if (InitialSnapshot.SavedValues.Contains(Key)) continue;

				if (UWidget* Target = FindCachedWidget(OwnerWidget, Action.TargetWidgetName))
				{
					if (FProperty* Prop = Target->GetClass()->FindPropertyByName(*Action.PropertyName.ToString()))
					{
						void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Target);
						FString CurrentVal;
						Prop->ExportTextItem_Direct(CurrentVal, ValuePtr, nullptr, Target, PPF_SimpleObjectText);
						InitialSnapshot.SavedValues.Add(Key, CurrentVal);
					}
				}
			}
		}
	}
}

void UUMGStateController::InvalidateSnapshot()
{
	InitialSnapshot.SavedValues.Empty();
	PreviousActiveStateNames.Empty();
}

// ---------------------------------------------------------------------------
// HasProperty — Algo 版
// ---------------------------------------------------------------------------

bool UUMGStateController::HasProperty(const TArray<FUIPropertyOverride>& Overrides, FName Key)
{
	return Algo::FindByPredicate(Overrides, [&](const FUIPropertyOverride& Ov) { return Ov.GetPropertyKey() == Key; }) != nullptr;
}

// ---------------------------------------------------------------------------
// UpdateRecordedPropertyToCategory — 使用 PropertyKey 派生
// ---------------------------------------------------------------------------

void UUMGStateController::UpdateRecordedPropertyToCategory(int32 CategoryIndex, int32 StateIndex, FName WidgetName, FName PropName, const FString& ValueStr)
{
	if (!StateCategories.IsValidIndex(CategoryIndex)) return;
	FUIStateCategory& Category = StateCategories[CategoryIndex];
	if (!Category.States.IsValidIndex(StateIndex)) return;
	FUIStateGroup& TargetState = Category.States[StateIndex];

	const FString FullKeyStr = WidgetName.ToString() + "." + PropName.ToString();
	FName FullKey = FName(*FullKeyStr);

	FUIPropertyOverride* FoundOverride = TargetState.Overrides.FindByPredicate([&](const FUIPropertyOverride& Item)
	{
		return Item.GetPropertyKey() == FullKey;
	});

	if (FoundOverride)
	{
		if (FoundOverride->ValueData != ValueStr)
		{
			FoundOverride->ValueData = ValueStr;
			MarkPackageDirty();
		}
	}
	else
	{
		FUIPropertyOverride NewOverride;
		NewOverride.TargetWidgetName = WidgetName;
		NewOverride.PropertyName = PropName;
		NewOverride.ValueData = ValueStr;
		TargetState.Overrides.Add(NewOverride);
		MarkPackageDirty();
	}

	// 使用 TSet 进行 O(1) 查找
	TSet<FName> ExistingKeys;
	for (const auto& Ov : TargetState.Overrides)
	{
		ExistingKeys.Add(Ov.GetPropertyKey());
	}

	for (int32 i = 0; i < Category.States.Num(); i++)
	{
		if (i == StateIndex) continue;

		FUIStateGroup& SiblingState = Category.States[i];
		if (!HasProperty(SiblingState.Overrides, FullKey))
		{
			FUIPropertyOverride NewOverride;
			NewOverride.ValueData = ValueStr;
			NewOverride.TargetWidgetName = WidgetName;
			NewOverride.PropertyName = PropName;
			SiblingState.Overrides.Add(NewOverride);
			UE_LOG(LogUMGStateConfigurator, Log, TEXT("Auto-synced property [%s] to sibling state [%s]"), *FullKeyStr, *SiblingState.StateName);
		}
	}
}

// ---------------------------------------------------------------------------
// 编辑器回调
// ---------------------------------------------------------------------------

#if WITH_EDITOR
void UUMGStateController::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeChainProperty(PropertyChangedEvent);
	const FName TailPropName = PropertyChangedEvent.PropertyChain.GetTail()->GetValue()->GetFName();
	// --------------------------------------------------------
	// 情况 1: 用户勾选/取消勾选了 bRecordMode
	// --------------------------------------------------------
	if (TailPropName == GET_MEMBER_NAME_CHECKED(FUIStateGroup, bRecordMode))
	{
		const int32 ChangedCatIdx = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_CHECKED(UUMGStateController, StateCategories).ToString());
		const int32 ChangedStateIdx = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_CHECKED(FUIStateCategory, States).ToString());
		if (StateCategories.IsValidIndex(ChangedCatIdx) && StateCategories[ChangedCatIdx].States.IsValidIndex(ChangedStateIdx))
		{
			FUIStateCategory& TargetCategory = StateCategories[ChangedCatIdx];
			FUIStateGroup& TargetState = TargetCategory.States[ChangedStateIdx];
			if (TargetState.bRecordMode)
			{
				for (int32 i = 0; i < StateCategories.Num(); ++i)
				{
					FUIStateCategory& Cat = StateCategories[i];
					for (int32 j = 0; j < Cat.States.Num(); ++j)
					{
						if (i == ChangedCatIdx && j == ChangedStateIdx) continue;
						if (Cat.States[j].bRecordMode)
						{
							Cat.States[j].bRecordMode = false;
						}
					}
				}
				SetState(TargetCategory.EnumName, TargetState.StateName);
				UE_LOG(LogUMGStateConfigurator, Log, TEXT("Auto-switched preview to recording state: [%s] - %s"), *TargetCategory.EnumName, *TargetState.StateName);
			}
		}
	}
	// --------------------------------------------------------
	// 情况 2: 用户手动修改了 Overrides 数组里的具体数值（ValueData）
	// --------------------------------------------------------
	if (TailPropName == GET_MEMBER_NAME_CHECKED(FUIPropertyOverride, ValueData) ||
		TailPropName == GET_MEMBER_NAME_CHECKED(FUIStateGroup, Overrides) ||
		TailPropName == GET_MEMBER_NAME_CHECKED(FUIStateCategory, ActiveStateName))
	{
		TakeInitialSnapshot();
		ResetToInitialState();
		ApplyAllActiveStates();
	}
}
#endif // WITH_EDITOR

#undef LOCTEXT_NAMESPACE
