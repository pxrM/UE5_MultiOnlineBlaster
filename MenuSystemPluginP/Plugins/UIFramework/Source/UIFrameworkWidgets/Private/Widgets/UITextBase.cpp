// Copyright TikiStar. All Rights Reserved.

#include "Widgets/UITextBase.h"
#include "Styles/UITextStyle.h"
#include "Components/TextBlock.h"

void UUITextBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	if (Text && !DefaultText.IsEmpty())
	{
		Text->SetText(DefaultText);
	}
	ApplyStyle();
}

void UUITextBase::SetText(const FText& InText)
{
	if (Text)
	{
		Text->SetText(InText);
	}
}

void UUITextBase::SetStyleAsset(UUITextStyle* InStyle)
{
	Style = InStyle;
	ApplyStyle();
}

void UUITextBase::ApplyStyle()
{
	if (!Text || !Style)
	{
		return;
	}

	Text->SetFont(Style->Font);
	Text->SetColorAndOpacity(Style->Color);
	Text->SetShadowOffset(Style->ShadowOffset);
	Text->SetShadowColorAndOpacity(Style->ShadowColor);
}
