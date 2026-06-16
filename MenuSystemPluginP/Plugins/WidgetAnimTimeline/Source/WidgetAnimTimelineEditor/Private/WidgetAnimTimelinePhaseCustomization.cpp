#include "WidgetAnimTimelinePhaseCustomization.h"

#include "DetailWidgetRow.h"
#include "IDetailChildrenBuilder.h"
#include "PropertyHandle.h"
#include "Widgets/SNullWidget.h"

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
	.MinDesiredWidth(120.0f)
	[
		SNullWidget::NullWidget
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
