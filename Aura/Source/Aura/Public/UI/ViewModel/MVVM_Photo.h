// P

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_Photo.generated.h"

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSetPhotoTexture, int32, InSizeX, int32, InSizeY, UTexture2D*, PhotoTexture);


/**
 * 
 */
UCLASS()
class AURA_API UMVVM_Photo : public UMVVMViewModelBase
{
	GENERATED_BODY()

protected:


public:
	//UFUNCTION(BlueprintCallable)
	//void OnPhotoButtonPressed();
	//UFUNCTION()
	//void MScreenShot(int32 InSizeX, int32 InSizeY, const TArray<FColor>& InImageData);

public:
	//UPROPERTY(BlueprintAssignable)
	//FSetPhotoTexture SetPhotoTexture;
	
	void SetEditPanelVisibility(const ESlateVisibility InEditPanelVisibility);
	ESlateVisibility GetEditPanelVisibility() const { return EditPanelVisibility; }

	void SetPhotoBrush(const FSlateBrush NewBrush);
	FSlateBrush GetPhotoBrush() const { return PhotoBrush; };

private:
	FDelegateHandle PhotoDelegateHandle;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	ESlateVisibility EditPanelVisibility;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess = "true"))
	FSlateBrush PhotoBrush;

	TArray<FColor> CachedImageData;
	
};
