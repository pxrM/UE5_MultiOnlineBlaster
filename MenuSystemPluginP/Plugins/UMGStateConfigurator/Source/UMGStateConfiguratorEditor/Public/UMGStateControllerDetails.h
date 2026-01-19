// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyHandle.h"
#include "Widgets/Input/SComboBox.h"

class FUIPropertyOverrideCustomization : public IPropertyTypeCustomization
{
public:
	// 创建实例的静态方法，供注册时调用
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	/** IPropertyTypeCustomization 接口 */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	/** 刷新指定控件的可选属性列表 */
	void RefreshPropertyOptions(class UWidget* InWidget);

	/** 内部句柄 */
	TSharedPtr<IPropertyHandle> WidgetNameHandle;
	TSharedPtr<IPropertyHandle> PropNameHandle;

	/** 下拉框的数据源 */
	TArray<TSharedPtr<FString>> WidgetNameOptions;
	TArray<TSharedPtr<FString>> PropertyNameOptions;

	/** 属性下拉框的指针，用于动态刷新 */
	TSharedPtr<SComboBox<TSharedPtr<FString>>> PropertyComboBox;
};
