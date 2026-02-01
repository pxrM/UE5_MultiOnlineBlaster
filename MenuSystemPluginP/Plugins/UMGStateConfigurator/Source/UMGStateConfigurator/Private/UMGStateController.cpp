#include "UMGStateController.h"

#include "UnrealWidgetFwd.h"
#include "Components/Image.h"

void UUMGStateController::SetState(FString CategoryName, FString NewStateName)
{
	TakeInitialSnapshot();
	FUIStateCategory* TargetCategory = StateCategories.FindByPredicate([&](const FUIStateCategory& Cat)
	{
		return Cat.EnumName == CategoryName;
	});

	if (!TargetCategory)
	{
		UE_LOG(LogTemp, Warning, TEXT("Category not found: %s"), *CategoryName);
		return;
	}

	// 3. 更新该分类的“当前活跃状态”
	// (我们需要在 FUIStateCategory 结构体里加一个 ActiveStateName 成员变量来记录当前选中的是啥)
	TargetCategory->ActiveStateName = NewStateName; 

	// 4. 【核心逻辑】刷新整个 UI
	// 因为多个枚举可能控制同一个 Widget 的不同属性，或者互斥属性
	// 最安全的做法是：先全部重置，再按顺序叠加所有分类的当前状态
	ResetToInitialState();
	ApplyAllActiveStates();
}

void UUMGStateController::ApplyAllActiveStates()
{
	for (const auto& Cat:StateCategories)
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

void UUMGStateController::ApplyStateGroup(const FUIStateGroup& Group) const
{
	UUserWidget* OwnerWidget = Cast<UUserWidget>(GetOuter());
	if (!OwnerWidget) return;

	for (const auto& Action : Group.Overrides)
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
			Prop->ImportText_Direct(*Action.ValueData, ValuePtr, TargetChild, PPF_SimpleObjectText);
			if (UImage* ImageWidget = Cast<UImage>(TargetChild))
			{
				if (*Action.PropertyName == FName("Brush"))
				{
					// 骗过 Slate 的指针检查机制：先设空，再设回
					FSlateBrush NewBrush = ImageWidget->GetBrush();
					ImageWidget->SetBrushFromTexture(nullptr); 
					ImageWidget->SetBrush(NewBrush);
				}
			}
			FPropertyChangedEvent ChangeEvent(Prop, EPropertyChangeType::ValueSet);
			TargetChild->PostEditChangeProperty(ChangeEvent);
			TargetChild->SynchronizeProperties();
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
		if (!Elem.Key.Split(TEXT("."), &WidgetName, &PropName)) continue;
		if (UWidget* Target = OwnerWidget->GetWidgetFromName(FName(*WidgetName)))
		{
			if (FProperty* Prop = Target->GetClass()->FindPropertyByName(*PropName))
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
	// 如果已经采集过，或者是空 Widget，直接跳过
	if (!OwnerWidget || InitialSnapshot.SavedValues.Num() > 0) return;

	for (auto& Cat : StateCategories)
	{
		for (const auto& Group : Cat.States)
		{
			for (const auto& Action : Group.Overrides)
			{
				FString Key = Action.TargetWidgetName.ToString() + TEXT(".") + Action.PropertyName;
				if (InitialSnapshot.SavedValues.Contains(Key)) continue;
			
				if (UWidget* Target = OwnerWidget->GetWidgetFromName(Action.TargetWidgetName))
				{
					if (FProperty* Prop = Target->GetClass()->FindPropertyByName(*Action.PropertyName))
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

bool UUMGStateController::HasProperty(const TArray<FUIPropertyOverride>& Overrides, FName Key)
{
	for (const auto& Ov: Overrides)
	{
		if (Ov.PropertyKey == Key) return true;
	}
	return false;
}

void UUMGStateController::UpdateRecordedPropertyToCategory(int32 CategoryIndex, int32 StateIndex, FName WidgetName, FName PropName, FString ValueStr)
{
	if (!StateCategories.IsValidIndex(CategoryIndex)) return;
	FUIStateCategory& Category = StateCategories[CategoryIndex];
	if (!Category.States.IsValidIndex(StateIndex)) return;
	FUIStateGroup& TargetState = Category.States[StateIndex];

	const FString FullKeyStr = WidgetName.ToString() + "." + PropName.ToString() ;
	FName FullKey = FName(*FullKeyStr);

	FUIPropertyOverride* FoundOverride = TargetState.Overrides.FindByPredicate([&](const FUIPropertyOverride& Item)
	{
		return Item.PropertyKey == FullKey;
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
		NewOverride.PropertyKey = FullKey;
		NewOverride.TargetWidgetName = WidgetName;
		NewOverride.PropertyName = PropName.ToString();
		NewOverride.ValueData = ValueStr;
		TargetState.Overrides.Add(NewOverride);
		MarkPackageDirty();
	}

	for (int32 i = 0; i < Category.States.Num(); i++)
	{
		if (i == StateIndex) continue;

		FUIStateGroup& SiblingState = Category.States[i];
		if (!HasProperty(SiblingState.Overrides, FullKey))
		{
			FUIPropertyOverride NewOverride;
			NewOverride.PropertyKey = FullKey;
			NewOverride.ValueData = ValueStr; 
			NewOverride.TargetWidgetName = WidgetName;
			NewOverride.PropertyName = PropName.ToString();
			SiblingState.Overrides.Add(NewOverride);
			UE_LOG(LogTemp, Log, TEXT("Auto-synced property [%s] to sibling state [%s]"), *FullKeyStr, *SiblingState.StateName);
		}
	}
}

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
				UE_LOG(LogTemp, Log, TEXT("Auto-switched preview to recording state: [%s] - %s"), *TargetCategory.EnumName, *TargetState.StateName);
			}
		}
	}
	// --------------------------------------------------------
	// 情况 2: 用户手动修改了 Overrides 数组里的具体数值（ValueData）
	// --------------------------------------------------------
	// 或者是修改了 StateName 等其他属性
	if (TailPropName == GET_MEMBER_NAME_CHECKED(FUIPropertyOverride, ValueData) ||
		TailPropName == GET_MEMBER_NAME_CHECKED(FUIStateGroup, Overrides) ||
		TailPropName == GET_MEMBER_NAME_CHECKED(FUIStateCategory, ActiveStateName))
	{
		// 为了看到修改效果，我们需要重新刷新一遍当前的所有状态
		// 这里复用 SetState 里的逻辑：重置 + 应用
		TakeInitialSnapshot();
		ResetToInitialState();
		ApplyAllActiveStates();
	}
}

