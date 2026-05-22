#include "UMGStateConfigPropertyRuntimeLibrary.h"

#include "Components/Image.h"
#include "Components/RichTextBlock.h"
#include "Components/TextBlock.h"
#include "Engine/Texture2D.h"
#include "Materials/MaterialInterface.h"
#include "Slate/SlateBrushAsset.h"
#include "UObject/UnrealType.h"

namespace
{
struct FResolvedSerializedProperty
{
	FProperty* Property = nullptr;
	void* ValuePtr = nullptr;
};

bool ParseSegment(const FString& Segment, FString& OutName, int32& OutIndex)
{
	OutIndex = INDEX_NONE;
	const int32 OpenBracket = Segment.Find(TEXT("["));
	if (OpenBracket == INDEX_NONE)
	{
		OutName = Segment;
		return true;
	}
	if (!Segment.EndsWith(TEXT("]")))
	{
		return false;
	}
	OutName = Segment.Left(OpenBracket);
	const FString IndexText = Segment.Mid(OpenBracket + 1, Segment.Len() - OpenBracket - 2);
	if (IndexText.IsEmpty() || !IndexText.IsNumeric())
	{
		return false;
	}
	OutIndex = FCString::Atoi(*IndexText);
	return OutIndex >= 0;
}

bool ResolveSerializedProperty(UObject* Object, const FString& PropertyPath, FResolvedSerializedProperty& OutResolved)
{
	if (!Object || PropertyPath.IsEmpty())
	{
		return false;
	}

	TArray<FString> PathSegments;
	PropertyPath.ParseIntoArray(PathSegments, TEXT("."), true);
	if (PathSegments.Num() == 0)
	{
		return false;
	}

	UStruct* CurrentStruct = Object->GetClass();
	void* CurrentContainer = Object;
	for (int32 SegmentIndex = 0; SegmentIndex < PathSegments.Num(); ++SegmentIndex)
	{
		if (!CurrentStruct || !CurrentContainer)
		{
			return false;
		}

		FString SegmentName;
		int32 ArrayIndex = INDEX_NONE;
		if (!ParseSegment(PathSegments[SegmentIndex], SegmentName, ArrayIndex))
		{
			return false;
		}

		FProperty* Property = CurrentStruct->FindPropertyByName(FName(*SegmentName));
		if (!Property)
		{
			return false;
		}

		void* ValuePtr = Property->ContainerPtrToValuePtr<void>(CurrentContainer);
		FProperty* ElementProperty = Property;

		if (ArrayIndex != INDEX_NONE)
		{
			FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property);
			if (!ArrayProperty)
			{
				return false;
			}
			FScriptArrayHelper Helper(ArrayProperty, ValuePtr);
			if (!Helper.IsValidIndex(ArrayIndex))
			{
				return false;
			}
			ValuePtr = Helper.GetRawPtr(ArrayIndex);
			ElementProperty = ArrayProperty->Inner;
		}

		if (SegmentIndex == PathSegments.Num() - 1)
		{
			OutResolved.Property = ElementProperty;
			OutResolved.ValuePtr = ValuePtr;
			return true;
		}

		FStructProperty* StructProperty = CastField<FStructProperty>(ElementProperty);
		if (!StructProperty)
		{
			return false;
		}

		CurrentStruct = StructProperty->Struct;
		CurrentContainer = ValuePtr;
	}

	return false;
}

bool ExportSerializedPropertyValue(UObject* Object, const FString& PropertyPath, FUMGStateConfigPropertyValue& OutValue)
{
	FResolvedSerializedProperty Resolved;
	if (!ResolveSerializedProperty(Object, PropertyPath, Resolved) || !Resolved.Property || !Resolved.ValuePtr)
	{
		return false;
	}

	FString ExportedValue;
	Resolved.Property->ExportTextItem_Direct(ExportedValue, Resolved.ValuePtr, nullptr, Object, PPF_None);
	OutValue.SerializedPropertyPath = PropertyPath;
	OutValue.SerializedPropertyValue = ExportedValue;
	return true;
}

bool ImportSerializedPropertyValue(UWidget* TargetWidget, const FUMGStateConfigPropertyValue& Value)
{
	FResolvedSerializedProperty Resolved;
	if (!ResolveSerializedProperty(TargetWidget, Value.SerializedPropertyPath, Resolved) || !Resolved.Property || !Resolved.ValuePtr)
	{
		return false;
	}

	const TCHAR* ImportResult = Resolved.Property->ImportText_Direct(*Value.SerializedPropertyValue, Resolved.ValuePtr, TargetWidget, PPF_None);
	if (!ImportResult)
	{
		return false;
	}

	TargetWidget->SynchronizeProperties();
	TargetWidget->InvalidateLayoutAndVolatility();
	return true;
}

bool AreLinearColorsEqual(const FLinearColor& A, const FLinearColor& B)

{
	return A.Equals(B, KINDA_SMALL_NUMBER);
}

bool AreSlateColorsEqual(const FSlateColor& A, const FSlateColor& B)
{
	return AreLinearColorsEqual(A.GetSpecifiedColor(), B.GetSpecifiedColor());
}

bool AreBrushesEqual(const FSlateBrush& A, const FSlateBrush& B)
{
	return A.GetResourceObject() == B.GetResourceObject()
		&& A.ImageSize.Equals(B.ImageSize, KINDA_SMALL_NUMBER)
		&& A.Margin == B.Margin
		&& A.DrawAs == B.DrawAs
		&& A.Tiling == B.Tiling
		&& A.Mirroring == B.Mirroring
		&& AreSlateColorsEqual(A.TintColor, B.TintColor);
}
}

bool FUMGStateConfigPropertyRuntimeLibrary::ArePropertyValuesEqual(EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& A, const FUMGStateConfigPropertyValue& B)
{
	switch (PropertyType)
	{
	case EUMGStateConfigPropertyType::Visibility:
		return A.VisibilityValue == B.VisibilityValue;
	case EUMGStateConfigPropertyType::RenderOpacity:
		return FMath::IsNearlyEqual(A.FloatValue, B.FloatValue, KINDA_SMALL_NUMBER);
	case EUMGStateConfigPropertyType::Text:
		return A.TextValue.EqualTo(B.TextValue);
	case EUMGStateConfigPropertyType::TextAppearance:
		return AreLinearColorsEqual(A.ColorValue, B.ColorValue)
			&& A.FontValue == B.FontValue
			&& A.VectorValue.Equals(B.VectorValue, KINDA_SMALL_NUMBER)
			&& AreLinearColorsEqual(A.SecondaryColorValue, B.SecondaryColorValue);
	case EUMGStateConfigPropertyType::TextColor:
	case EUMGStateConfigPropertyType::BrushTint:
		return AreLinearColorsEqual(A.ColorValue, B.ColorValue);
	case EUMGStateConfigPropertyType::BrushImage:
		return A.ObjectValue == B.ObjectValue;
	case EUMGStateConfigPropertyType::ImageAppearance:
		return AreBrushesEqual(A.BrushValue, B.BrushValue)
			&& AreLinearColorsEqual(A.ColorValue, B.ColorValue);
	case EUMGStateConfigPropertyType::SerializedProperty:
		return A.SerializedPropertyPath == B.SerializedPropertyPath
			&& A.SerializedPropertyValue == B.SerializedPropertyValue;
	default:
		return false;
	}
}


bool FUMGStateConfigPropertyRuntimeLibrary::CaptureCurrentValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, FUMGStateConfigPropertyValue& OutValue)
{
	if (!TargetWidget)
	{
		return false;
	}

	switch (PropertyType)
	{
	case EUMGStateConfigPropertyType::Visibility:
		OutValue.VisibilityValue = TargetWidget->GetVisibility();
		return true;
	case EUMGStateConfigPropertyType::RenderOpacity:
		OutValue.FloatValue = TargetWidget->GetRenderOpacity();
		return true;
	case EUMGStateConfigPropertyType::Text:
		if (const UTextBlock* TextBlock = Cast<UTextBlock>(TargetWidget))
		{
			OutValue.TextValue = TextBlock->GetText();
			return true;
		}
		if (const URichTextBlock* RichTextBlock = Cast<URichTextBlock>(TargetWidget))
		{
			OutValue.TextValue = RichTextBlock->GetText();
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::TextAppearance:
		if (const UTextBlock* TextBlock = Cast<UTextBlock>(TargetWidget))
		{
			OutValue.ColorValue = TextBlock->GetColorAndOpacity().GetSpecifiedColor();
			OutValue.FontValue = TextBlock->GetFont();
			OutValue.VectorValue = TextBlock->GetShadowOffset();
			OutValue.SecondaryColorValue = TextBlock->GetShadowColorAndOpacity();
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::TextColor:
		if (const UTextBlock* TextBlock = Cast<UTextBlock>(TargetWidget))
		{
			OutValue.ColorValue = TextBlock->GetColorAndOpacity().GetSpecifiedColor();
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::BrushImage:
		if (const UImage* Image = Cast<UImage>(TargetWidget))
		{
			OutValue.ObjectValue = Image->GetBrush().GetResourceObject();
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::BrushTint:
		if (const UImage* Image = Cast<UImage>(TargetWidget))
		{
			OutValue.ColorValue = Image->GetColorAndOpacity();
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::ImageAppearance:
		if (const UImage* Image = Cast<UImage>(TargetWidget))
		{
			OutValue.BrushValue = Image->GetBrush();
			OutValue.ColorValue = Image->GetColorAndOpacity();
			OutValue.ObjectValue = Image->GetBrush().GetResourceObject();
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::SerializedProperty:
		return ExportSerializedPropertyValue(TargetWidget, OutValue.SerializedPropertyPath, OutValue);
	default:
		return false;
	}
}


bool FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& Value)
{
	if (!TargetWidget)
	{
		return false;
	}

	switch (PropertyType)
	{
	case EUMGStateConfigPropertyType::Visibility:
		if (TargetWidget->GetVisibility() != Value.VisibilityValue)
		{
			TargetWidget->SetVisibility(Value.VisibilityValue);
		}
		return true;
	case EUMGStateConfigPropertyType::RenderOpacity:
	{
		const float TargetOpacity = FMath::Clamp(Value.FloatValue, 0.0f, 1.0f);
		if (!FMath::IsNearlyEqual(TargetWidget->GetRenderOpacity(), TargetOpacity, KINDA_SMALL_NUMBER))
		{
			TargetWidget->SetRenderOpacity(TargetOpacity);
		}
		return true;
	}
	case EUMGStateConfigPropertyType::Text:
		if (UTextBlock* TextBlock = Cast<UTextBlock>(TargetWidget))
		{
			if (!TextBlock->GetText().EqualTo(Value.TextValue))
			{
				TextBlock->SetText(Value.TextValue);
			}
			return true;
		}
		if (URichTextBlock* RichTextBlock = Cast<URichTextBlock>(TargetWidget))
		{
			if (!RichTextBlock->GetText().EqualTo(Value.TextValue))
			{
				RichTextBlock->SetText(Value.TextValue);
			}
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::TextAppearance:
		if (UTextBlock* TextBlock = Cast<UTextBlock>(TargetWidget))
		{
			if (!AreLinearColorsEqual(TextBlock->GetColorAndOpacity().GetSpecifiedColor(), Value.ColorValue))
			{
				TextBlock->SetColorAndOpacity(FSlateColor(Value.ColorValue));
			}
			if (!(TextBlock->GetFont() == Value.FontValue))
			{
				TextBlock->SetFont(Value.FontValue);
			}
			if (!TextBlock->GetShadowOffset().Equals(Value.VectorValue, KINDA_SMALL_NUMBER))
			{
				TextBlock->SetShadowOffset(Value.VectorValue);
			}
			if (!AreLinearColorsEqual(TextBlock->GetShadowColorAndOpacity(), Value.SecondaryColorValue))
			{
				TextBlock->SetShadowColorAndOpacity(Value.SecondaryColorValue);
			}
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::TextColor:
		if (UTextBlock* TextBlock = Cast<UTextBlock>(TargetWidget))
		{
			if (!AreLinearColorsEqual(TextBlock->GetColorAndOpacity().GetSpecifiedColor(), Value.ColorValue))
			{
				TextBlock->SetColorAndOpacity(FSlateColor(Value.ColorValue));
			}
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::BrushImage:
		if (UImage* Image = Cast<UImage>(TargetWidget))
		{
			if (Image->GetBrush().GetResourceObject() == Value.ObjectValue.Get())
			{
				return true;
			}
			if (UTexture2D* Texture = Cast<UTexture2D>(Value.ObjectValue.Get()))
			{
				Image->SetBrushFromTexture(Texture, false);
				return true;
			}
			if (UMaterialInterface* Material = Cast<UMaterialInterface>(Value.ObjectValue.Get()))
			{
				Image->SetBrushFromMaterial(Material);
				return true;
			}
			if (USlateBrushAsset* BrushAsset = Cast<USlateBrushAsset>(Value.ObjectValue.Get()))
			{
				Image->SetBrushFromAsset(BrushAsset);
				return true;
			}
			Image->SetBrushResourceObject(Value.ObjectValue.Get());
			return Value.ObjectValue != nullptr;
		}
		return false;
	case EUMGStateConfigPropertyType::BrushTint:
		if (UImage* Image = Cast<UImage>(TargetWidget))
		{
			if (!AreLinearColorsEqual(Image->GetColorAndOpacity(), Value.ColorValue))
			{
				Image->SetColorAndOpacity(Value.ColorValue);
			}
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::ImageAppearance:
		if (UImage* Image = Cast<UImage>(TargetWidget))
		{
			if (!AreBrushesEqual(Image->GetBrush(), Value.BrushValue))
			{
				Image->SetBrush(Value.BrushValue);
			}
			if (!AreLinearColorsEqual(Image->GetColorAndOpacity(), Value.ColorValue))
			{
				Image->SetColorAndOpacity(Value.ColorValue);
			}
			return true;
		}
		return false;
	case EUMGStateConfigPropertyType::SerializedProperty:
		return ImportSerializedPropertyValue(TargetWidget, Value);
	default:
		return false;
	}
}

