// Copyright Epic Games, Inc. All Rights Reserved.

#include "UMGStateControllerDetails.h"

#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "UMGStateController.h" // 引用 Runtime 模块的数据结构

#define LOCTEXT_NAMESPACE "FUIPropertyOverrideCustomization"

TSharedRef<IPropertyTypeCustomization> FUIPropertyOverrideCustomization::MakeInstance()
{
	return MakeShareable(new FUIPropertyOverrideCustomization());
}

void FUIPropertyOverrideCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// 让 Header 显示默认的结构体名称或内容
	HeaderRow.NameContent()[PropertyHandle->CreatePropertyNameWidget()];
}

void FUIPropertyOverrideCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// 1. 获取子属性句柄
	WidgetNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIPropertyOverride, TargetWidgetName));
	PropNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIPropertyOverride, PropertyName));
	TSharedPtr<IPropertyHandle> ValueDataHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FUIPropertyOverride, ValueData));

	// 2. 获取当前编辑的 UMG 预览对象
	UUserWidget* PreviewWidget = nullptr;
	TArray<UObject*> Outers;
	PropertyHandle->GetOuterObjects(Outers);
	if (Outers.Num() > 0 && Outers[0])
	{
		// 逻辑：FUIPropertyOverride -> FUIStateGroup -> UUMGStateController -> UserWidget (Outer)
		if (UUMGStateController* Controller = Cast<UUMGStateController>(Outers[0]))
		{
			PreviewWidget = Cast<UUserWidget>(Controller->GetOuter());
		}
	}

	// 3. 填充控件名称列表
	WidgetNameOptions.Empty();
	if (PreviewWidget && PreviewWidget->WidgetTree)
	{
		PreviewWidget->WidgetTree->ForEachWidget([&](UWidget* W)
		{
			WidgetNameOptions.Add(MakeShared<FString>(W->GetName()));
		});
	}

	// --- 构建 UI ---

	// 第一行：Widget 选择器
	ChildBuilder.AddCustomRow(LOCTEXT("WidgetPickerRow", "Target Widget"))
	            .NameContent()[SNew(STextBlock).Text(LOCTEXT("TargetWidget", "Target Widget")).Font(IDetailLayoutBuilder::GetDetailFont())]
		.ValueContent()
		[
			SNew(SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&WidgetNameOptions)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) { return SNew(STextBlock).Text(FText::FromString(*InItem)); })
			.OnSelectionChanged_Lambda([this, PreviewWidget, PropertyHandle](TSharedPtr<FString> Selection, ESelectInfo::Type)
			{
				if (Selection.IsValid())
				{
					WidgetNameHandle->SetValue(*Selection);
					// 联动：当控件改变时，刷新属性列表
					if (PreviewWidget)
					{
						UWidget* SelectedWidget = PreviewWidget->GetWidgetFromName(FName(**Selection));
						RefreshPropertyOptions(SelectedWidget);
					}
					if (Selection.IsValid())
					{
						WidgetNameHandle->SetValue(*Selection);
						// 关键：通知底层对象数据已变动，触发 PostEditChangeProperty
						PropertyHandle->NotifyFinishedChangingProperties();
					}
				}
			})
			[
				SNew(STextBlock).Text_Lambda([this]()
				{
					FName V;
					WidgetNameHandle->GetValue(V);
					return V.IsNone() ? LOCTEXT("SelectWidget", "Select Widget...") : FText::FromName(V);
				})
			]
		];

	// 第二行：Property 选择器
	ChildBuilder.AddCustomRow(LOCTEXT("PropertyPickerRow", "Property"))
	            .NameContent()[SNew(STextBlock).Text(LOCTEXT("Property", "Property (Type)")).Font(IDetailLayoutBuilder::GetDetailFont())]
		.ValueContent()
		[
			SAssignNew(PropertyComboBox, SComboBox<TSharedPtr<FString>>)
			.OptionsSource(&PropertyNameOptions)
			.OnGenerateWidget_Lambda([](TSharedPtr<FString> InItem) { return SNew(STextBlock).Text(FText::FromString(*InItem)); })
			.OnSelectionChanged_Lambda([this](TSharedPtr<FString> Selection, ESelectInfo::Type)
			{
				if (Selection.IsValid())
				{
					FString PureName, TypePart;
					Selection->Split(TEXT(" ("), &PureName, &TypePart);
					PropNameHandle->SetValue(PureName.IsEmpty() ? *Selection : PureName);
				}
			})
			[
				SNew(STextBlock).Text_Lambda([this]()
				{
					FString V;
					PropNameHandle->GetValue(V);
					return V.IsEmpty() ? LOCTEXT("SelectProp", "Select Property...") : FText::FromString(V);
				})
			]
		];

	// 第三行：原始数据输入（用于填写具体的值）
	ChildBuilder.AddProperty(ValueDataHandle.ToSharedRef());
}

void FUIPropertyOverrideCustomization::RefreshPropertyOptions(UWidget* InWidget)
{
	PropertyNameOptions.Empty();
	if (InWidget)
	{
		for (TFieldIterator<FProperty> It(InWidget->GetClass()); It; ++It)
		{
			FProperty* Prop = *It;
			if (Prop->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
			{
				FString Display = FString::Printf(TEXT("%s (%s)"), *Prop->GetName(), *Prop->GetCPPType());
				PropertyNameOptions.Add(MakeShared<FString>(Display));
			}
		}
	}

	if (PropertyComboBox.IsValid())
	{
		PropertyComboBox->RefreshOptions();
	}
}

#undef LOCTEXT_NAMESPACE
