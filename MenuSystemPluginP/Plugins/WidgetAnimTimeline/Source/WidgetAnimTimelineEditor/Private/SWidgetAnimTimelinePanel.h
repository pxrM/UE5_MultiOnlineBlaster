#pragma once

#include "CoreMinimal.h"
#include "Input/CursorReply.h"
#include "Input/Reply.h"
#include "ScopedTransaction.h"
#include "WidgetAnimTimelineSequence.h"
#include "Widgets/SCompoundWidget.h"

class IPropertyHandle;
class UWidgetBlueprint;

class SWidgetAnimTimelinePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWidgetAnimTimelinePanel) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PhaseHandle)
		SLATE_ARGUMENT(UWidgetBlueprint*, WidgetBlueprint)
		SLATE_ARGUMENT(int32, PhaseIndex)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	                      int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;
	// 鼠标悬停条目边缘时改抓取手光标显示
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;
	// 返回true，让面板能接收键盘焦点
	virtual bool SupportsKeyboardFocus() const override;

private:
	struct FEntryViewModel
	{
		int32 EntryIndex = INDEX_NONE;				//指向 FWidgetAnimTimelineConfig 中真实条目的索引，用于回写修改
		FString LaneName;							//所属轨道名（按目标 Widget 名分组），决定条目画在哪一行
		FString Label;								//条目块上显示的文本（如动画名 / 子阶段名）
		FString ValidationError;					//校验错误信息（目标 Widget 不存在、动画缺失等），显示红色警告
		float StartTime = 0.0f;						//条目起始时间（秒），决定条块 X 坐标
		float Duration = 0.5f;						//条目持续时长（秒），决定条块宽度，默认 0.5s
		FLinearColor Color = FLinearColor::White;	//条块颜色，按条目类型或轨道区分
	};

	struct FTimelinePaintContext
	{
		const FGeometry& Geometry;					// 控件几何（坐标变换用）
		FSlateWindowElementList& OutDrawElements;   // 绘制元素输出列表
		const FSlateBrush* WhiteBrush;				// 缓存的白色画刷（FAppStyle 取一次）
		const FSlateFontInfo& SmallFont;			// 缓存的小字体
		FVector2D Size;								// 控件局部尺寸
		float Duration;								// 时间轴总时长
		float TimelineWidth;						// 时间轴区域宽度（不含 header 和 inspector）
		float TimelineRight;						// 时间轴右边界 = HeaderWidth + TimelineWidth
		float LaneAreaBottom;						// 轨道区域底部
		int32 LayerId;								// 起始层深度
	};

	// entry 的块颜色、描边颜色、强调色、副文本颜色
	struct FEntryPaintColors
	{
		FLinearColor Block;
		FLinearColor Outline;
		FLinearColor Accent;
		FLinearColor SecondaryText;
	};

	// entry 的位置和宽度
	struct FEntryPaintLayout
	{
		FVector2D Position;
		float Width = 0.0f;
	};

private:
	void RefreshEntries();
	void RefreshPhaseOptions();
	void RefreshAutoPlayOptions();
	// 画四个底层纯色矩形块，四个不同灰度色块把面板分为 工具栏 → 标尺 → 轨道名区 → 时间轴轨道区
	void PaintTimelineBackground(const FTimelinePaintContext& Context) const;
	// 画时间轴标尺和竖线网格
	void PaintTimelineRuler(const FTimelinePaintContext& Context) const;
	void PaintTimelineLanes(const FTimelinePaintContext& Context) const;
	void PaintTimelineEntries(const FTimelinePaintContext& Context) const;
	static FEntryPaintColors GetEntryPaintColors(const FEntryViewModel& Entry, bool bSelected, bool bActive, bool bHasValidationError);
	void PaintTimelineEntry(const FTimelinePaintContext& Context, const FEntryViewModel& Entry, const FEntryPaintLayout& Layout, const FEntryPaintColors& Colors, bool bSelected, bool bHasValidationError) const;
	FWidgetAnimTimelineConfig* GetTimelineConfig() const;
	FWidgetAnimTimelinePhase* GetTimelinePhase() const;
	UWidgetBlueprint* GetWidgetBlueprint() const;
	UClass* ResolveOwnerWidgetClass() const;
	UClass* ResolveTargetWidgetClass(UWidgetBlueprint* WidgetBlueprint, FName TargetWidgetName) const;
	bool CanEditPhases() const;
	FName MakeUniquePhaseName(const FString& BaseName) const;
	void CommitConfigChange(const FText& TransactionText, TFunctionRef<void(FWidgetAnimTimelineConfig&)> ChangeConfig);
	void MarkWidgetBlueprintModified() const;
	FName GetCurrentPhaseName() const;
	bool HasAnimation(UClass* TargetClass, FName AnimationName) const;
	bool HasChildPhase(UClass* TargetClass, FName TargetWidgetName, FName ChildPhaseName) const;
	float GetEntryDuration(FName TargetWidgetName, EWidgetAnimTimelineEntryType EntryType, FName AnimationName, float PlaybackRate, int32 NumLoopsToPlay) const;
	FString BuildEntryValidationError(FName TargetWidgetName, EWidgetAnimTimelineEntryType EntryType, FName AnimationName, FName ChildPhaseName) const;
	void CommitEntryStartTime(int32 EntryIndex, float NewStartTime);
	bool GetEntrySnapshot(int32 EntryIndex, FWidgetAnimTimelineEntry& OutEntry) const;
	void CommitEntryChange(int32 EntryIndex, const FText& TransactionText, TFunctionRef<void(FWidgetAnimTimelineEntry&)> ChangeEntry);
	void CommitEntryName(int32 EntryIndex, FName PropertyName, FName NewValue, const FText& TransactionText);
	void CommitEntryFloat(int32 EntryIndex, FName PropertyName, float NewValue, const FText& TransactionText);
	void CommitEntryInt(int32 EntryIndex, FName PropertyName, int32 NewValue, const FText& TransactionText);
	void CommitEntryType(int32 EntryIndex, EWidgetAnimTimelineEntryType NewValue, const FText& TransactionText);
	void AddEntry(EWidgetAnimTimelineEntryType EntryType);
	void DeleteEntry(int32 EntryIndex);
	float GetTimelineDuration() const;
	float GetTimelineContentEndTime() const;
	float GetTimelineWidth(const FGeometry& Geometry) const;
	float GetInspectorWidth() const;
	float GetMaxViewStartTime(float TimelineWidth) const;
	void ClampViewStartTime(float TimelineWidth);
	float TimeToX(float Time, const FGeometry& Geometry) const;
	float XToTime(float X, const FGeometry& Geometry) const;
	bool IsInTimelineArea(const FGeometry& Geometry, FVector2D LocalPosition) const;
	int32 HitTestEntry(const FGeometry& Geometry, FVector2D LocalPosition) const;
	int32 GetLaneIndex(const FString& LaneName) const;
	float SnapTime(float Time, bool bUseFineSnap) const;
	static FString FormatTime(float Time);
	TSharedPtr<IPropertyHandle> GetEntriesHandle() const;
	TSharedPtr<IPropertyHandle> GetEntryHandle(int32 EntryIndex) const;
	FText GetPhaseTitle() const;
	FText GetSelectedPhaseText() const;
	FText GetSelectedPhaseNameEditText() const;
	TSharedRef<SWidget> MakePhaseOptionWidget(TSharedPtr<int32> Option) const;
	void OnPhaseSelectionChanged(TSharedPtr<int32> Option, ESelectInfo::Type SelectInfo);
	void OnSelectedPhaseNameCommitted(const FText& NewText, ETextCommit::Type CommitType);
	FReply AddPhase();
	FReply DuplicatePhase();
	FReply DeletePhase();
	bool CanDuplicatePhase() const;
	bool CanDeletePhase() const;
	FText GetSelectedAutoPlayText() const;
	TSharedRef<SWidget> MakeAutoPlayOptionWidget(TSharedPtr<FName> Option) const;
	void OnAutoPlayOptionsOpening();
	void OnAutoPlayChanged(TSharedPtr<FName> Option, ESelectInfo::Type SelectInfo);
	FText GetBlueprintCompileStatusText() const;
	FSlateColor GetBlueprintCompileStatusColor() const;
	FReply PlayDesignerPreview() const;
	FReply FitTimelineToContent();
	EVisibility GetEntryInspectorVisibility() const;
	FText GetSelectedEntryTitleText() const;
	FText GetSelectedEntryTypeText() const;
	FText GetSelectedEntryTargetText() const;
	FText GetSelectedEntryAnimationText() const;
	FText GetSelectedEntryChildPhaseText() const;
	EVisibility GetAnimationFieldVisibility() const;
	EVisibility GetChildPhaseFieldVisibility() const;
	TOptional<float> GetSelectedEntryStartTime() const;
	TOptional<float> GetSelectedEntryPlaybackRate() const;
	TOptional<int32> GetSelectedEntryNumLoops() const;
	static FText GetNameOptionText(TSharedPtr<FName> Option);
	TSharedRef<SWidget> MakeNameOptionWidget(TSharedPtr<FName> Option) const;
	TSharedRef<SWidget> MakeEntryTypeOptionWidget(TSharedPtr<EWidgetAnimTimelineEntryType> Option) const;
	void RefreshTargetOptions();
	void RefreshAnimationOptions();
	void RefreshChildPhaseOptions();
	void OnTargetOptionsOpening();
	void OnAnimationOptionsOpening();
	void OnChildPhaseOptionsOpening();
	void OnSelectedEntryTypeChanged(TSharedPtr<EWidgetAnimTimelineEntryType> Option, ESelectInfo::Type SelectInfo);
	void OnSelectedEntryTargetChanged(TSharedPtr<FName> Option, ESelectInfo::Type SelectInfo);
	void OnSelectedEntryAnimationChanged(TSharedPtr<FName> Option, ESelectInfo::Type SelectInfo);
	void OnSelectedEntryChildPhaseChanged(TSharedPtr<FName> Option, ESelectInfo::Type SelectInfo);
	void OnSelectedEntryStartTimeCommitted(float NewValue, ETextCommit::Type CommitType);
	void OnSelectedEntryPlaybackRateCommitted(float NewValue, ETextCommit::Type CommitType);
	void OnSelectedEntryNumLoopsCommitted(int32 NewValue, ETextCommit::Type CommitType);
	FReply AddDirectAnimationEntry();
	FReply AddChildSequenceEntry();
	FReply DeleteSelectedEntry();
	bool CanDeleteSelectedEntry() const;

private:
	TSharedPtr<IPropertyHandle> PhaseHandle;
	TWeakObjectPtr<UWidgetBlueprint> SourceWidgetBlueprint;
	int32 PhaseIndex = INDEX_NONE;
	TArray<TSharedPtr<int32>> PhaseOptions;
	TSharedPtr<int32> SelectedPhaseOption;
	TArray<TSharedPtr<FName>> AutoPlayOptions;
	TArray<TSharedPtr<EWidgetAnimTimelineEntryType>> EntryTypeOptions;
	TArray<TSharedPtr<FName>> TargetOptions;
	TArray<TSharedPtr<FName>> AnimationOptions;
	TArray<TSharedPtr<FName>> ChildPhaseOptions;
	TArray<FEntryViewModel> Entries;
	TArray<FString> LaneNames;
	int32 SelectedEntryIndex = INDEX_NONE;
	int32 DraggingEntryIndex = INDEX_NONE;
	bool bPanningTimeline = false;
	mutable int32 HoveredEntryIndex = INDEX_NONE;
	TUniquePtr<FScopedTransaction> DragTransaction;
	float DragEntryStartTime = 0.0f;
	float DragMouseStartTime = 0.0f;
	float PanMouseStartX = 0.0f;
	float PanViewStartTime = 0.0f;
	float ViewStartTime = 0.0f;
	float PixelsPerSecond = 120.0f;
	mutable float LastTimelineWidth = 0.0f;
	float LaneHeight = 36.0f;
	float HeaderWidth = 180.0f;
	float TimelineTop = 104.0f;
	float RulerHeight = 28.0f;
	float InspectorWidth = 300.0f;
	float SnapInterval = 0.1f;
};
