#pragma once

#include "CoreMinimal.h"
#include "Input/CursorReply.h"
#include "Input/Reply.h"
#include "WidgetAnimTimelineSequence.h"
#include "Widgets/SCompoundWidget.h"

class IPropertyHandle;
class UWidgetBlueprint;

class SWidgetAnimTimelinePanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SWidgetAnimTimelinePanel) {}
		SLATE_ARGUMENT(TSharedPtr<IPropertyHandle>, PhaseHandle)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FCursorReply OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const override;

private:
	struct FEntryViewModel
	{
		int32 EntryIndex = INDEX_NONE;
		FString LaneName;
		FString Label;
		FString ValidationError;
		float StartTime = 0.0f;
		float Duration = 0.5f;
		FLinearColor Color = FLinearColor::White;
	};

	void RefreshEntries();
	UWidgetBlueprint* GetWidgetBlueprint() const;
	UClass* ResolveOwnerWidgetClass() const;
	UClass* ResolveTargetWidgetClass(UWidgetBlueprint* WidgetBlueprint, FName TargetWidgetName) const;
	FName GetCurrentPhaseName() const;
	bool HasAnimation(UClass* TargetClass, FName AnimationName) const;
	bool HasChildPhase(UClass* TargetClass, FName TargetWidgetName, FName ChildPhaseName) const;
	FString BuildEntryValidationError(FName TargetWidgetName, EWidgetAnimTimelineEntryType EntryType, FName AnimationName, FName ChildPhaseName) const;
	void CommitEntryStartTime(int32 EntryIndex, float NewStartTime) const;
	float GetTimelineDuration() const;
	float GetTimelineWidth(const FGeometry& Geometry) const;
	float TimeToX(float Time, const FGeometry& Geometry) const;
	float XToTime(float X, const FGeometry& Geometry) const;
	int32 HitTestEntry(const FGeometry& Geometry, FVector2D LocalPosition) const;
	int32 GetLaneIndex(const FString& LaneName) const;
	float SnapTime(float Time, bool bUseFineSnap) const;
	FString FormatTime(float Time) const;
	TSharedPtr<IPropertyHandle> GetEntriesHandle() const;
	TSharedPtr<IPropertyHandle> GetEntryHandle(int32 EntryIndex) const;
	FText GetPhaseTitle() const;

	TSharedPtr<IPropertyHandle> PhaseHandle;
	TArray<FEntryViewModel> Entries;
	TArray<FString> LaneNames;
	int32 DraggingEntryIndex = INDEX_NONE;
	mutable int32 HoveredEntryIndex = INDEX_NONE;
	float DragEntryStartTime = 0.0f;
	float DragMouseStartTime = 0.0f;
	float PixelsPerSecond = 120.0f;
	float LaneHeight = 36.0f;
	float HeaderWidth = 180.0f;
	float TimelineTop = 56.0f;
	float RulerHeight = 28.0f;
	float SnapInterval = 0.1f;
};
