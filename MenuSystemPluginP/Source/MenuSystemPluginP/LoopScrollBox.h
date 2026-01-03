// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"
#include "LoopScrollBox.generated.h"


class UCanvasPanel;
class UInvalidationBox;  // 用于缓存

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoopScrollItemClicked, int32, DataIndex);

UCLASS(Abstract, Blueprintable)
class MENUSYSTEMPLUGINP_API ULoopScrollItem : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, Category = "LoopScroll")
	int32 DataIndex = -1;

	UPROPERTY(BlueprintAssignable, Category = "LoopScroll")
	FOnLoopScrollItemClicked OnItemClicked;

	// 在蓝图里，把按钮的 OnClicked 连到这个函数上
	UFUNCTION(BlueprintCallable, Category = "LoopScroll")
	virtual void NotifyClick()
	{
		if (OnItemClicked.IsBound())
		{
			OnItemClicked.Broadcast(DataIndex);
		}
	}

	UFUNCTION(BlueprintImplementableEvent, Category = "LoopScroll")
	void OnRefreshItem(int32 NewIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "LoopScroll")
	void PlayIntroAnimation(float Delay);

	UFUNCTION(BlueprintImplementableEvent, Category = "LoopScroll")
	void OnItemReleased();
};


UCLASS()
class MENUSYSTEMPLUGINP_API ULoopScrollBox : public UWidget
{
    GENERATED_BODY()

public:
    ULoopScrollBox();

 // --- 核心配置 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	TEnumAsByte<EOrientation> Orientation = Orient_Vertical;

	/** 垂直滚动时为列数，水平滚动时为行数 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	int32 FixedLineCount = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D ItemDimensions = FVector2D(200.0f, 200.0f);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Layout")
	FVector2D Spacing = FVector2D(10.0f, 10.0f);

	// --- 物理模拟 (借鉴 SScrollBox) ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scroll")
	float Friction = 8.0f; // 阻尼系数

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scroll")
	bool bAllowOverscroll = true; // 是否允许边缘拉伸回弹

	UPROPERTY(EditAnywhere, Category = "Content")
	TSubclassOf<ULoopScrollItem> ItemClass;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLoopScrollItemClicked OnItemClicked;

	// --- 蓝图接口 (CRUD) ---
	UFUNCTION(BlueprintCallable, Category = "LoopScroll")
	void InitList(int32 InDataCount);

	UFUNCTION(BlueprintCallable, Category = "LoopScroll")
	void RefreshAllItems();

	UFUNCTION(BlueprintCallable, Category = "LoopScroll")
	void AddItemAtLast();

	UFUNCTION(BlueprintCallable, Category = "LoopScroll")
	void RemoveItemAtIndex(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "LoopScroll")
	void ScrollToItemIndex(int32 Index, bool bAnimate = true);

	UFUNCTION(BlueprintCallable, Category = "LoopScroll")
	void ClearList() { InitList(0); }
	void NativeTick(float DeltaTime);

	FReply HandleOnMouseWheel(const FGeometry& G, const FPointerEvent& E);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	
	// 输入处理 (借鉴 SScrollBox 逻辑)
	FReply HandleOnMouseButtonDown(const FGeometry& G, const FPointerEvent& E);
	FReply HandleOnMouseMove(const FGeometry& G, const FPointerEvent& E);
	FReply HandleOnMouseButtonUp(const FGeometry& G, const FPointerEvent& E);
	

private:
	void UpdateLayout();
	void RebuildPool();
	float GetMaxScrollOffset() const;
	
	UFUNCTION()
	void HandleItemClicked(int32 Idx) { OnItemClicked.Broadcast(Idx); }

	UPROPERTY()
	UCanvasPanel* InnerCanvas;

	UPROPERTY()
	TArray<ULoopScrollItem*> ItemPool;

	int32 TotalDataCount = 0;
	float CurrentOffset = 0.0f;
	float ScrollVelocity = 0.0f;
	float TargetOffset = 0.0f;
	bool bIsDragging = false;
	bool bIsAnimating = false;
	FVector2D LastMousePos;
};