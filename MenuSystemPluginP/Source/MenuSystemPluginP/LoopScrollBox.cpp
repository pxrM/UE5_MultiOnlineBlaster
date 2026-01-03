#include "LoopScrollBox.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Widgets/Layout/SBox.h"

/** 内部封装：处理 Tick 和 MouseWheel 转发 */
class SLoopScrollWrapper : public SBox {
public:
	ULoopScrollBox* Owner;
	virtual void Tick(const FGeometry& G, const double T, const float dt) override {
		SBox::Tick(G, T, dt);
		if (Owner) Owner->NativeTick(dt);
	}
	virtual FReply OnMouseWheel(const FGeometry& G, const FPointerEvent& E) override {
		return Owner ? Owner->HandleOnMouseWheel(G, E) : FReply::Unhandled();
	}
};

ULoopScrollBox::ULoopScrollBox() {
	bIsVariable = true;
	Clipping = EWidgetClipping::ClipToBounds;
}

TSharedRef<SWidget> ULoopScrollBox::RebuildWidget() {
	InnerCanvas = NewObject<UCanvasPanel>(this);
	InnerCanvas->SetClipping(EWidgetClipping::ClipToBounds);

	TSharedRef<SLoopScrollWrapper> Root = SNew(SLoopScrollWrapper);
	Root->Owner = this;
	Root->SetContent(InnerCanvas->TakeWidget());

	Root->SetOnMouseButtonDown(FPointerEventHandler::CreateUObject(this, &ULoopScrollBox::HandleOnMouseButtonDown));
	Root->SetOnMouseMove(FPointerEventHandler::CreateUObject(this, &ULoopScrollBox::HandleOnMouseMove));
	Root->SetOnMouseButtonUp(FPointerEventHandler::CreateUObject(this, &ULoopScrollBox::HandleOnMouseButtonUp));

	if (ItemClass) RebuildPool();
	return Root;
}

/*
LogOutputDevice: Error: Ensure condition failed: GExitPurge || !IsGarbageCollecting() [File:D:\build\++UE5\Sync\Engine\Source\Runtime\UMG\Private\Slate\SObjectWidget.cpp] [Line: 43] 
SObjectWidget for 'LoopItemTest_C /Engine/Transient.UnrealEdEngine_0:GameInstance_0.NewWidgetBlueprint_C_0.WidgetTree_0.LoopItemTest_C_8' destroyed while collecting garbage.  This can lead to multiple GCs being required to cleanup the object.  Possible causes might be,
1) ReleaseSlateResources not being implemented for the owner of this pointer.
2) You may just be holding onto some slate pointers on an actor that don't get reset until the actor is Garbage Collected.  You should avoid doing this, and instead reset those references when the actor is Destroyed.
 */
/** 核心物理模拟：借鉴 SScrollBox 的指数衰减与 Overscroll 回弹 */
void ULoopScrollBox::NativeTick(float DeltaTime) {
	const float MaxOff = GetMaxScrollOffset();

	if (!bIsDragging && FMath::Abs(ScrollVelocity) > 0.1f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Inertia Active. Velocity: %f, Offset: %f"), ScrollVelocity, CurrentOffset);
	}
	
	if (bIsAnimating) {
		CurrentOffset = FMath::FInterpTo(CurrentOffset, TargetOffset, DeltaTime, 12.0f);
		if (FMath::IsNearlyEqual(CurrentOffset, TargetOffset, 0.5f)) {
			CurrentOffset = TargetOffset;
			bIsAnimating = false;
		}
	} 
	else if (!bIsDragging) {
		// 1. 边界回弹（如果有过界）
		if (CurrentOffset < -0.1f || CurrentOffset > MaxOff + 0.1f) {
			float Dest = (CurrentOffset < 0) ? 0.0f : MaxOff;
			CurrentOffset = FMath::FInterpTo(CurrentOffset, Dest, DeltaTime, 15.0f);
			ScrollVelocity = 0; // 回弹时不需要惯性
		}
		// 2. 惯性滑动：只要速度大于一个最小值（例如 20 像素/秒）
		else if (FMath::Abs(ScrollVelocity) > 20.0f) {
			// 应用位移
			CurrentOffset += ScrollVelocity * DeltaTime;

			// 应用摩擦力衰减
			ScrollVelocity *= FMath::Exp(-Friction * DeltaTime);

			// 撞墙检测
			if (CurrentOffset <= 0 || CurrentOffset >= MaxOff) {
				CurrentOffset = FMath::Clamp(CurrentOffset, 0.0f, MaxOff);
				ScrollVelocity = 0.0f;
			}
		}
		else {
			// 速度太小了，直接静止
			ScrollVelocity = 0.0f;
		}
	}
	UpdateLayout();
}

/** 核心布局：实现“左上角起点、多排多列、数据无限循环” */
void ULoopScrollBox::UpdateLayout() {
	if (TotalDataCount <= 0 || ItemPool.Num() == 0 || FixedLineCount <= 0) return;

	const float FullW = ItemDimensions.X + Spacing.X;
	const float FullH = ItemDimensions.Y + Spacing.Y;
	const float MainStep = (Orientation == Orient_Vertical) ? FullH : FullW;
	
	// 池子在主轴的总跨度
	const int32 PoolLineCount = ItemPool.Num() / FixedLineCount;
	const float PoolMainRange = PoolLineCount * MainStep;

	for (int32 i = 0; i < ItemPool.Num(); ++i) {
		ULoopScrollItem* Item = ItemPool[i];
		int32 pLine = i / FixedLineCount;
		int32 pCross = i % FixedLineCount;

		// 1. 计算主轴位置（处理环绕）
		float RawMain = (pLine * MainStep) - CurrentOffset;
		RawMain = FMath::Fmod(RawMain + MainStep, PoolMainRange);
		if (RawMain < 0) RawMain += PoolMainRange;
		RawMain -= MainStep;

		// 2. 映射物理坐标（以左上角为起点的 Grid）
		float FinalX = (Orientation == Orient_Vertical) ? (pCross * FullW) : RawMain;
		float FinalY = (Orientation == Orient_Vertical) ? RawMain : (pCross * FullH);

		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Item->Slot)) {
			CanvasSlot->SetPosition(FVector2D(FinalX, FinalY));
		}

		// 3. 计算数据索引：Row * Columns + Column
		int32 LogicLine = FMath::RoundToInt((RawMain + CurrentOffset) / MainStep);
		int32 Row = (Orientation == Orient_Vertical) ? LogicLine : pCross;
		int32 Col = (Orientation == Orient_Vertical) ? pCross : LogicLine;
		
		// 此时 FixedLineCount 在竖向是列数，在横向是行数
		int32 Columns = (Orientation == Orient_Vertical) ? FixedLineCount : 10000; // 横向模式列数逻辑由数据定
		int32 RealDataIdx = (Orientation == Orient_Vertical) ? (LogicLine * FixedLineCount + pCross) : (pCross * 10000 + LogicLine);
		
		// 严谨计算：基于 Row 和 Col 还原 DataIndex
		RealDataIdx = (Orientation == Orient_Vertical) ? (Row * FixedLineCount + Col) : (Col * FixedLineCount + Row);

		if (RealDataIdx >= 0 && RealDataIdx < TotalDataCount && LogicLine >= -1) {
			Item->SetVisibility(ESlateVisibility::Visible);
			if (Item->DataIndex != RealDataIdx) {
				Item->DataIndex = RealDataIdx;
				Item->OnRefreshItem(RealDataIdx);
			}
		} else {
			Item->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

// --- 蓝图接口实现 ---

void ULoopScrollBox::InitList(int32 InCount) {
	TotalDataCount = InCount;
	CurrentOffset = 0;
	ScrollVelocity = 0;
	bIsAnimating = false;
	RebuildPool();
	UpdateLayout();
}

void ULoopScrollBox::ScrollToItemIndex(int32 Index, bool bAnimate) {
	int32 Line = Index / FixedLineCount;
	float Step = (Orientation == Orient_Vertical) ? (ItemDimensions.Y + Spacing.Y) : (ItemDimensions.X + Spacing.X);
	TargetOffset = FMath::Clamp(Line * Step, 0.0f, GetMaxScrollOffset());
	if (bAnimate) bIsAnimating = true; else { CurrentOffset = TargetOffset; UpdateLayout(); }
}

float ULoopScrollBox::GetMaxScrollOffset() const {
	int32 TotalLines = FMath::CeilToInt((float)TotalDataCount / FixedLineCount);
	float Step = (Orientation == Orient_Vertical) ? (ItemDimensions.Y + Spacing.Y) : (ItemDimensions.X + Spacing.X);
	return FMath::Max(0.0f, TotalLines * Step - Spacing.Y);
}

// --- 输入转发 ---

FReply ULoopScrollBox::HandleOnMouseButtonDown(const FGeometry& G, const FPointerEvent& E) {
	if (E.GetEffectingButton() == EKeys::LeftMouseButton) {
		bIsDragging = true; bIsAnimating = false; ScrollVelocity = 0;
		LastMousePos = G.AbsoluteToLocal(E.GetScreenSpacePosition());
		return FReply::Handled().CaptureMouse(TakeWidget());
	}
	return FReply::Unhandled();
}

FReply ULoopScrollBox::HandleOnMouseMove(const FGeometry& G, const FPointerEvent& E) {
	if (bIsDragging)
	{
		FVector2D Cur = G.AbsoluteToLocal(E.GetScreenSpacePosition());
		// 计算当前帧位移
		float Delta = (Orientation == Orient_Vertical) ? (LastMousePos.Y - Cur.Y) : (LastMousePos.X - Cur.X);
        
		// 限制边界
		float MaxOff = GetMaxScrollOffset();
		CurrentOffset = FMath::Clamp(CurrentOffset + Delta, 0.0f, MaxOff);

		UpdateLayout();

		// --- 核心修正：速度采样 ---
		float dt = FApp::GetDeltaTime();
		if (dt > 0)
		{
			// 使用 Lerp 平滑速度，防止单帧采样抖动，确保松手瞬间速度是连续的
			float CurrentFrameVelocity = Delta / dt;
			ScrollVelocity = FMath::Lerp(ScrollVelocity, CurrentFrameVelocity, 0.5f);
		}

		LastMousePos = Cur;
		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply ULoopScrollBox::HandleOnMouseButtonUp(const FGeometry& G, const FPointerEvent& E) {
	if (bIsDragging)
	{
		bIsDragging = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FReply ULoopScrollBox::HandleOnMouseWheel(const FGeometry& G, const FPointerEvent& E) {
	bIsAnimating = false;
	ScrollVelocity -= E.GetWheelDelta() * 1500.0f;
	return FReply::Handled();
}

void ULoopScrollBox::RebuildPool() {
	if (!InnerCanvas || !ItemClass) return;
	InnerCanvas->ClearChildren(); ItemPool.Empty();
	int32 Lines = 6; // 默认视口容纳行数
	int32 PoolSize = (Lines + 2) * FixedLineCount;
	for (int32 i = 0; i < PoolSize; ++i) {
		ULoopScrollItem* NewItem = CreateWidget<ULoopScrollItem>(this, ItemClass);
		ItemPool.Add(NewItem);
		UCanvasPanelSlot* CanvasSlot = InnerCanvas->AddChildToCanvas(NewItem);
		CanvasSlot->SetSize(ItemDimensions);
		CanvasSlot->SetAnchors(FAnchors(0, 0));
	}
}

void ULoopScrollBox::RefreshAllItems()
{
	for (auto* I : ItemPool) I->DataIndex = -1; UpdateLayout();
}
void ULoopScrollBox::AddItemAtLast()
{
	TotalDataCount++; UpdateLayout();
}
void ULoopScrollBox::RemoveItemAtIndex(int32 Index)
{
	TotalDataCount = FMath::Max(0, TotalDataCount - 1); UpdateLayout();
}
void ULoopScrollBox::ReleaseSlateResources(bool b)
{
	Super::ReleaseSlateResources(b); ItemPool.Empty(); InnerCanvas = nullptr;
}
void ULoopScrollBox::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	// 在 UMG 编辑器预览模式下自动创建 Item
	if (IsDesignTime()) {
		if (ItemClass && (ItemPool.Num() == 0 || TotalDataCount == 0)) {
			TotalDataCount = 10; // 设计时预览 10 个
			RebuildPool();
		}
		UpdateLayout();
	}
}