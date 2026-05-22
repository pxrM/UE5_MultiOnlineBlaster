#include "SWidgetAnimTimelinePanel.h"

#include "Animation/WidgetAnimation.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "PropertyHandle.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprint.h"
#include "WidgetAnimTimelineSequence.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"

namespace WidgetAnimTimelinePanelConstants
{
	static constexpr float MinPixelsPerSecond = 60.0f;
	static constexpr float MaxPixelsPerSecond = 420.0f;
	static constexpr float BlockMinWidth = 38.0f;
	static constexpr float MajorGridAlpha = 0.34f;
	static constexpr float MinorGridAlpha = 0.11f;

	static bool IsTimeOnStep(float Time, float Step)
	{
		return FMath::IsNearlyZero(FMath::Fmod(Time + KINDA_SMALL_NUMBER, Step), 0.001f);
	}

	static FString FormatRulerTime(float Time)
	{
		if (FMath::IsNearlyEqual(Time, FMath::RoundToFloat(Time), 0.001f))
		{
			return FString::Printf(TEXT("%ds"), FMath::RoundToInt(Time));
		}

		FString Text = FString::Printf(TEXT("%.2fs"), Time);
		Text.ReplaceInline(TEXT("0s"), TEXT("s"));
		return Text;
	}
}

void SWidgetAnimTimelinePanel::Construct(const FArguments& InArgs)
{
	PhaseHandle = InArgs._PhaseHandle;
	RefreshEntries();

	ChildSlot
	[
		SNew(SBorder)
		.Padding(0.0f)
		.BorderImage(FAppStyle::GetBrush(TEXT("ToolPanel.GroupBorder")))
		[
			SNew(SBox)
			.MinDesiredHeight(260.0f)
			.MinDesiredWidth(720.0f)
		]
	];
}

void SWidgetAnimTimelinePanel::RefreshEntries()
{
	Entries.Reset();
	LaneNames.Reset();

	TSharedPtr<IPropertyHandle> EntriesHandle = GetEntriesHandle();
	if (!EntriesHandle.IsValid())
	{
		return;
	}

	uint32 EntryCount = 0;
	EntriesHandle->GetNumChildren(EntryCount);

	for (uint32 EntryIndex = 0; EntryIndex < EntryCount; ++EntryIndex)
	{
		TSharedPtr<IPropertyHandle> EntryHandle = EntriesHandle->GetChildHandle(EntryIndex);
		if (!EntryHandle.IsValid())
		{
			continue;
		}

		TSharedPtr<IPropertyHandle> TargetHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, TargetWidgetName));
		TSharedPtr<IPropertyHandle> TypeHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, EntryType));
		TSharedPtr<IPropertyHandle> AnimationHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, AnimationName));
		TSharedPtr<IPropertyHandle> ChildPhaseHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, ChildPhaseName));
		TSharedPtr<IPropertyHandle> StartHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, StartTime));
		TSharedPtr<IPropertyHandle> RateHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, PlaybackRate));

		FName TargetName = NAME_None;
		FName AnimationName = NAME_None;
		FName ChildPhaseName = NAME_None;
		uint8 TypeValue = 0;
		float StartTime = 0.0f;
		float PlaybackRate = 1.0f;

		if (TargetHandle.IsValid())
		{
			TargetHandle->GetValue(TargetName);
		}
		if (TypeHandle.IsValid())
		{
			TypeHandle->GetValue(TypeValue);
		}
		if (AnimationHandle.IsValid())
		{
			AnimationHandle->GetValue(AnimationName);
		}
		if (ChildPhaseHandle.IsValid())
		{
			ChildPhaseHandle->GetValue(ChildPhaseName);
		}
		if (StartHandle.IsValid())
		{
			StartHandle->GetValue(StartTime);
		}
		if (RateHandle.IsValid())
		{
			RateHandle->GetValue(PlaybackRate);
		}

		FEntryViewModel ViewModel;
		ViewModel.EntryIndex = static_cast<int32>(EntryIndex);
		ViewModel.LaneName = TargetName.IsNone() ? TEXT("Self") : TargetName.ToString();
		ViewModel.StartTime = StartTime;
		ViewModel.Duration = 0.5f / FMath::Max(PlaybackRate, KINDA_SMALL_NUMBER);

		const EWidgetAnimTimelineEntryType EntryType = static_cast<EWidgetAnimTimelineEntryType>(TypeValue);
		ViewModel.ValidationError = BuildEntryValidationError(TargetName, EntryType, AnimationName, ChildPhaseName);
		if (EntryType == EWidgetAnimTimelineEntryType::DirectAnimation)
		{
			ViewModel.Label = AnimationName.IsNone() ? TEXT("Animation") : AnimationName.ToString();
			ViewModel.Color = FLinearColor(0.12f, 0.48f, 0.95f, 1.0f);
		}
		else
		{
			ViewModel.Label = ChildPhaseName.IsNone() ? TEXT("Child Phase") : FString::Printf(TEXT("Phase: %s"), *ChildPhaseName.ToString());
			ViewModel.Color = FLinearColor(0.22f, 0.7f, 0.36f, 1.0f);
		}

		Entries.Add(ViewModel);
		LaneNames.AddUnique(ViewModel.LaneName);
	}
}

int32 SWidgetAnimTimelinePanel::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	int32 CurrentLayer = SCompoundWidget::OnPaint(Args, AllottedGeometry, MyCullingRect, OutDrawElements, LayerId, InWidgetStyle, bParentEnabled);

	const FVector2D Size = AllottedGeometry.GetLocalSize();
	const FSlateBrush* WhiteBrush = FAppStyle::GetBrush(TEXT("WhiteBrush"));
	const FSlateFontInfo SmallFont = FAppStyle::GetFontStyle(TEXT("SmallFont"));
	const FSlateFontInfo NormalFont = FAppStyle::GetFontStyle(TEXT("NormalFont"));
	const float Duration = GetTimelineDuration();
	const int32 LaneCount = LaneNames.Num();
	const float TimelineWidth = GetTimelineWidth(AllottedGeometry);
	const float TimelineRight = HeaderWidth + TimelineWidth;
	const float LaneAreaBottom = FMath::Max(Size.Y - 8.0f, TimelineTop + LaneCount * LaneHeight);

	FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 1, AllottedGeometry.ToPaintGeometry(FVector2f(Size.X, Size.Y), FSlateLayoutTransform(FVector2f::ZeroVector)), WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.018f, 0.018f, 0.018f, 1.0f));
	FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 2, AllottedGeometry.ToPaintGeometry(FVector2f(Size.X, TimelineTop), FSlateLayoutTransform(FVector2f::ZeroVector)), WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.028f, 0.028f, 0.03f, 1.0f));
	FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 3, AllottedGeometry.ToPaintGeometry(FVector2f(HeaderWidth, Size.Y), FSlateLayoutTransform(FVector2f::ZeroVector)), WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.032f, 0.032f, 0.034f, 1.0f));
	FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 4, AllottedGeometry.ToPaintGeometry(FVector2f(TimelineWidth, RulerHeight), FSlateLayoutTransform(FVector2f(HeaderWidth, TimelineTop - RulerHeight))), WhiteBrush, ESlateDrawEffect::None, FLinearColor(0.055f, 0.055f, 0.06f, 1.0f));

	FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 5, AllottedGeometry.ToPaintGeometry(FVector2f(HeaderWidth - 16.0f, 22.0f), FSlateLayoutTransform(FVector2f(10.0f, 8.0f))), GetPhaseTitle(), NormalFont, ESlateDrawEffect::None, FLinearColor(0.88f, 0.88f, 0.88f, 1.0f));
	FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 5, AllottedGeometry.ToPaintGeometry(FVector2f(420.0f, 18.0f), FSlateLayoutTransform(FVector2f(HeaderWidth + 10.0f, 8.0f))), FText::FromString(TEXT("Drag: edit StartTime    MouseWheel: zoom    Shift: 0.01s snap")), SmallFont, ESlateDrawEffect::None, FLinearColor(0.52f, 0.52f, 0.52f, 1.0f));

	const float MinorStep = PixelsPerSecond >= 180.0f ? 0.1f : 0.25f;
	// Labels become denser after zooming so sub-second edits still show concrete ruler time.
	const float LabelStep = PixelsPerSecond >= 300.0f ? 0.1f : PixelsPerSecond >= 180.0f ? 0.25f : PixelsPerSecond >= 100.0f ? 0.5f : 1.0f;
	for (float Time = 0.0f; Time <= Duration + KINDA_SMALL_NUMBER; Time += MinorStep)
	{
		const bool bIsMajor = FMath::IsNearlyZero(FMath::Fmod(Time, 1.0f), 0.001f);
		const bool bIsHalf = !bIsMajor && FMath::IsNearlyZero(FMath::Fmod(Time, 0.5f), 0.001f);
		const bool bDrawLabel = WidgetAnimTimelinePanelConstants::IsTimeOnStep(Time, LabelStep);
		if (!bIsMajor && !bIsHalf && PixelsPerSecond < 180.0f)
		{
			continue;
		}

		const float X = TimeToX(Time, AllottedGeometry);
		if (X < HeaderWidth || X > TimelineRight)
		{
			continue;
		}

		const float TickHeight = bIsMajor ? 11.0f : 6.0f;
		const FLinearColor LineColor = bIsMajor ? FLinearColor(0.45f, 0.45f, 0.45f, WidgetAnimTimelinePanelConstants::MajorGridAlpha) : FLinearColor(0.38f, 0.38f, 0.38f, WidgetAnimTimelinePanelConstants::MinorGridAlpha);
		const TArray<FVector2D> GridLine = { FVector2D(X, TimelineTop - RulerHeight + TickHeight), FVector2D(X, LaneAreaBottom) };
		FSlateDrawElement::MakeLines(OutDrawElements, CurrentLayer + 6, AllottedGeometry.ToPaintGeometry(), GridLine, ESlateDrawEffect::None, LineColor, true, bIsMajor ? 1.0f : 0.5f);

		const TArray<FVector2D> RulerTick = { FVector2D(X, TimelineTop - TickHeight), FVector2D(X, TimelineTop) };
		FSlateDrawElement::MakeLines(OutDrawElements, CurrentLayer + 7, AllottedGeometry.ToPaintGeometry(), RulerTick, ESlateDrawEffect::None, FLinearColor(0.64f, 0.64f, 0.64f, bIsMajor ? 0.78f : 0.28f), true, 1.0f);

		if (bDrawLabel)
		{
			const FLinearColor LabelColor = bIsMajor ? FLinearColor(0.78f, 0.78f, 0.78f, 1.0f) : FLinearColor(0.62f, 0.62f, 0.62f, 0.88f);
			FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 8, AllottedGeometry.ToPaintGeometry(FVector2f(48.0f, 16.0f), FSlateLayoutTransform(FVector2f(X + 4.0f, TimelineTop - RulerHeight + 2.0f))), FText::FromString(WidgetAnimTimelinePanelConstants::FormatRulerTime(Time)), SmallFont, ESlateDrawEffect::None, LabelColor);
		}
	}

	for (int32 LaneIndex = 0; LaneIndex < LaneCount; ++LaneIndex)
	{
		const float Y = TimelineTop + LaneIndex * LaneHeight;
		const FLinearColor LaneColor = LaneIndex % 2 == 0 ? FLinearColor(0.042f, 0.042f, 0.044f, 1.0f) : FLinearColor(0.06f, 0.06f, 0.064f, 1.0f);
		FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 9, AllottedGeometry.ToPaintGeometry(FVector2f(Size.X, LaneHeight - 1.0f), FSlateLayoutTransform(FVector2f(0.0f, Y))), WhiteBrush, ESlateDrawEffect::None, LaneColor);
		FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 10, AllottedGeometry.ToPaintGeometry(FVector2f(HeaderWidth - 16.0f, 18.0f), FSlateLayoutTransform(FVector2f(10.0f, Y + 9.0f))), FText::FromString(LaneNames[LaneIndex]), SmallFont, ESlateDrawEffect::None, FLinearColor(0.82f, 0.82f, 0.82f, 1.0f));
	}

	for (const FEntryViewModel& Entry : Entries)
	{
		const int32 LaneIndex = GetLaneIndex(Entry.LaneName);
		if (LaneIndex == INDEX_NONE)
		{
			continue;
		}

		const float Y = TimelineTop + LaneIndex * LaneHeight;
		const float X = TimeToX(Entry.StartTime, AllottedGeometry);
		const float Width = FMath::Max(WidgetAnimTimelinePanelConstants::BlockMinWidth, Entry.Duration * PixelsPerSecond);
		const bool bActive = Entry.EntryIndex == DraggingEntryIndex || Entry.EntryIndex == HoveredEntryIndex;
		const bool bHasValidationError = !Entry.ValidationError.IsEmpty();
		const FLinearColor ErrorColor(1.0f, 0.14f, 0.08f, 1.0f);
		const FLinearColor BlockColor = bHasValidationError ? FLinearColor(0.42f, 0.06f, 0.04f, bActive ? 1.0f : 0.92f) : (bActive ? Entry.Color.CopyWithNewOpacity(1.0f) : Entry.Color.CopyWithNewOpacity(0.82f));
		const FLinearColor OutlineColor = bHasValidationError ? ErrorColor : (bActive ? FLinearColor(1.0f, 1.0f, 1.0f, 0.62f) : Entry.Color.CopyWithNewOpacity(0.36f));

		FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 12, AllottedGeometry.ToPaintGeometry(FVector2f(Width, LaneHeight - 10.0f), FSlateLayoutTransform(FVector2f(X, Y + 5.0f))), WhiteBrush, ESlateDrawEffect::None, BlockColor);
		FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 13, AllottedGeometry.ToPaintGeometry(FVector2f(Width, 1.0f), FSlateLayoutTransform(FVector2f(X, Y + 5.0f))), WhiteBrush, ESlateDrawEffect::None, OutlineColor);
		if (bHasValidationError)
		{
			FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 13, AllottedGeometry.ToPaintGeometry(FVector2f(Width, 1.0f), FSlateLayoutTransform(FVector2f(X, Y + LaneHeight - 6.0f))), WhiteBrush, ESlateDrawEffect::None, OutlineColor);
			FSlateDrawElement::MakeBox(OutDrawElements, CurrentLayer + 13, AllottedGeometry.ToPaintGeometry(FVector2f(4.0f, LaneHeight - 10.0f), FSlateLayoutTransform(FVector2f(X, Y + 5.0f))), WhiteBrush, ESlateDrawEffect::None, ErrorColor);
		}
		FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 14, AllottedGeometry.ToPaintGeometry(FVector2f(Width - 10.0f, 18.0f), FSlateLayoutTransform(FVector2f(X + 7.0f, Y + 7.0f))), FText::FromString(Entry.Label), SmallFont, ESlateDrawEffect::None, FLinearColor::White);
		FSlateDrawElement::MakeText(OutDrawElements, CurrentLayer + 14, AllottedGeometry.ToPaintGeometry(FVector2f(Width - 10.0f, 14.0f), FSlateLayoutTransform(FVector2f(X + 7.0f, Y + 21.0f))), FText::FromString(bHasValidationError ? Entry.ValidationError : FormatTime(Entry.StartTime)), SmallFont, ESlateDrawEffect::None, bHasValidationError ? FLinearColor(1.0f, 0.82f, 0.76f, 1.0f) : FLinearColor(0.82f, 0.9f, 1.0f, 0.82f));
	}

	return CurrentLayer + 15;
}

UWidgetBlueprint* SWidgetAnimTimelinePanel::GetWidgetBlueprint() const
{
	if (!PhaseHandle.IsValid())
	{
		return nullptr;
	}

	TArray<UObject*> OuterObjects;
	PhaseHandle->GetOuterObjects(OuterObjects);
	for (UObject* OuterObject : OuterObjects)
	{
		for (UObject* Object = OuterObject; Object != nullptr; Object = Object->GetOuter())
		{
			if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Object))
			{
				return WidgetBlueprint;
			}

			if (UClass* Class = Cast<UClass>(Object))
			{
				if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Class->ClassGeneratedBy))
				{
					return WidgetBlueprint;
				}
			}

			if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Object->GetClass()->ClassGeneratedBy))
			{
				return WidgetBlueprint;
			}
		}
	}

	return nullptr;
}

UClass* SWidgetAnimTimelinePanel::ResolveOwnerWidgetClass() const
{
	if (UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint())
	{
		return WidgetBlueprint->GeneratedClass;
	}

	if (!PhaseHandle.IsValid())
	{
		return nullptr;
	}

	TArray<UObject*> OuterObjects;
	PhaseHandle->GetOuterObjects(OuterObjects);
	for (UObject* OuterObject : OuterObjects)
	{
		for (UObject* Object = OuterObject; Object != nullptr; Object = Object->GetOuter())
		{
			if (UClass* Class = Cast<UClass>(Object))
			{
				return Class;
			}

			UClass* ObjectClass = Object->GetClass();
			if (ObjectClass != nullptr && ObjectClass->ClassGeneratedBy != nullptr)
			{
				return ObjectClass;
			}
		}
	}

	return nullptr;
}

UClass* SWidgetAnimTimelinePanel::ResolveTargetWidgetClass(UWidgetBlueprint* WidgetBlueprint, FName TargetWidgetName) const
{
	if (TargetWidgetName.IsNone())
	{
		return ResolveOwnerWidgetClass();
	}

	if (WidgetBlueprint != nullptr && WidgetBlueprint->WidgetTree != nullptr)
	{
		UWidget* TargetWidget = WidgetBlueprint->WidgetTree->FindWidget(TargetWidgetName);
		if (TargetWidget != nullptr && TargetWidget->GetClass()->IsChildOf(UUserWidget::StaticClass()))
		{
			return TargetWidget->GetClass();
		}
	}

	if (UClass* OwnerClass = ResolveOwnerWidgetClass())
	{
		for (TFieldIterator<FObjectProperty> It(OwnerClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
		{
			if (It->GetFName() == TargetWidgetName && It->PropertyClass != nullptr && It->PropertyClass->IsChildOf(UUserWidget::StaticClass()))
			{
				return It->PropertyClass;
			}
		}
	}

	return nullptr;
}

FName SWidgetAnimTimelinePanel::GetCurrentPhaseName() const
{
	if (!PhaseHandle.IsValid())
	{
		return NAME_None;
	}

	TSharedPtr<IPropertyHandle> PhaseNameHandle = PhaseHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, PhaseName));
	FName PhaseName = NAME_None;
	if (PhaseNameHandle.IsValid())
	{
		PhaseNameHandle->GetValue(PhaseName);
	}
	return PhaseName;
}

bool SWidgetAnimTimelinePanel::HasAnimation(UClass* TargetClass, FName AnimationName) const
{
	if (TargetClass == nullptr || AnimationName.IsNone())
	{
		return false;
	}

	for (TFieldIterator<FObjectProperty> It(TargetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->PropertyClass != UWidgetAnimation::StaticClass())
		{
			continue;
		}

		FString PropertyName = It->GetName();
		PropertyName.RemoveFromEnd(TEXT("_INST"));
		if (PropertyName == AnimationName.ToString())
		{
			return true;
		}
	}

	return false;
}

bool SWidgetAnimTimelinePanel::HasChildPhase(UClass* TargetClass, FName TargetWidgetName, FName ChildPhaseName) const
{
	if (TargetClass == nullptr || ChildPhaseName.IsNone())
	{
		return false;
	}

	if (TargetWidgetName.IsNone() && ChildPhaseName == GetCurrentPhaseName())
	{
		return false;
	}

	for (TFieldIterator<FStructProperty> It(TargetClass, EFieldIteratorFlags::IncludeSuper); It; ++It)
	{
		if (It->Struct != FWidgetAnimTimelineConfig::StaticStruct())
		{
			continue;
		}

		UObject* DefaultObject = TargetClass->GetDefaultObject();
		if (DefaultObject == nullptr)
		{
			continue;
		}

		const FWidgetAnimTimelineConfig* Config = It->ContainerPtrToValuePtr<FWidgetAnimTimelineConfig>(DefaultObject);
		if (Config == nullptr)
		{
			continue;
		}

		for (const FWidgetAnimTimelinePhase& Phase : Config->Phases)
		{
			if (Phase.PhaseName == ChildPhaseName)
			{
				return true;
			}
		}
	}

	return false;
}

FString SWidgetAnimTimelinePanel::BuildEntryValidationError(FName TargetWidgetName, EWidgetAnimTimelineEntryType EntryType, FName AnimationName, FName ChildPhaseName) const
{
	UWidgetBlueprint* WidgetBlueprint = GetWidgetBlueprint();
	UClass* TargetClass = ResolveTargetWidgetClass(WidgetBlueprint, TargetWidgetName);
	if (!TargetWidgetName.IsNone() && TargetClass == nullptr)
	{
		return FString::Printf(TEXT("Missing TargetWidgetName: %s"), *TargetWidgetName.ToString());
	}

	if (EntryType == EWidgetAnimTimelineEntryType::DirectAnimation)
	{
		if (AnimationName.IsNone())
		{
			return TEXT("AnimationName is not set");
		}
		if (!HasAnimation(TargetClass, AnimationName))
		{
			return FString::Printf(TEXT("Missing AnimationName: %s"), *AnimationName.ToString());
		}
		return FString();
	}

	if (ChildPhaseName.IsNone())
	{
		return TEXT("ChildPhaseName is not set");
	}
	if (!HasChildPhase(TargetClass, TargetWidgetName, ChildPhaseName))
	{
		return FString::Printf(TEXT("Missing ChildPhaseName: %s"), *ChildPhaseName.ToString());
	}

	return FString();
}

FReply SWidgetAnimTimelinePanel::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const int32 HitEntryIndex = HitTestEntry(MyGeometry, LocalPosition);
	if (HitEntryIndex == INDEX_NONE)
	{
		return FReply::Unhandled();
	}

	const FEntryViewModel* Entry = Entries.FindByPredicate([HitEntryIndex](const FEntryViewModel& Item)
	{
		return Item.EntryIndex == HitEntryIndex;
	});

	if (Entry == nullptr)
	{
		return FReply::Unhandled();
	}

	DraggingEntryIndex = HitEntryIndex;
	DragEntryStartTime = Entry->StartTime;
	DragMouseStartTime = XToTime(LocalPosition.X, MyGeometry);
	return FReply::Handled().CaptureMouse(SharedThis(this));
}

FReply SWidgetAnimTimelinePanel::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (DraggingEntryIndex == INDEX_NONE)
	{
		return FReply::Unhandled();
	}

	DraggingEntryIndex = INDEX_NONE;
	return FReply::Handled().ReleaseMouseCapture();
}

FReply SWidgetAnimTimelinePanel::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (DraggingEntryIndex == INDEX_NONE || !HasMouseCapture())
	{
		return FReply::Unhandled();
	}

	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
	const float CurrentMouseTime = XToTime(LocalPosition.X, MyGeometry);
	const float NewStartTime = SnapTime(FMath::Max(0.0f, DragEntryStartTime + CurrentMouseTime - DragMouseStartTime), MouseEvent.IsShiftDown());

	// Dragging commits directly to the StartTime property, so Details panel and serialized data stay in sync.
	CommitEntryStartTime(DraggingEntryIndex, NewStartTime);
	RefreshEntries();
	return FReply::Handled();
}

FReply SWidgetAnimTimelinePanel::OnMouseWheel(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	const float ZoomFactor = MouseEvent.GetWheelDelta() > 0.0f ? 1.12f : 1.0f / 1.12f;
	PixelsPerSecond = FMath::Clamp(PixelsPerSecond * ZoomFactor, WidgetAnimTimelinePanelConstants::MinPixelsPerSecond, WidgetAnimTimelinePanelConstants::MaxPixelsPerSecond);
	return FReply::Handled();
}

FCursorReply SWidgetAnimTimelinePanel::OnCursorQuery(const FGeometry& MyGeometry, const FPointerEvent& CursorEvent) const
{
	const FVector2D LocalPosition = MyGeometry.AbsoluteToLocal(CursorEvent.GetScreenSpacePosition());
	HoveredEntryIndex = HitTestEntry(MyGeometry, LocalPosition);
	return HoveredEntryIndex != INDEX_NONE ? FCursorReply::Cursor(EMouseCursor::GrabHand) : FCursorReply::Unhandled();
}

void SWidgetAnimTimelinePanel::CommitEntryStartTime(int32 EntryIndex, float NewStartTime) const
{
	TSharedPtr<IPropertyHandle> EntryHandle = GetEntryHandle(EntryIndex);
	if (!EntryHandle.IsValid())
	{
		return;
	}

	TSharedPtr<IPropertyHandle> StartHandle = EntryHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelineEntry, StartTime));
	if (StartHandle.IsValid())
	{
		StartHandle->SetValue(NewStartTime);
	}
}

float SWidgetAnimTimelinePanel::GetTimelineDuration() const
{
	float Duration = 3.0f;
	for (const FEntryViewModel& Entry : Entries)
	{
		Duration = FMath::Max(Duration, Entry.StartTime + Entry.Duration + 0.5f);
	}
	return Duration;
}

float SWidgetAnimTimelinePanel::GetTimelineWidth(const FGeometry& Geometry) const
{
	const float DesiredWidth = GetTimelineDuration() * PixelsPerSecond;
	return FMath::Max(Geometry.GetLocalSize().X - HeaderWidth - 12.0f, DesiredWidth);
}

float SWidgetAnimTimelinePanel::TimeToX(float Time, const FGeometry& Geometry) const
{
	return HeaderWidth + Time * PixelsPerSecond;
}

float SWidgetAnimTimelinePanel::XToTime(float X, const FGeometry& Geometry) const
{
	return FMath::Max(0.0f, (X - HeaderWidth) / PixelsPerSecond);
}

int32 SWidgetAnimTimelinePanel::HitTestEntry(const FGeometry& Geometry, FVector2D LocalPosition) const
{
	for (const FEntryViewModel& Entry : Entries)
	{
		const int32 LaneIndex = GetLaneIndex(Entry.LaneName);
		if (LaneIndex == INDEX_NONE)
		{
			continue;
		}

		const float Y = TimelineTop + LaneIndex * LaneHeight + 4.0f;
		const float X = TimeToX(Entry.StartTime, Geometry);
		const float Width = FMath::Max(WidgetAnimTimelinePanelConstants::BlockMinWidth, Entry.Duration * PixelsPerSecond);
		const FVector2D Min(X, Y);
		const FVector2D Max(X + Width, Y + LaneHeight - 10.0f);

		if (LocalPosition.X >= Min.X && LocalPosition.X <= Max.X && LocalPosition.Y >= Min.Y && LocalPosition.Y <= Max.Y)
		{
			return Entry.EntryIndex;
		}
	}

	return INDEX_NONE;
}

int32 SWidgetAnimTimelinePanel::GetLaneIndex(const FString& LaneName) const
{
	return LaneNames.IndexOfByKey(LaneName);
}

float SWidgetAnimTimelinePanel::SnapTime(float Time, bool bUseFineSnap) const
{
	return FMath::GridSnap(Time, bUseFineSnap ? 0.01f : SnapInterval);
}

FString SWidgetAnimTimelinePanel::FormatTime(float Time) const
{
	return FString::Printf(TEXT("%.2fs"), Time);
}

TSharedPtr<IPropertyHandle> SWidgetAnimTimelinePanel::GetEntriesHandle() const
{
	return PhaseHandle.IsValid() ? PhaseHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, Entries)) : nullptr;
}

TSharedPtr<IPropertyHandle> SWidgetAnimTimelinePanel::GetEntryHandle(int32 EntryIndex) const
{
	TSharedPtr<IPropertyHandle> EntriesHandle = GetEntriesHandle();
	return EntriesHandle.IsValid() ? EntriesHandle->GetChildHandle(static_cast<uint32>(EntryIndex)) : nullptr;
}

FText SWidgetAnimTimelinePanel::GetPhaseTitle() const
{
	if (!PhaseHandle.IsValid())
	{
		return FText::FromString(TEXT("Phase: None"));
	}

	TSharedPtr<IPropertyHandle> PhaseNameHandle = PhaseHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FWidgetAnimTimelinePhase, PhaseName));
	FName PhaseName = NAME_None;
	if (PhaseNameHandle.IsValid())
	{
		PhaseNameHandle->GetValue(PhaseName);
	}

	return FText::FromString(FString::Printf(TEXT("Phase: %s"), PhaseName.IsNone() ? TEXT("None") : *PhaseName.ToString()));
}
