// P


#include "Utils/UIBlueprintFunctionLibrary.h"

#include "ImageUtils.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Engine/Texture2D.h"
#include "Engine/Texture2DDynamic.h"
#include "Components/Widget.h"
#include "Components/Image.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Materials/MaterialInstanceDynamic.h"

UCanvasPanelSlot* UUIBlueprintFunctionLibrary::GetCanvasPanelSlot(const UWidget* InWidget)
{
	const FString FunctionName(__FUNCTION__);
	if (!InWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The widget is null."), *FunctionName, __LINE__);
		return nullptr;
	}
	if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(InWidget->Slot))
	{
		return Slot;
	}
	UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The widget '%s' is not a child of a CanvasPanel."), *FunctionName, __LINE__, *InWidget->GetName());
    return nullptr;
}

FVector2D UUIBlueprintFunctionLibrary::GetLocalSize(const UWidget* InWidget)
{
	if (InWidget == nullptr)
	{
		const FString FunctionName(__FUNCTION__);
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The widget is null."), *FunctionName, __LINE__);
		return FVector2D::ZeroVector;
	}
	return USlateBlueprintLibrary::GetLocalSize(InWidget->GetCachedGeometry());
}

FVector2D UUIBlueprintFunctionLibrary::GetCanvasSlotSize(const UWidget* InWidget)
{
	if (const UCanvasPanelSlot* Slot = GetCanvasPanelSlot(InWidget))
	{
		return Slot->GetSize();
	}
	return FVector2D::ZeroVector;
}

void UUIBlueprintFunctionLibrary::SetCanvasSlotSize(UWidget* InWidget, const FVector2D& InSize)
{
	if (UCanvasPanelSlot* Slot = GetCanvasPanelSlot(InWidget))
	{
		Slot->SetSize(InSize);
	}
}

FVector2D UUIBlueprintFunctionLibrary::GetCanvasSlotPosition(const UWidget* InWidget)
{
	if (const UCanvasPanelSlot* Slot = GetCanvasPanelSlot(InWidget))
	{
		return Slot->GetPosition();
	}
	return FVector2D::ZeroVector;
}

void UUIBlueprintFunctionLibrary::SetCanvasSlotPosition(UWidget* InWidget, const FVector2D& InPosition)
{
	if (UCanvasPanelSlot* Slot = GetCanvasPanelSlot(InWidget))
	{
		Slot->SetPosition(InPosition);
	}
}

bool UUIBlueprintFunctionLibrary::SetImageBrushResourceObject(UImage* InImage, UObject* InResourceObject, bool InIsMatchSize)
{
	const FString FunctionName(__FUNCTION__);
	if (InImage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The image is null."), *FunctionName, __LINE__);
		return false;
	}
	if(InResourceObject == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The resource object is null."), *FunctionName, __LINE__);
		return false;
	}
	if(!InImage->IsA(UImage::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The widget '%s' is not an Image."), *FunctionName, __LINE__, *InImage->GetName());
		return false;
	}
	
	bool bIsSuccessful = true;
	if (UTexture2D* Texture2D = Cast<UTexture2D>(InResourceObject))
	{
		InImage->SetBrushFromTexture(Texture2D, InIsMatchSize);
	}
	else if(UTexture2DDynamic* TextureDynamic = Cast<UTexture2DDynamic>(InResourceObject))
	{
		InImage->SetBrushFromTextureDynamic(TextureDynamic, InIsMatchSize);
	}
	else if(ISlateTextureAtlasInterface* TextureAtlas = Cast<ISlateTextureAtlasInterface>(InResourceObject))
	{
		InImage->SetBrushFromAtlasInterface(InResourceObject, InIsMatchSize);
	}
	else if(UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(InResourceObject))
	{
		InImage->SetBrushFromMaterial(MaterialInstance);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The resource object '%s' is not a valid texture type."), *FunctionName, __LINE__, *InResourceObject->GetName());
		bIsSuccessful = false;
	}
	return bIsSuccessful;
}

bool UUIBlueprintFunctionLibrary::SetImageMaterialScalarParameterValue(UImage* InImage, const FName& InParameterName,
	const float InValue)
{
	const FString FunctionName(__FUNCTION__);
	if (InImage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The image is null."), *FunctionName, __LINE__);
		return false;
	}
	
	UMaterialInstanceDynamic* DynamicMaterial = InImage->GetDynamicMaterial();
	if (DynamicMaterial == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The DynamicMaterial is null."), *FunctionName, __LINE__);
		return false;
	}

	DynamicMaterial->SetScalarParameterValue(InParameterName, InValue);
	return true;
}

bool UUIBlueprintFunctionLibrary::IsUnderLocation(const UWidget* InWidget, const FVector2D& InAbsoluteCoordinate)
{
	if (InWidget == nullptr)
	{
		const FString FunctionName(__FUNCTION__);
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The widget is null."), *FunctionName, __LINE__);
		return false;
	}
	return USlateBlueprintLibrary::IsUnderLocation(InWidget->GetCachedGeometry(), InAbsoluteCoordinate);
}

FVector2D UUIBlueprintFunctionLibrary::AbsoluteToLocal(const UWidget* InWidget, const FVector2D& InAbsoluteCoordinate)
{
	if (InWidget == nullptr)
	{
		const FString FunctionName(__FUNCTION__);
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The widget is null."), *FunctionName, __LINE__);
		return FVector2D::ZeroVector;
	}
	return USlateBlueprintLibrary::AbsoluteToLocal(InWidget->GetCachedGeometry(), InAbsoluteCoordinate);
}

FVector2D UUIBlueprintFunctionLibrary::LocalToAbsolute(const UWidget* InWidget, const FVector2D& InAbsoluteCoordinate)
{
	if (InWidget == nullptr)
	{
		const FString FunctionName(__FUNCTION__);
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The widget is null."), *FunctionName, __LINE__);
		return FVector2D::ZeroVector;
	}
	return USlateBlueprintLibrary::LocalToAbsolute(InWidget->GetCachedGeometry(), InAbsoluteCoordinate);
}

UTexture2D* UUIBlueprintFunctionLibrary::GetTexture2DFromImage(UImage* InImage)
{
	const FString FunctionName(__FUNCTION__);
	if (InImage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The image is null."), *FunctionName, __LINE__);
		return nullptr;
	}

	const FSlateBrush& Brush = InImage->GetBrush();
	if (Brush.GetResourceObject() == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The resource on image is null."), *FunctionName, __LINE__);
		return nullptr;
	}

	if (Brush.GetResourceObject()->IsA(UTexture2D::StaticClass()) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The resource type on the image is not Texture2D."), *FunctionName, __LINE__);
		return nullptr;
	}

	return Cast<UTexture2D>(Brush.GetResourceObject());
}

UTexture2D* UUIBlueprintFunctionLibrary::CropTexture2D(UTexture2D* SourceTexture, const int32 StartPosX,
	const int32 StartPosY, const int32 CropWidth, const int32 CropHeight)
{
	const FString FunctionName(__FUNCTION__);
	if (SourceTexture == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The texture is null."), *FunctionName, __LINE__);
		return nullptr;
	}

	const int32 SourceWidth = SourceTexture->GetSizeX();
	const int32 SourceHeight = SourceTexture->GetSizeY();

	// 锁定纹理数据
	const FTexture2DMipMap& Mip = SourceTexture->GetPlatformData()->Mips[0];
	const FColor* SourceData = reinterpret_cast<const FColor*>(Mip.BulkData.LockReadOnly());

	// 提取裁剪区域的像素数据
	TArray<FColor> CroppedPixels;
	CroppedPixels.SetNumUninitialized(CropWidth * CropHeight);
	for (int32 Y = 0; Y < CropHeight; Y++)
	{
		const int32 SrcRowStart = (StartPosY + Y) * SourceWidth + StartPosX;
		const int32 DstRowStart = Y * CropWidth;
		FMemory::Memcpy(&CroppedPixels[DstRowStart], &SourceData[SrcRowStart], CropWidth * sizeof(FColor));
	}
	Mip.BulkData.Unlock();

	// 创建新纹理
	UTexture2D* CroppedTexture = UTexture2D::CreateTransient(CropWidth, CropHeight, PF_B8G8R8A8);
	FTexture2DMipMap& NewMip = CroppedTexture->GetPlatformData()->Mips[0];
	void* TextureData = NewMip.BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, CroppedPixels.GetData(), CroppedPixels.Num() * sizeof(FColor));
	NewMip.BulkData.Unlock();

	CroppedTexture->UpdateResource();

	return CroppedTexture;
}

FString UUIBlueprintFunctionLibrary::BuildLocalSavaTexturePath()
{
	const FDateTime Now = FDateTime::Now();
	const FString TimeStr = Now.ToString(TEXT("%Y-%m-%d_%H-%M-%S"));
	FString FilePath = FPaths::ProjectSavedDir() / FString::Printf(TEXT("Image_%s.png"), *TimeStr);
	return FilePath;
}

void UUIBlueprintFunctionLibrary::SaveTextureToDisk(UTexture2D* InTexture, FString InFilePath)
{
	const FString FunctionName(__FUNCTION__);
	if (InTexture == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[%s Line=%d]: The texture is null."), *FunctionName, __LINE__);
		return;
	}
	
	if (InFilePath.IsEmpty())
	{
		InFilePath = BuildLocalSavaTexturePath();
	}
	// 获取像素数据
	const FTexture2DMipMap& Mip = InTexture->GetPlatformData()->Mips[0];
	const int32 Width = InTexture->GetSizeX();
	const int32 Height = InTexture->GetSizeY();
	const FColor* FormattedImageData = static_cast<const FColor*>(Mip.BulkData.LockReadOnly());

	TArray<FColor> Pixels;
	Pixels.AddUninitialized(Width * Height);
	FMemory::Memcpy(Pixels.GetData(), FormattedImageData, Width * Height * sizeof(FColor));
	Mip.BulkData.Unlock();

	// 压缩为PNG
	TArray64<uint8> CompressedPNG;
	FImageUtils::PNGCompressImageArray(Width, Height, Pixels, CompressedPNG);

	// 保存到本地
	FFileHelper::SaveArrayToFile(CompressedPNG, *InFilePath);
}
