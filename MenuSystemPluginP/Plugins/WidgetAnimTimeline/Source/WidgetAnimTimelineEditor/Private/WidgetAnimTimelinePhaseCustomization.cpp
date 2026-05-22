#include "WidgetAnimTimelinePhaseCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "SWidgetAnimTimelinePanel.h"
#include "WidgetAnimTimelineDesignerPreviewController.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SWindow.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/Application/SlateApplication.h"

TSharedRef<IPropertyTypeCustomization> FWidgetAnimTimelinePhaseCustomization::MakeInstance()
{
	return MakeShareable(new FWidgetAnimTimelinePhaseCustomization());
}

void FWidgetAnimTimelinePhaseCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PhaseHandle = PropertyHandle;

	HeaderRow.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(320.0f)
	[
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Open Timeline")))
			.OnClicked(this, &FWidgetAnimTimelinePhaseCustomization::OpenTimeline)
		]
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 0.0f, 4.0f, 0.0f)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("Preview In Designer")))
			.OnClicked(this, &FWidgetAnimTimelinePhaseCustomization::PlayDesignerPreview)
		]
	];
}

void FWidgetAnimTimelinePhaseCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	uint32 ChildCount = 0;
	PropertyHandle->GetNumChildren(ChildCount);
	for (uint32 ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
	{
		TSharedPtr<IPropertyHandle> ChildHandle = PropertyHandle->GetChildHandle(ChildIndex);
		if (ChildHandle.IsValid())
		{
			ChildBuilder.AddProperty(ChildHandle.ToSharedRef());
		}
	}
}

FReply FWidgetAnimTimelinePhaseCustomization::OpenTimeline()
{
	TSharedRef<SWindow> Window = SNew(SWindow)
		.Title(FText::FromString(TEXT("Widget Anim Timeline")))
		.ClientSize(FVector2D(900.0f, 460.0f))
		.SupportsMaximize(true)
		.SupportsMinimize(false)
		[
			SNew(SWidgetAnimTimelinePanel)
			.PhaseHandle(PhaseHandle)
		];

	FSlateApplication::Get().AddWindow(Window);
	return FReply::Handled();
}

FReply FWidgetAnimTimelinePhaseCustomization::PlayDesignerPreview()
{
	FWidgetAnimTimelineDesignerPreviewController::Play(PhaseHandle);
	return FReply::Handled();
}
