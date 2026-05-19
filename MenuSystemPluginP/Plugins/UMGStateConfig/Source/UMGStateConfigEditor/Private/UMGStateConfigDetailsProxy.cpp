#include "UMGStateConfigDetailsProxy.h"

void UUMGStateConfigVisibilityProxy::FromValue(const FUMGStateConfigPropertyValue& Value)
{
	Visibility = Value.VisibilityValue;
}

void UUMGStateConfigVisibilityProxy::ToValue(FUMGStateConfigPropertyValue& Value) const
{
	Value.VisibilityValue = Visibility;
}

void UUMGStateConfigRenderOpacityProxy::FromValue(const FUMGStateConfigPropertyValue& Value)
{
	RenderOpacity = Value.FloatValue;
}

void UUMGStateConfigRenderOpacityProxy::ToValue(FUMGStateConfigPropertyValue& Value) const
{
	Value.FloatValue = FMath::Clamp(RenderOpacity, 0.0f, 1.0f);
}

void UUMGStateConfigTextContentProxy::FromValue(const FUMGStateConfigPropertyValue& Value)
{
	Text = Value.TextValue;
}

void UUMGStateConfigTextContentProxy::ToValue(FUMGStateConfigPropertyValue& Value) const
{
	Value.TextValue = Text;
}

void UUMGStateConfigImageAppearanceProxy::FromValue(const FUMGStateConfigPropertyValue& Value)
{
	Brush = Value.BrushValue;
	ColorAndOpacity = Value.ColorValue;
}

void UUMGStateConfigImageAppearanceProxy::ToValue(FUMGStateConfigPropertyValue& Value) const
{
	Value.BrushValue = Brush;
	Value.ColorValue = ColorAndOpacity;
	Value.ObjectValue = Brush.GetResourceObject();
}

void UUMGStateConfigTextAppearanceProxy::FromValue(const FUMGStateConfigPropertyValue& Value)
{
	ColorAndOpacity = FSlateColor(Value.ColorValue);
	Font = Value.FontValue;
	ShadowOffset = Value.VectorValue;
	ShadowColorAndOpacity = Value.SecondaryColorValue;
}

void UUMGStateConfigTextAppearanceProxy::ToValue(FUMGStateConfigPropertyValue& Value) const
{
	Value.ColorValue = ColorAndOpacity.GetSpecifiedColor();
	Value.FontValue = Font;
	Value.VectorValue = ShadowOffset;
	Value.SecondaryColorValue = ShadowColorAndOpacity;
}
