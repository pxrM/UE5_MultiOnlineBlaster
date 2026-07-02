// Copyright TikiStar. All Rights Reserved.

#include "Widgets/UIButtonBase.h"
#include "Styles/UIButtonStyle.h"
#include "Styles/UITextStyle.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Styling/SlateTypes.h"

void UUIButtonBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Button)
	{
		// Avoid double-binding if reinitialized.
		Button->OnClicked.RemoveDynamic(this, &UUIButtonBase::HandleClicked);
		Button->OnClicked.AddDynamic(this, &UUIButtonBase::HandleClicked);
	}
}

void UUIButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (Label && !DefaultLabel.IsEmpty())
	{
		Label->SetText(DefaultLabel);
	}
	ApplyStyle();
}

void UUIButtonBase::SetLabel(const FText& InText)
{
	if (Label)
	{
		Label->SetText(InText);
	}
}

void UUIButtonBase::SetStyleAsset(UUIButtonStyle* InStyle)
{
	Style = InStyle;
	ApplyStyle();
}

void UUIButtonBase::SetButtonEnabled(bool bEnabled)
{
	if (Button)
	{
		Button->SetIsEnabled(bEnabled);
	}
}

void UUIButtonBase::ApplyStyle()
{
	if (!Style)
	{
		return;
	}

	if (Button)
	{
		FButtonStyle NewStyle = Button->GetStyle();
		NewStyle.SetNormal(Style->Normal);
		NewStyle.SetHovered(Style->Hovered);
		NewStyle.SetPressed(Style->Pressed);
		NewStyle.SetDisabled(Style->Disabled);
		NewStyle.SetNormalPadding(Style->ContentPadding);
		NewStyle.SetPressedPadding(Style->ContentPadding);
		Button->SetStyle(NewStyle);
	}

	// Cascade the label text style from the button style.
	if (Label && Style->TextStyle)
	{
		const UUITextStyle* TextStyle = Style->TextStyle;
		Label->SetFont(TextStyle->Font);
		Label->SetColorAndOpacity(TextStyle->Color);
		Label->SetShadowOffset(TextStyle->ShadowOffset);
		Label->SetShadowColorAndOpacity(TextStyle->ShadowColor);
	}
}

void UUIButtonBase::HandleClicked()
{
	OnClicked.Broadcast();
}
