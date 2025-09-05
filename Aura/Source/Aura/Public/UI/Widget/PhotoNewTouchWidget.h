// P

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PhotoNewTouchWidget.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPhotoNewTouchDelegate, const FVector2D&, ScreenPos);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPhotoTouchMovedDelegate, const FVector2D&, ScreenPos, const FVector2D&, DeltaScreenPos);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPhotoMouseWheelDelegate, const FVector2D&, ScreenPos, const float, Scale);


USTRUCT()
struct FPhotoTouchInfo
{
	GENERATED_BODY()

public:
	FPhotoTouchInfo()
	{ 
	}
	FPhotoTouchInfo(int InPointerIndex, bool InIsTouching, FVector2D InTouchScreenPos)
		: PointerIndex(InPointerIndex), bIsTouching(InIsTouching), TouchScreenPos(InTouchScreenPos)
	{
	}
	bool operator==(const FPhotoTouchInfo& Other) const
	{
		return this->PointerIndex == Other.PointerIndex;
	}

	void Reset()
	{
		//PointerIndex = -1;
		bIsTouching = false;
		TouchScreenPos = FVector2D::ZeroVector;
	}

public:
	int32 PointerIndex;
	bool bIsTouching = false;
	FVector2D TouchScreenPos;

};


/**
 *
 */
UCLASS()
class AURA_API UPhotoNewTouchWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual FReply NativeOnTouchStarted(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent);
	virtual FReply NativeOnTouchMoved(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent);
	virtual FReply NativeOnTouchEnded(const FGeometry& InGeometry, const FPointerEvent& InGestureEvent);
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent);

	void OnZoomed(const FVector2D MidPoint, const float Scale);
	FPhotoTouchInfo* GetTouchInfoByIndex(const int32 PointerIndex);
	FPhotoTouchInfo* GetAnotherTouchInfo(const int32 PointerIndex);
	int32 GetTouchCount() const;
	void ResetTouchData();

public:
	FPhotoNewTouchDelegate TouchStartedCallBack;
	FPhotoTouchMovedDelegate TouchMovedCallBack;
	FPhotoNewTouchDelegate TouchEndedCallBack;
	FPhotoMouseWheelDelegate MouseWheelCallBack;

private:
	bool bIsTouching = false;
	float ZoomDistance = 0.0f;
	TArray<FPhotoTouchInfo> TouchInfos
	{
		FPhotoTouchInfo{0, false, FVector2D::ZeroVector},
		FPhotoTouchInfo{1, false, FVector2D::ZeroVector},
	};
};
