#include "UMGStateConfigValidator.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "UMGStateConfigBlueprintExtension.h"
#include "UMGStateConfigData.h"
#include "WidgetBlueprint.h"

namespace

{
FText GetPropertyTypeText(EUMGStateConfigPropertyType PropertyType)
{
	const UEnum* PropertyEnum = StaticEnum<EUMGStateConfigPropertyType>();
	return PropertyEnum
		? FText::FromString(PropertyEnum->GetNameStringByValue(static_cast<int64>(PropertyType)))
		: NSLOCTEXT("UMGStateConfigValidator", "UnknownPropertyType", "Unknown");
}

FString MakeChangeKey(const FUMGStatePropertyChange& Change)
{
	return Change.PropertyType == EUMGStateConfigPropertyType::SerializedProperty
		? FString::Printf(TEXT("%s.%d.%s"), *Change.TargetWidgetName.ToString(), static_cast<int32>(Change.PropertyType), *Change.Value.SerializedPropertyPath)
		: FString::Printf(TEXT("%s.%d"), *Change.TargetWidgetName.ToString(), static_cast<int32>(Change.PropertyType));
}

}

void FUMGStateConfigValidator::Validate(const UWidgetBlueprint* WidgetBlueprint, TArray<FText>& OutErrors, TArray<FText>& OutWarnings, TArray<FText>& OutHints)
{
	OutErrors.Reset();
	OutWarnings.Reset();
	OutHints.Reset();

	if (!WidgetBlueprint)
	{
		OutErrors.Add(NSLOCTEXT("UMGStateConfigValidator", "InvalidBlueprint", "当前没有有效的 Widget Blueprint。"));
		return;
	}

	const UUMGStateConfigBlueprintExtension* Extension = UUMGStateConfigBlueprintExtension::Find(WidgetBlueprint);
	if (!Extension)
	{
		OutHints.Add(NSLOCTEXT("UMGStateConfigValidator", "NoConfig", "当前蓝图还没有 UI 状态配置。"));
		return;
	}

	if (Extension->ConfigData.StateGroups.Num() == 0)
	{
		OutWarnings.Add(NSLOCTEXT("UMGStateConfigValidator", "NoStateGroup", "当前配置没有状态组。"));
		return;
	}

	const UWidgetTree* WidgetTree = WidgetBlueprint->WidgetTree;
	if (!WidgetTree)
	{
		OutErrors.Add(NSLOCTEXT("UMGStateConfigValidator", "NoWidgetTree", "当前蓝图没有有效的 WidgetTree。"));
		return;
	}

	TSet<FName> GroupNames;
	TMap<FString, FName> GlobalChangeGroupByKey;
	TSet<FString> ReportedGlobalConflictKeys;
	for (const FUMGStateConfigGroup& Group : Extension->ConfigData.StateGroups)

	{
		if (Group.GroupName.IsNone())
		{
			OutErrors.Add(NSLOCTEXT("UMGStateConfigValidator", "EmptyGroupName", "存在未命名的状态组。"));
		}
		else if (GroupNames.Contains(Group.GroupName))
		{
			OutErrors.Add(FText::Format(
				NSLOCTEXT("UMGStateConfigValidator", "DuplicateGroupName", "状态组名称重复：{0}"),
				FText::FromName(Group.GroupName)));
		}
		GroupNames.Add(Group.GroupName);

		if (Group.States.Num() == 0)
		{
			OutWarnings.Add(FText::Format(
				NSLOCTEXT("UMGStateConfigValidator", "GroupNoStates", "状态组 {0} 没有子状态。"),
				FText::FromName(Group.GroupName)));
			continue;
		}

		TSet<FName> StateNames;
		bool bHasDefaultState = false;
		for (const FUMGStateConfigState& State : Group.States)

		{
			if (State.StateName.IsNone())
			{
				OutErrors.Add(FText::Format(
					NSLOCTEXT("UMGStateConfigValidator", "EmptyStateName", "状态组 {0} 存在未命名的子状态。"),
					FText::FromName(Group.GroupName)));
			}
			else if (StateNames.Contains(State.StateName))
			{
				OutErrors.Add(FText::Format(
					NSLOCTEXT("UMGStateConfigValidator", "DuplicateStateName", "状态组 {0} 的子状态名称重复：{1}"),
					FText::FromName(Group.GroupName),
					FText::FromName(State.StateName)));
			}
			StateNames.Add(State.StateName);

			if (State.StateName == Group.DefaultStateName)
			{
				bHasDefaultState = true;
			}

			for (FName WidgetName : State.ConfiguredWidgetNames)
			{
				if (WidgetName.IsNone())
				{
					OutWarnings.Add(FText::Format(
						NSLOCTEXT("UMGStateConfigValidator", "EmptyConfiguredWidget", "状态 {0}/{1} 存在空控件配置。"),
						FText::FromName(Group.GroupName),
						FText::FromName(State.StateName)));
					continue;
				}
				if (!WidgetTree->FindWidget(WidgetName))
				{
					OutWarnings.Add(FText::Format(
						NSLOCTEXT("UMGStateConfigValidator", "ConfiguredWidgetMissing", "状态 {0}/{1} 配置的控件不存在：{2}"),
						FText::FromName(Group.GroupName),
						FText::FromName(State.StateName),
						FText::FromName(WidgetName)));
				}
			}

			TSet<FString> ChangeKeys;
			for (const FUMGStatePropertyChange& Change : State.PropertyChanges)
			{
				if (Change.TargetWidgetName.IsNone())
				{
					OutErrors.Add(FText::Format(
						NSLOCTEXT("UMGStateConfigValidator", "EmptyChangeWidget", "状态 {0}/{1} 存在目标控件为空的属性配置。"),
						FText::FromName(Group.GroupName),
						FText::FromName(State.StateName)));
					continue;
				}

				const FString ChangeKey = MakeChangeKey(Change);
				if (ChangeKeys.Contains(ChangeKey))
				{
					OutWarnings.Add(FText::Format(
						NSLOCTEXT("UMGStateConfigValidator", "DuplicatePropertyChange", "状态 {0}/{1} 中 {2} 的属性 {3} 重复配置。"),
						FText::FromName(Group.GroupName),
						FText::FromName(State.StateName),
						FText::FromName(Change.TargetWidgetName),
						GetPropertyTypeText(Change.PropertyType)));
				}
				ChangeKeys.Add(ChangeKey);

				if (const FName* ExistingGroupName = GlobalChangeGroupByKey.Find(ChangeKey))
				{
					if (*ExistingGroupName != Group.GroupName && !ReportedGlobalConflictKeys.Contains(ChangeKey))
					{
						OutWarnings.Add(FText::Format(
							NSLOCTEXT("UMGStateConfigValidator", "CrossGroupPropertyConflict", "多个状态组修改同一控件属性：{0}，已出现在 {1} 和 {2}。运行时会按状态组 Priority 和调用顺序重算，建议确认是否符合预期。"),
							FText::FromString(ChangeKey),
							FText::FromName(*ExistingGroupName),
							FText::FromName(Group.GroupName)));
						ReportedGlobalConflictKeys.Add(ChangeKey);
					}
				}
				else
				{
					GlobalChangeGroupByKey.Add(ChangeKey, Group.GroupName);
				}

				const UWidget* TargetWidget = WidgetTree->FindWidget(Change.TargetWidgetName);

				if (!TargetWidget)
				{
					OutWarnings.Add(FText::Format(
						NSLOCTEXT("UMGStateConfigValidator", "ChangeWidgetMissing", "状态 {0}/{1} 的属性配置目标控件不存在：{2}"),
						FText::FromName(Group.GroupName),
						FText::FromName(State.StateName),
						FText::FromName(Change.TargetWidgetName)));
					continue;
				}

				const UClass* ExpectedClass = Change.ExpectedWidgetClass.Get();
				if (ExpectedClass && !TargetWidget->IsA(ExpectedClass))
				{
					OutErrors.Add(FText::Format(
						NSLOCTEXT("UMGStateConfigValidator", "ExpectedClassMismatch", "状态 {0}/{1} 的控件 {2} 类型不匹配，期望 {3}，实际 {4}。"),
						FText::FromName(Group.GroupName),
						FText::FromName(State.StateName),
						FText::FromName(Change.TargetWidgetName),
						FText::FromString(ExpectedClass->GetName()),
						FText::FromString(TargetWidget->GetClass()->GetName())));
				}

				if (!State.ConfiguredWidgetNames.Contains(Change.TargetWidgetName))
				{
					OutHints.Add(FText::Format(
						NSLOCTEXT("UMGStateConfigValidator", "ChangeWidgetNotListed", "状态 {0}/{1} 的控件 {2} 有属性配置但不在 ConfiguredWidgetNames 中，保存时建议补齐。"),
						FText::FromName(Group.GroupName),
						FText::FromName(State.StateName),
						FText::FromName(Change.TargetWidgetName)));
				}


				if (Change.PropertyType == EUMGStateConfigPropertyType::SerializedProperty && Change.Value.SerializedPropertyPath.IsEmpty())
				{
					OutErrors.Add(FText::Format(
						NSLOCTEXT("UMGStateConfigValidator", "SerializedPropertyPathEmpty", "状态 {0}/{1} 的控件 {2} Details 属性路径为空。"),
						FText::FromName(Group.GroupName),
						FText::FromName(State.StateName),
						FText::FromName(Change.TargetWidgetName)));
				}

			}
		}

		if (!bHasDefaultState)
		{
			OutWarnings.Add(FText::Format(
				NSLOCTEXT("UMGStateConfigValidator", "DefaultStateMissing", "状态组 {0} 的默认状态不存在：{1}"),
				FText::FromName(Group.GroupName),
				FText::FromName(Group.DefaultStateName)));
		}
	}
}

