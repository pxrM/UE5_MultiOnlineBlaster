#pragma once

#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "WidgetAnimTimelineSequence.h"

class UWidgetBlueprint;

class FWidgetAnimTimelineEntryCustomization : public IPropertyTypeCustomization
{
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	UWidgetBlueprint* GetWidgetBlueprint() const;
	UClass* ResolveOwnerWidgetClass() const;
	UClass* ResolveTargetWidgetClass(FName TargetWidgetName) const;
	void AddTargetWidgetOption(FName TargetWidgetName);
	void RefreshTargetWidgetOptions();
	void RefreshAnimationOptions();
	void RefreshChildPhaseOptions();
	FName GetOwnerPhaseName() const;
	bool HasOption(const TArray<TSharedPtr<FString>>& Options, const FString& Value) const;

	void OnTargetWidgetSelected(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void OnAnimationSelected(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void OnChildPhaseSelected(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo);
	void OnEntryTypeChanged();

	FText GetCurrentTargetWidgetName() const;
	FText GetCurrentAnimationName() const;
	FText GetCurrentChildPhaseName() const;
	FText GetTargetWidgetValidationText() const;
	FText GetAnimationNameValidationText() const;
	FText GetChildPhaseNameValidationText() const;
	EVisibility GetTargetWidgetValidationVisibility() const;
	EVisibility GetAnimationNameValidationVisibility() const;
	EVisibility GetChildPhaseNameValidationVisibility() const;
	EWidgetAnimTimelineEntryType GetEntryType() const;

	TSharedPtr<IPropertyHandle> StructHandle;
	TSharedPtr<IPropertyHandle> TargetWidgetNameHandle;
	TSharedPtr<IPropertyHandle> EntryTypeHandle;
	TSharedPtr<IPropertyHandle> AnimationNameHandle;
	TSharedPtr<IPropertyHandle> ChildPhaseNameHandle;
	TSharedPtr<IPropertyUtilities> PropertyUtilities;

	TArray<TSharedPtr<FString>> TargetWidgetOptions;
	TArray<TSharedPtr<FString>> AnimationOptions;
	TArray<TSharedPtr<FString>> ChildPhaseOptions;
};
