#include "UMGStateController.h"

void UUMGStateController::SetControllerState(FString NewStateName)
{
	// 1. 先采集快照（如果还没采集的话）
	TakeInitialSnapshot();

	// 2. 先全部还原到初始状态（防止状态污染）
	ResetToInitialState();

	// 3. 如果名字为空，直接返回（实现“清空状态”即恢复默认）
	if (NewStateName.IsEmpty()) 
	{
		ActiveStateName = TEXT("");
		return;
	}

	// 4. 应用新状态
	const int32 FoundIndex = StateGroups.IndexOfByPredicate([&](const FUIStateGroup& Item) {
		return Item.StateName == NewStateName;
	});

	if (FoundIndex != INDEX_NONE)
	{
		ApplyState(FoundIndex);
		CurrentState = NewStateName;
	}
}

void UUMGStateController::ApplyState(int32 Index)
{
	UUserWidget* OwnerWidget = Cast<UUserWidget>(GetOuter());
	if (!OwnerWidget || !StateGroups.IsValidIndex(Index)) return;

	for (const FUIPropertyOverride& Action : StateGroups[Index].Overrides)
	{
		UWidget* TargetChild = OwnerWidget->GetWidgetFromName(Action.TargetWidgetName);
		if (!TargetChild) continue;

		if (FProperty* Prop = TargetChild->GetClass()->FindPropertyByName(*Action.PropertyName))
		{
			// ValuePtr：属性值的原始指针，ContainerPtrToValuePtr：获取该属性在对象实例中的内存地址，TargetChild：包含此属性的对象实例
			void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(TargetChild);
			// 从文本设置属性值，ImportText_Direct()：将字符串转换为属性值并写入内存
			// *Action.ValueData：包含属性值的文本字符串
			// ValuePtr：要写入的内存地址
			// TargetChild：属性所有者（用于解析引用等）
			Prop->ImportText_Direct(*Action.ValueData, ValuePtr, TargetChild, 0);
			TargetChild->SynchronizeProperties(); // 强制刷新 UMG 表现
		}
	}
}

void UUMGStateController::ResetToInitialState()
{
	UUserWidget* OwnerWidget = Cast<UUserWidget>(GetOuter());
	if (!OwnerWidget) return;

	for (auto& Elem : InitialSnapshot.SavedValues)
	{
		FString WidgetName, PropName;
		Elem.Key.Split(TEXT("."), &WidgetName, &PropName);

		UWidget* Target = OwnerWidget->GetWidgetFromName(FName(*WidgetName));
		if (Target)
		{
			FProperty* Prop = Target->GetClass()->FindPropertyByName(*PropName);
			if (Prop)
			{
				void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Target);
				Prop->ImportText_Direct(*Elem.Value, ValuePtr, Target, 0);
				Target->SynchronizeProperties();
			}
		}
	}
}

void UUMGStateController::TakeInitialSnapshot()
{
	UUserWidget* OwnerWidget = Cast<UUserWidget>(GetOuter());
	if (!OwnerWidget || InitialSnapshot.SavedValues.Num() > 0) return; // 只采集一次

	for (const FUIStateGroup& Group : StateGroups)
	{
		for (const FUIPropertyOverride& Action : Group.Overrides)
		{
			FString Key = Action.TargetWidgetName.ToString() + TEXT(".") + Action.PropertyName;
			if (InitialSnapshot.SavedValues.Contains(Key)) continue;

			UWidget* Target = OwnerWidget->GetWidgetFromName(Action.TargetWidgetName);
			if (Target)
			{
				FProperty* Prop = Target->GetClass()->FindPropertyByName(*Action.PropertyName);
				if (Prop)
				{
					void* ValuePtr = Prop->ContainerPtrToValuePtr<void>(Target);
					FString CurrentVal;
					Prop->ExportTextItem_Direct(CurrentVal, ValuePtr, nullptr, Target, 0);
					InitialSnapshot.SavedValues.Add(Key, CurrentVal);
				}
			}
		}
	}
}

void UUMGStateController::UpdateRecordedPropertyToGroup(FUIStateGroup& TargetGroup, FName TargetWidgetName, FName PropertyName, const FString& ValueStr)
{
	FUIPropertyOverride* FoundOverride = TargetGroup.Overrides.FindByPredicate([&](const FUIPropertyOverride& Item)
	{
		return Item.PropertyName == TargetWidgetName && Item.PropertyName == PropertyName.ToString();
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
		NewOverride.TargetWidgetName = TargetWidgetName;
		NewOverride.PropertyName = PropertyName.ToString();
		NewOverride.ValueData = ValueStr;
		TargetGroup.Overrides.Add(NewOverride);
		MarkPackageDirty();
	}
}

void UUMGStateController::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UObject::PostEditChangeProperty(PropertyChangedEvent);

	if (!PropertyChangedEvent.Property) return; 

	const FName PropertyName = PropertyChangedEvent.Property->GetFName();

	if (PropertyName == GET_MEMBER_NAME_CHECKED(FUIStateGroup, bRecordMode))
	{
		int32 Index = PropertyChangedEvent.GetArrayIndex(GET_MEMBER_NAME_CHECKED(UUMGStateController, StateGroups).ToString());
		if (StateGroups.IsValidIndex(Index) && StateGroups[Index].bRecordMode)
		{
			ActiveStateName = StateGroups[Index].StateName;
			SetControllerState(ActiveStateName);
			UE_LOG(LogTemp, Log, TEXT("Auto-switched preview to recording state: %s"), *ActiveStateName);
		}
	}
	// 当我们修改 ActiveStateName 或者 StateGroups 数组里的内容时，立即预览
	// if (PropertyName == GET_MEMBER_NAME_CHECKED(UUMGStateController, ActiveStateName) ||
	// 	PropertyName == GET_MEMBER_NAME_CHECKED(FUIStateGroup, Overrides))
	// {
	// 	SetControllerState(ActiveStateName);
	// }
}
