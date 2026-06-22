#include "WidgetAnimTimelineEntryCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "IPropertyUtilities.h"
#include "PropertyHandle.h"
#include "WidgetBlueprint.h"
#include "WidgetAnimTimelineEditorUtils.h"
#include "WidgetAnimTimelineSequence.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<IPropertyTypeCustomization> FWidgetAnimTimelineEntryCustomization::MakeInstance()
{
	return MakeShareable(new FWidgetAnimTimelineEntryCustomization());
}

void FWidgetAnimTimelineEntryCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	StructHandle = PropertyHandle;
	HeaderRow.NameContent()[PropertyHandle->CreatePropertyNameWidget()];
}

void FWidgetAnimTimelineEntryCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	StructHandle = PropertyHandle;
	PropertyUtilities = CustomizationUtils.GetPropertyUtilities();
	TargetWidgetNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, TargetWidgetName));
	EntryTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, EntryType));
	AnimationNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName));
	ChildPhaseNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName));

	RefreshTargetWidgetOptions();
	RefreshAnimationOptions();
	RefreshChildPhaseOptions();

	if (EntryTypeHandle.IsValid())
	{
		EntryTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FWidgetAnimTimelineEntryCustomization::OnEntryTypeChanged));
	}

	if (TargetWidgetNameHandle.IsValid())
	{
		ChildBuilder.AddCustomRow(FText::FromString(TEXT("TargetWidgetName")))
		.NameContent()[TargetWidgetNameHandle->CreatePropertyNameWidget()]
		.ValueContent()
		.MinDesiredWidth(220.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&TargetWidgetOptions)
				.OnSelectionChanged(this, &FWidgetAnimTimelineEntryCustomization::OnTargetWidgetSelected)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock).Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty());
				})
				.Content()
				[
					SNew(STextBlock).Text(this, &FWidgetAnimTimelineEntryCustomization::GetCurrentTargetWidgetName)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 3.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &FWidgetAnimTimelineEntryCustomization::GetTargetWidgetValidationText)
				.Visibility(this, &FWidgetAnimTimelineEntryCustomization::GetTargetWidgetValidationVisibility)
				.ColorAndOpacity(FLinearColor(1.0f, 0.22f, 0.18f, 1.0f))
			]
		];
	}

	if (EntryTypeHandle.IsValid())
	{
		ChildBuilder.AddProperty(EntryTypeHandle.ToSharedRef());
	}

	const EWidgetAnimTimelineEntryType CurrentEntryType = GetEntryType();
	if (AnimationNameHandle.IsValid() && CurrentEntryType == EWidgetAnimTimelineEntryType::DirectAnimation)
	{
		ChildBuilder.AddCustomRow(FText::FromString(TEXT("AnimationName")))
		.NameContent()[AnimationNameHandle->CreatePropertyNameWidget()]
		.ValueContent()
		.MinDesiredWidth(220.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&AnimationOptions)
				.OnSelectionChanged(this, &FWidgetAnimTimelineEntryCustomization::OnAnimationSelected)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock).Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty());
				})
				.Content()
				[
					SNew(STextBlock).Text(this, &FWidgetAnimTimelineEntryCustomization::GetCurrentAnimationName)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 3.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &FWidgetAnimTimelineEntryCustomization::GetAnimationNameValidationText)
				.Visibility(this, &FWidgetAnimTimelineEntryCustomization::GetAnimationNameValidationVisibility)
				.ColorAndOpacity(FLinearColor(1.0f, 0.22f, 0.18f, 1.0f))
			]
		];
	}

	if (ChildPhaseNameHandle.IsValid() && CurrentEntryType == EWidgetAnimTimelineEntryType::ChildSequencePhase)
	{
		ChildBuilder.AddCustomRow(FText::FromString(TEXT("ChildPhaseName")))
		.NameContent()[ChildPhaseNameHandle->CreatePropertyNameWidget()]
		.ValueContent()
		.MinDesiredWidth(220.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SComboBox<TSharedPtr<FString>>)
				.OptionsSource(&ChildPhaseOptions)
				.OnSelectionChanged(this, &FWidgetAnimTimelineEntryCustomization::OnChildPhaseSelected)
				.OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)
				{
					return SNew(STextBlock).Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty());
				})
				.Content()
				[
					SNew(STextBlock).Text(this, &FWidgetAnimTimelineEntryCustomization::GetCurrentChildPhaseName)
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 3.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(this, &FWidgetAnimTimelineEntryCustomization::GetChildPhaseNameValidationText)
				.Visibility(this, &FWidgetAnimTimelineEntryCustomization::GetChildPhaseNameValidationVisibility)
				.ColorAndOpacity(FLinearColor(1.0f, 0.22f, 0.18f, 1.0f))
			]
		];
	}

	uint32 ChildCount = 0;
	PropertyHandle->GetNumChildren(ChildCount);
	for (uint32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex);
		if (!ChildHandle.IsValid())
		{
			continue;
		}

		const FName PropertyName = ChildHandle->GetProperty() != nullptr ? ChildHandle->GetProperty()->GetFName() : NAME_None;
		if (PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, TargetWidgetName) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, EntryType) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName) ||
			PropertyName == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName))
		{
			continue;
		}

		ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
	}
}

UWidgetBlueprint* FWidgetAnimTimelineEntryCustomization::GetWidgetBlueprint() const
{
	return FWidgetAnimTimelineEditorUtils::ResolveWidgetBlueprint(StructHandle);
}

UClass* FWidgetAnimTimelineEntryCustomization::ResolveOwnerWidgetClass() const
{
	return FWidgetAnimTimelineEditorUtils::ResolveOwnerWidgetClass(StructHandle, GetWidgetBlueprint());
}

UClass* FWidgetAnimTimelineEntryCustomization::ResolveTargetWidgetClass(FName TargetWidgetName) const
{
	return FWidgetAnimTimelineEditorUtils::ResolveTargetWidgetClass(GetWidgetBlueprint(), ResolveOwnerWidgetClass(), TargetWidgetName);
}

void FWidgetAnimTimelineEntryCustomization::AddTargetWidgetOption(FName TargetWidgetName)
{
	if (TargetWidgetName.IsNone())
	{
		return;
	}

	const FString TargetWidgetNameString = TargetWidgetName.ToString();
	if (!HasOption(TargetWidgetOptions, TargetWidgetNameString))
	{
		TargetWidgetOptions.Add(MakeShared<FString>(TargetWidgetNameString));
	}
}

void FWidgetAnimTimelineEntryCustomization::RefreshTargetWidgetOptions()
{
	TargetWidgetOptions.Reset();
	TargetWidgetOptions.Add(MakeShared<FString>(TEXT("Self (Owner)")));

	TArray<FName> TargetWidgetNames;
	FWidgetAnimTimelineEditorUtils::CollectTargetWidgetNames(GetWidgetBlueprint(), ResolveOwnerWidgetClass(), TargetWidgetNames);
	for (const FName TargetWidgetName : TargetWidgetNames)
	{
		AddTargetWidgetOption(TargetWidgetName);
	}
}

void FWidgetAnimTimelineEntryCustomization::RefreshAnimationOptions()
{
	AnimationOptions.Reset();
	AnimationOptions.Add(MakeShared<FString>(TEXT("None")));

	FName TargetName = NAME_None;
	if (TargetWidgetNameHandle.IsValid())
	{
		TargetWidgetNameHandle->GetValue(TargetName);
	}

	UClass* TargetClass = ResolveTargetWidgetClass(TargetName);
	if (TargetClass == nullptr)
	{
		return;
	}

	TArray<FName> AnimationNames;
	FWidgetAnimTimelineEditorUtils::CollectAnimationNames(TargetClass, AnimationNames);
	for (const FName AnimationName : AnimationNames)
	{
		AnimationOptions.Add(MakeShared<FString>(AnimationName.ToString()));
	}
}

void FWidgetAnimTimelineEntryCustomization::RefreshChildPhaseOptions()
{
	ChildPhaseOptions.Reset();
	ChildPhaseOptions.Add(MakeShared<FString>(TEXT("None")));

	FName TargetName = NAME_None;
	if (TargetWidgetNameHandle.IsValid())
	{
		TargetWidgetNameHandle->GetValue(TargetName);
	}

	UClass* TargetClass = ResolveTargetWidgetClass(TargetName);
	if (TargetClass == nullptr)
	{
		return;
	}

	TArray<FName> PhaseNames;
	const FName ExcludedPhaseName = TargetName.IsNone() ? GetOwnerPhaseName() : NAME_None;
	FWidgetAnimTimelineEditorUtils::CollectChildPhaseNames(TargetClass, ExcludedPhaseName, PhaseNames);
	for (const FName PhaseName : PhaseNames)
	{
		ChildPhaseOptions.Add(MakeShared<FString>(PhaseName.ToString()));
	}
}

FName FWidgetAnimTimelineEntryCustomization::GetOwnerPhaseName() const
{
	if (!StructHandle.IsValid())
	{
		return NAME_None;
	}

	TSharedPtr<IPropertyHandle> ParentHandle = StructHandle->GetParentHandle();
	while (ParentHandle.IsValid())
	{
		if (ParentHandle->GetProperty() != nullptr && ParentHandle->GetProperty()->GetFName() == GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, Entries))
		{
			TSharedPtr<IPropertyHandle> PhaseHandle = ParentHandle->GetParentHandle();
			if (PhaseHandle.IsValid())
			{
				TSharedPtr<IPropertyHandle> PhaseNameHandle = PhaseHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, PhaseName));
				FName PhaseName = NAME_None;
				if (PhaseNameHandle.IsValid())
				{
					PhaseNameHandle->GetValue(PhaseName);
				}
				return PhaseName;
			}
		}

		ParentHandle = ParentHandle->GetParentHandle();
	}

	return NAME_None;
}

bool FWidgetAnimTimelineEntryCustomization::HasOption(const TArray<TSharedPtr<FString>>& Options, const FString& Value) const
{
	for (const TSharedPtr<FString>& Option : Options)
	{
		if (Option.IsValid() && *Option == Value)
		{
			return true;
		}
	}

	return false;
}

void FWidgetAnimTimelineEntryCustomization::OnTargetWidgetSelected(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (!NewValue.IsValid() || !TargetWidgetNameHandle.IsValid())
	{
		return;
	}

	TargetWidgetNameHandle->SetValue(*NewValue == TEXT("Self (Owner)") ? NAME_None : FName(**NewValue));
	RefreshAnimationOptions();
	RefreshChildPhaseOptions();

	if (AnimationNameHandle.IsValid())
	{
		AnimationNameHandle->SetValue(NAME_None);
	}
	if (ChildPhaseNameHandle.IsValid())
	{
		ChildPhaseNameHandle->SetValue(NAME_None);
	}
}

void FWidgetAnimTimelineEntryCustomization::OnAnimationSelected(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (NewValue.IsValid() && AnimationNameHandle.IsValid())
	{
		AnimationNameHandle->SetValue(*NewValue == TEXT("None") ? NAME_None : FName(**NewValue));
	}
}

void FWidgetAnimTimelineEntryCustomization::OnChildPhaseSelected(TSharedPtr<FString> NewValue, ESelectInfo::Type SelectInfo)
{
	if (NewValue.IsValid() && ChildPhaseNameHandle.IsValid())
	{
		ChildPhaseNameHandle->SetValue(*NewValue == TEXT("None") ? NAME_None : FName(**NewValue));
	}
}

void FWidgetAnimTimelineEntryCustomization::OnEntryTypeChanged()
{
	if (PropertyUtilities.IsValid())
	{
		PropertyUtilities->ForceRefresh();
	}
}

FText FWidgetAnimTimelineEntryCustomization::GetCurrentTargetWidgetName() const
{
	FName Value = NAME_None;
	if (TargetWidgetNameHandle.IsValid())
	{
		TargetWidgetNameHandle->GetValue(Value);
	}
	return Value.IsNone() ? FText::FromString(TEXT("Self (Owner)")) : FText::FromName(Value);
}

FText FWidgetAnimTimelineEntryCustomization::GetCurrentAnimationName() const
{
	FName Value = NAME_None;
	if (AnimationNameHandle.IsValid())
	{
		AnimationNameHandle->GetValue(Value);
	}
	return Value.IsNone() ? FText::FromString(TEXT("None")) : FText::FromName(Value);
}

FText FWidgetAnimTimelineEntryCustomization::GetCurrentChildPhaseName() const
{
	FName Value = NAME_None;
	if (ChildPhaseNameHandle.IsValid())
	{
		ChildPhaseNameHandle->GetValue(Value);
	}
	return Value.IsNone() ? FText::FromString(TEXT("None")) : FText::FromName(Value);
}

FText FWidgetAnimTimelineEntryCustomization::GetTargetWidgetValidationText() const
{
	FName Value = NAME_None;
	if (TargetWidgetNameHandle.IsValid())
	{
		TargetWidgetNameHandle->GetValue(Value);
	}

	if (!Value.IsNone() && !HasOption(TargetWidgetOptions, Value.ToString()))
	{
		return FText::FromString(FString::Printf(TEXT("Missing TargetWidgetName: %s"), *Value.ToString()));
	}

	return FText::GetEmpty();
}

FText FWidgetAnimTimelineEntryCustomization::GetAnimationNameValidationText() const
{
	if (GetEntryType() != EWidgetAnimTimelineEntryType::DirectAnimation)
	{
		return FText::GetEmpty();
	}

	FName Value = NAME_None;
	if (AnimationNameHandle.IsValid())
	{
		AnimationNameHandle->GetValue(Value);
	}

	if (Value.IsNone())
	{
		return FText::FromString(TEXT("AnimationName is not set"));
	}
	if (!HasOption(AnimationOptions, Value.ToString()))
	{
		return FText::FromString(FString::Printf(TEXT("Missing AnimationName: %s"), *Value.ToString()));
	}

	return FText::GetEmpty();
}

FText FWidgetAnimTimelineEntryCustomization::GetChildPhaseNameValidationText() const
{
	if (GetEntryType() != EWidgetAnimTimelineEntryType::ChildSequencePhase)
	{
		return FText::GetEmpty();
	}

	FName Value = NAME_None;
	if (ChildPhaseNameHandle.IsValid())
	{
		ChildPhaseNameHandle->GetValue(Value);
	}

	if (Value.IsNone())
	{
		return FText::FromString(TEXT("ChildPhaseName is not set"));
	}
	if (!HasOption(ChildPhaseOptions, Value.ToString()))
	{
		return FText::FromString(FString::Printf(TEXT("Missing ChildPhaseName: %s"), *Value.ToString()));
	}

	return FText::GetEmpty();
}

EVisibility FWidgetAnimTimelineEntryCustomization::GetTargetWidgetValidationVisibility() const
{
	return GetTargetWidgetValidationText().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility FWidgetAnimTimelineEntryCustomization::GetAnimationNameValidationVisibility() const
{
	return GetAnimationNameValidationText().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}

EVisibility FWidgetAnimTimelineEntryCustomization::GetChildPhaseNameValidationVisibility() const
{
	return GetChildPhaseNameValidationText().IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible;
}

EWidgetAnimTimelineEntryType FWidgetAnimTimelineEntryCustomization::GetEntryType() const
{
	uint8 EntryTypeValue = static_cast<uint8>(EWidgetAnimTimelineEntryType::DirectAnimation);
	if (EntryTypeHandle.IsValid())
	{
		if (EntryTypeHandle->GetValue(EntryTypeValue) == FPropertyAccess::Success)
		{
			return static_cast<EWidgetAnimTimelineEntryType>(EntryTypeValue);
		}

		FString FormattedValue;
		if (EntryTypeHandle->GetValueAsFormattedString(FormattedValue) == FPropertyAccess::Success)
		{
			if (FormattedValue.Contains(TEXT("ChildSequencePhase")))
			{
				return EWidgetAnimTimelineEntryType::ChildSequencePhase;
			}
			if (FormattedValue.Contains(TEXT("DirectAnimation")))
			{
				return EWidgetAnimTimelineEntryType::DirectAnimation;
			}
		}
	}

	return static_cast<EWidgetAnimTimelineEntryType>(EntryTypeValue);
}
