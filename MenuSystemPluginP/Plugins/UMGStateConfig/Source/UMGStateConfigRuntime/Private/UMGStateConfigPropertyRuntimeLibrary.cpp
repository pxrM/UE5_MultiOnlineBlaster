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
#include "Misc/DefaultValueHelper.h"
#include "UObject/UnrealType.h"
#include "UMGStateConfigSettings.h"


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


bool IsSerializedPropertySupported(const FProperty* Property);
bool IsSerializedPropertyPathAllowedByPolicy(const UStruct* RootStruct, const FString& PropertyPath);

bool IsSerializedPropertyPathAllowedInternal(const UWidget* TargetWidget, const FString& PropertyPath)
{
	if (!TargetWidget || PropertyPath.IsEmpty())
	{
		return false;
	}
	return IsSerializedPropertyPathAllowedByPolicy(TargetWidget->GetClass(), PropertyPath);
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

bool IsDangerousTopLevelProperty(const FProperty* Property)
{
	// L3 blacklist: structural / identity / navigation properties are never configurable,
	// even when marked EditAnywhere.
	static const TSet<FName> BlockedNames = {
		FName(TEXT("Slot")),
		FName(TEXT("Navigation")),
		FName(TEXT("bIsVariable")),
		FName(TEXT("bIsVariableOnSpawn")),
		FName(TEXT("bIsEnabledInHierarchy"))
	};
	return Property && BlockedNames.Contains(Property->GetFName());
}

bool IsSerializedPropertyPathAllowedByPolicy(const UStruct* RootStruct, const FString& PropertyPath)
{
	// L2: allow any reflected, editor-exposed property instead of a hardcoded per-class list.
	// Every path segment must pass the L1 safety floor (IsSerializedPropertySupported);
	// the top-level property must be editor-editable (CPF_Edit) and not blacklisted (L3).
	if (!RootStruct || PropertyPath.IsEmpty())
	{
		return false;
	}

	TArray<FString> Segments;
	PropertyPath.ParseIntoArray(Segments, TEXT("."), true);
	if (Segments.Num() == 0)
	{
		return false;
	}

	const UStruct* CurrentStruct = RootStruct;
	const UUMGStateConfigSettings* Settings = GetDefault<UUMGStateConfigSettings>();
	for (int32 SegmentIndex = 0; SegmentIndex < Segments.Num(); ++SegmentIndex)
	{
		if (!CurrentStruct)
		{
			return false;
		}

		const FProperty* Property = CurrentStruct->FindPropertyByName(FName(*Segments[SegmentIndex]));
		if (!Property || !IsSerializedPropertySupported(Property))
		{
			return false;
		}

		if (Settings && Settings->BlockedPropertyNames.Contains(Property->GetFName()))
		{
			return false;
		}

		if (SegmentIndex == 0)
		{
			const bool bExplicitlyAllowed = Settings
				&& (Settings->AdditionalAllowedPropertyNames.Contains(Property->GetFName())
					|| Settings->AdditionalAllowedPropertyPaths.Contains(PropertyPath));
			if (!bExplicitlyAllowed)
			{
				const bool bAllowEditable = !Settings || Settings->bAllowAllEditableProperties;
				if (!bAllowEditable
					|| !Property->HasAnyPropertyFlags(CPF_Edit)
					|| IsDangerousTopLevelProperty(Property))
				{
					return false;
				}
			}
		}

		if (SegmentIndex == Segments.Num() - 1)
		{
			return true;
		}

		const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
		if (!StructProperty)
		{
			return false;
		}
		CurrentStruct = StructProperty->Struct;
	}

	return false;
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

	// 引用资产由 ApplyUIState 上游异步预加载并等待完成，此处不再同步加载，避免主线程卡帧
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

static bool AreSerializedPropertyValuesEquivalent(const FString& A, const FString& B)
{
	if (A == B)
	{
		return true;
	}

	// 浮点精度容差比较：解析为 double 后比较
	double DoubleA, DoubleB;
	if (A.IsNumeric() && B.IsNumeric() && FDefaultValueHelper::ParseDouble(A, DoubleA) && FDefaultValueHelper::ParseDouble(B, DoubleB))
	{
		return FMath::IsNearlyEqual(DoubleA, DoubleB, KINDA_SMALL_NUMBER);
	}

	// 归一化空白后比较（处理尾部空格等差异）
	return A.TrimStartAndEnd().Compare(B.TrimStartAndEnd(), ESearchCase::IgnoreCase) == 0;
}

bool FUMGStateConfigPropertyRuntimeLibrary::ArePropertyValuesEqual(EUMGStateConfigPropertyType PropertyType, const FUMGStateConfigPropertyValue& A, const FUMGStateConfigPropertyValue& B)
{
	if (PropertyType != EUMGStateConfigPropertyType::SerializedProperty)
	{
		return false;
	}

	return A.SerializedPropertyPath == B.SerializedPropertyPath
		&& AreSerializedPropertyValuesEquivalent(A.SerializedPropertyValue, B.SerializedPropertyValue);
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

void FUMGStateConfigPropertyRuntimeLibrary::ResetCaches()
{
	GetSerializedPropertyPathCache().Reset();
}

void FUMGStateConfigPropertyRuntimeLibrary::PreloadReferencedAssets(const TArray<FSoftObjectPath>& ReferencedAssets, bool bAsync)
{
	TArray<FSoftObjectPath> AssetsToLoad;
	for (const FSoftObjectPath& ReferencedAsset : ReferencedAssets)
	{
		if (!ReferencedAsset.IsNull() && ReferencedAsset.ResolveObject() == nullptr)
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
		// 异步预热加载，不阻塞主线程
		FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
		StreamableManager.RequestAsyncLoad(AssetsToLoad);
	}
	else
	{
		for (const FSoftObjectPath& Asset : AssetsToLoad)
		{
			Asset.TryLoad();
		}
	}
}

bool FUMGStateConfigPropertyRuntimeLibrary::AreReferencedAssetsLoaded(const TArray<FSoftObjectPath>& ReferencedAssets)
{
	for (const FSoftObjectPath& ReferencedAsset : ReferencedAssets)
	{
		if (!ReferencedAsset.IsNull() && ReferencedAsset.ResolveObject() == nullptr)
		{
			return false;
		}
	}
	return true;
}

TSharedPtr<FStreamableHandle> FUMGStateConfigPropertyRuntimeLibrary::RequestPreloadReferencedAssetsAsync(const TArray<FSoftObjectPath>& ReferencedAssets, TFunction<void()> OnComplete)
{
	TArray<FSoftObjectPath> AssetsToLoad;
	for (const FSoftObjectPath& ReferencedAsset : ReferencedAssets)
	{
		if (!ReferencedAsset.IsNull() && ReferencedAsset.ResolveObject() == nullptr)
		{
			AssetsToLoad.Add(ReferencedAsset);
		}
	}

	if (AssetsToLoad.Num() == 0)
	{
		if (OnComplete)
		{
			OnComplete();
		}
		return nullptr;
	}

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	return StreamableManager.RequestAsyncLoad(AssetsToLoad, FStreamableDelegate::CreateLambda([OnComplete = MoveTemp(OnComplete)]()
	{
		if (OnComplete)
		{
			OnComplete();
		}
	}));
}
