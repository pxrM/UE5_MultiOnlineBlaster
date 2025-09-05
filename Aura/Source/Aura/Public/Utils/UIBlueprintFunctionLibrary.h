// P

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UIBlueprintFunctionLibrary.generated.h"

class UWidget;
class UImage;
class UCanvasPanelSlot;

/**
 * 
 */
UCLASS()
class AURA_API UUIBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static UCanvasPanelSlot* GetCanvasPanelSlot(const UWidget* InWidget);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static FVector2D GetLocalSize(const UWidget* InWidget);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static FVector2D GetCanvasSlotSize(const UWidget* InWidget);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static void SetCanvasSlotSize(UWidget* InWidget, const FVector2D& InSize);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static FVector2D GetCanvasSlotPosition(const UWidget* InWidget);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static void SetCanvasSlotPosition(UWidget* InWidget, const FVector2D& InPosition);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static bool SetImageBrushResourceObject(UImage* InImage, UObject* InResourceObject, bool InIsMatchSize = false);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static bool SetImageMaterialScalarParameterValue(UImage* InImage, const FName& InParameterName, const float InValue);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static bool IsUnderLocation(const UWidget* InWidget, const FVector2D& InAbsoluteCoordinate);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static FVector2D AbsoluteToLocal(const UWidget* InWidget, const FVector2D& InAbsoluteCoordinate);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static FVector2D LocalToAbsolute(const UWidget* InWidget, const FVector2D& InAbsoluteCoordinate);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static UTexture2D* GetTexture2DFromImage(UImage* InImage);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static UTexture2D* CropTexture2D(UTexture2D* SourceTexture, const int32 StartPosX, const int32 StartPosY, const int32 CropWidth, const int32 CropHeight);

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static FString BuildLocalSavaTexturePath();

	UFUNCTION(blueprintCallable, Category = "UI|Utils")
	static void SaveTextureToDisk(UTexture2D* InTexture, FString InFilePath = "");
};
