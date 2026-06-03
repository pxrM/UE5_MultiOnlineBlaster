#include "UMGStateConfigPropertyRuntimeLibrary.h"

#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/RichTextBlock.h"
#include "Components/Slider.h"
#include "Components/SpinBox.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "UObject/UnrealType.h"


namespace
{
struct FResolvedSerializedProperty
{
	FProperty* Property = nullptr;
	void* ValuePtr = nullptr;
};

TMap<FString, TArray<FName>>& GetSerializedPropertyPathCache()
{
	static TMap<FString, TArray<FName>> PathCache;
	return PathCache;
}

TSet<FSoftObjectPath>& GetLoadedSerializedAssetCache()
{
	static TSet<FSoftObjectPath> LoadedAssetPaths;
	return LoadedAssetPaths;
}

bool PathMatchesOrIsChildOf(const FString& PropertyPath, const TCHAR* AllowedPath)
{
	return PropertyPath == AllowedPath || PropertyPath.StartsWith(FString::Printf(TEXT("%s."), AllowedPath));
}

bool IsSerializedPropertyPathAllowedInternal(const UWidget* TargetWidget, const FString& PropertyPath)
{
	if (!TargetWidget || PropertyPath.IsEmpty())
	{
		return false;
	}

	if (PropertyPath == TEXT("Visibility")
		|| PropertyPath == TEXT("RenderOpacity")
		|| PathMatchesOrIsChildOf(PropertyPath, TEXT("RenderTransform")))
	{
		return true;
	}

	if (TargetWidget->IsA<UImage>())
	{
		return PathMatchesOrIsChildOf(PropertyPath, TEXT("Brush"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("ColorAndOpacity"));
	}

	if (TargetWidget->IsA<UTextBlock>())
	{
		return PropertyPath == TEXT("Text")
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("ColorAndOpacity"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("Font"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("ShadowOffset"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("ShadowColorAndOpacity"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("StrikeBrush"));
	}

	if (TargetWidget->IsA<URichTextBlock>())
	{
		return PropertyPath == TEXT("Text");
	}

	if (TargetWidget->IsA<UButton>())
	{
		return PathMatchesOrIsChildOf(PropertyPath, TEXT("Style"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("ColorAndOpacity"))
			|| PropertyPath == TEXT("IsEnabled");
	}

	if (TargetWidget->IsA<UBorder>())
	{
		return PathMatchesOrIsChildOf(PropertyPath, TEXT("Brush"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("ContentColorAndOpacity"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("Padding"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("BackgroundColor"))
			|| PropertyPath == TEXT("BrushColor");
	}

	if (TargetWidget->IsA<UProgressBar>())
	{
		return PathMatchesOrIsChildOf(PropertyPath, TEXT("FillColorAndOpacity"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("BackgroundImage"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("FillImage"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("BorderImage"))
			|| PropertyPath == TEXT("Percent")
			|| PropertyPath == TEXT("BarFillType")
			|| PropertyPath == TEXT("bIsMarquee");
	}

	if (TargetWidget->IsA<USlider>())
	{
		return PathMatchesOrIsChildOf(PropertyPath, TEXT("SliderBarColor"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("SliderHandleColor"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("IndentHandle"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("Locked"))
			|| PropertyPath == TEXT("StepSize")
			|| PropertyPath == TEXT("Value");
	}

	if (TargetWidget->IsA<UCheckBox>())
	{
		return PathMatchesOrIsChildOf(PropertyPath, TEXT("Style"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("CheckedState"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("ForegroundColor"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("Padding"));
	}

	if (TargetWidget->IsA<UEditableTextBox>())
	{
		return PropertyPath == TEXT("Text")
			|| PropertyPath == TEXT("HintText")
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("ForegroundColor"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("BackgroundColor"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("Padding"));
	}

	if (TargetWidget->IsA<USpinBox>())
	{
		return PropertyPath == TEXT("Value")
			|| PropertyPath == TEXT("MinValue")
			|| PropertyPath == TEXT("MaxValue")
			|| PropertyPath == TEXT("Delta")
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("ForegroundColor"))
			|| PathMatchesOrIsChildOf(PropertyPath, TEXT("BrushColor"));
	}

	return false;
}

bool IsSerializedPropertySupported(const FProperty* Property)

{
	if (!Property)
	{
		return false;
	}

	const EPropertyFlags UnsupportedFlags = CPF_Transient | CPF_EditConst | CPF_Deprecated | CPF_DisableEditOnInstance;
	if (Property->HasAnyPropertyFlags(UnsupportedFlags))
	{
		return false;
	}

	if (CastField<FDelegateProperty>(Property)
		|| CastField<FMulticastDelegateProperty>(Property)
		|| CastField<FArrayProperty>(Property)
		|| CastField<FMapProperty>(Property)
		|| CastField<FSetProperty>(Property))

	{
		return false;
	}

	return true;
}

void CollectSerializedPropertyAssetReferences(FProperty* Property, void* ValuePtr, TArray<FSoftObjectPath>& OutReferences)
{
	if (!Property || !ValuePtr)
	{
		return;
	}

	if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
	{
		if (UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(ValuePtr))
		{
			OutReferences.AddUnique(FSoftObjectPath(ObjectValue));
		}
		return;
	}

	if (FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(Property))
	{
		const FSoftObjectPtr SoftObjectValue = SoftObjectProperty->GetPropertyValue(ValuePtr);
		if (!SoftObjectValue.IsNull())
		{
			OutReferences.AddUnique(SoftObjectValue.ToSoftObjectPath());
		}
		return;
	}

	if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
	{
		for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
		{
			FProperty* ChildProperty = *It;
			CollectSerializedPropertyAssetReferences(ChildProperty, ChildProperty->ContainerPtrToValuePtr<void>(ValuePtr), OutReferences);
		}
	}
}

bool ResolveSerializedProperty(UObject* Object, const FString& PropertyPath, FResolvedSerializedProperty& OutResolved)
{
	if (!Object || PropertyPath.IsEmpty())
	{
		return false;
	}

	TArray<FName>* CachedPathSegments = GetSerializedPropertyPathCache().Find(PropertyPath);
	if (!CachedPathSegments)
	{
		TArray<FString> PathSegmentStrings;
		PropertyPath.ParseIntoArray(PathSegmentStrings, TEXT("."), true);
		TArray<FName> ParsedPathSegments;
		for (const FString& PathSegmentString : PathSegmentStrings)
		{
			ParsedPathSegments.Add(FName(*PathSegmentString));
		}
		CachedPathSegments = &GetSerializedPropertyPathCache().Add(PropertyPath, MoveTemp(ParsedPathSegments));
	}
	if (!CachedPathSegments || CachedPathSegments->Num() == 0)
	{
		return false;
	}

	const TArray<FName>& PathSegments = *CachedPathSegments;
	UStruct* CurrentStruct = Object->GetClass();
	void* CurrentContainer = Object;
	for (int32 SegmentIndex = 0; SegmentIndex < PathSegments.Num(); ++SegmentIndex)
	{
		if (!CurrentStruct || !CurrentContainer)
		{
			return false;
		}

		FProperty* Property = CurrentStruct->FindPropertyByName(PathSegments[SegmentIndex]);
		if (!Property || !IsSerializedPropertySupported(Property))
		{
			return false;
		}

		void* ValuePtr = Property->ContainerPtrToValuePtr<void>(CurrentContainer);
		if (SegmentIndex == PathSegments.Num() - 1)
		{
			OutResolved.Property = Property;
			OutResolved.ValuePtr = ValuePtr;
			return true;
		}

		FStructProperty* StructProperty = CastField<FStructProperty>(Property);
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
	OutValue.SerializedPropertyTypeName = Resolved.Property->GetCPPType();
	OutValue.SerializedPropertyValue = ExportedValue;
	OutValue.SerializedReferencedAssets.Reset();
	CollectSerializedPropertyAssetReferences(Resolved.Property, Resolved.ValuePtr, OutValue.SerializedReferencedAssets);
	return true;
}

bool ImportSerializedPropertyValue(UWidget* TargetWidget, const FUMGStateConfigPropertyValue& Value, bool bRefreshAfterApply)
{
	if (!IsSerializedPropertyPathAllowedInternal(TargetWidget, Value.SerializedPropertyPath))
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("Serialized property path is not allowed: Widget=%s, PropertyPath=%s"),
			TargetWidget ? *TargetWidget->GetName() : TEXT("None"),
			*Value.SerializedPropertyPath);
		return false;
	}

	FResolvedSerializedProperty Resolved;

	if (!ResolveSerializedProperty(TargetWidget, Value.SerializedPropertyPath, Resolved) || !Resolved.Property || !Resolved.ValuePtr)
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("Resolve serialized property failed: Widget=%s, PropertyPath=%s"),
			TargetWidget ? *TargetWidget->GetName() : TEXT("None"),
			*Value.SerializedPropertyPath);
		return false;
	}

	FUMGStateConfigPropertyRuntimeLibrary::PreloadReferencedAssets(Value.SerializedReferencedAssets);



	const TCHAR* ImportResult = Resolved.Property->ImportText_Direct(*Value.SerializedPropertyValue, Resolved.ValuePtr, TargetWidget, PPF_None);
	if (!ImportResult)
	{
		UE_LOG(LogUMGStateConfig, Warning, TEXT("Import serialized property failed: Widget=%s, PropertyPath=%s, Value=%s"),
			*TargetWidget->GetName(),
			*Value.SerializedPropertyPath,
			*Value.SerializedPropertyValue);
		return false;
	}

	if (bRefreshAfterApply)
	{
		TargetWidget->SynchronizeProperties();
		TargetWidget->InvalidateLayoutAndVolatility();
	}
	return true;
}


}

bool FUMGStateConfigPropertyRuntimeLibrary::ArePropertyValuesEqual(EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& A, const FUMGStateConfigPropertyValue& B)
{
	if (PropertyType != EUMGStateConfigPropertyType::SerializedProperty)
	{
		return false;
	}

	return A.SerializedPropertyPath == B.SerializedPropertyPath
		&& A.SerializedPropertyValue == B.SerializedPropertyValue;
}

bool FUMGStateConfigPropertyRuntimeLibrary::CaptureCurrentValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, FUMGStateConfigPropertyValue& OutValue)
{
	if (!TargetWidget || PropertyType != EUMGStateConfigPropertyType::SerializedProperty)
	{
		return false;
	}
	if (!IsSerializedPropertyPathAllowed(TargetWidget, OutValue.SerializedPropertyPath))
	{
		return false;
	}

	return ExportSerializedPropertyValue(TargetWidget, OutValue.SerializedPropertyPath, OutValue);
}

bool FUMGStateConfigPropertyRuntimeLibrary::ApplyValue(UWidget* TargetWidget, EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& Value, bool bRefreshAfterApply)
{
	if (!TargetWidget || PropertyType != EUMGStateConfigPropertyType::SerializedProperty)
	{
		return false;
	}

	return ImportSerializedPropertyValue(TargetWidget, Value, bRefreshAfterApply);
}

bool FUMGStateConfigPropertyRuntimeLibrary::IsSerializedPropertyPathAllowed(const UWidget* TargetWidget, const FString& PropertyPath)
{
	return IsSerializedPropertyPathAllowedInternal(TargetWidget, PropertyPath);
}

void FUMGStateConfigPropertyRuntimeLibrary::PreloadReferencedAssets(const TArray<FSoftObjectPath>& ReferencedAssets, bool bAsync)
{
	TArray<FSoftObjectPath> AssetsToLoad;
	for (const FSoftObjectPath& ReferencedAsset : ReferencedAssets)
	{
		if (!ReferencedAsset.IsNull() && !GetLoadedSerializedAssetCache().Contains(ReferencedAsset))
		{
			AssetsToLoad.Add(ReferencedAsset);
		}
	}

	if (AssetsToLoad.Num() == 0)
	{
		return;
	}

	if (bAsync)
	{
		// 异步加载，完成后才更新缓存
		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		StreamableManager.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateLambda([AssetsToLoad]()
		{
			TSet<FSoftObjectPath>& LoadedCache = GetLoadedSerializedAssetCache();
			for (const FSoftObjectPath& LoadedAsset : AssetsToLoad)
			{
				LoadedCache.Add(LoadedAsset);
			}
		}));
	}
	else
	{
		for (const FSoftObjectPath& Asset : AssetsToLoad)
		{
			Asset.TryLoad();
			GetLoadedSerializedAssetCache().Add(Asset);
		}
	}
}


