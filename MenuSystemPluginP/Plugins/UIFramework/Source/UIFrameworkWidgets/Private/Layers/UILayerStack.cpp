// Copyright TikiStar. All Rights Reserved.

#include "UILayerStack.h"
#include "UICoverageConfig.h"
#include "Components/PanelWidget.h"
#include "CommonActivatableWidget.h"

namespace
{
	void DeactivateIfScreen(UUserWidget* Widget)
	{
		if (UCommonActivatableWidget* Screen = Cast<UCommonActivatableWidget>(Widget))
		{
			Screen->DeactivateWidget();
		}
	}
}

void UUILayerStack::SetCoverageConfig(const UUICoverageConfig* InConfig)
{
	CoverageConfig = InConfig;
	CheckShow();
}

int32 UUILayerStack::FindInsertIndex(EUILayer Layer) const
{
	// Frames are bottom->top; higher layer tier sits higher. Insert above the last
	// frame whose layer <= this one (same tier => on top, FILO within a layer).
	const uint8 Tier = static_cast<uint8>(Layer);
	int32 Index = Frames.Num();
	for (int32 i = Frames.Num() - 1; i >= 0; --i)
	{
		if (static_cast<uint8>(Frames[i].Layer) <= Tier)
		{
			Index = i + 1;
			break;
		}
		Index = i;
	}
	return Index;
}

UUserWidget* UUILayerStack::Push(UPanelWidget* Container, EUILayer Layer, UUserWidget* Widget, bool bRefresh, bool bBlocksInput, bool bHandlesBack)
{
	if (!Container || !Widget || !IsValidUILayer(Layer))
	{
		return nullptr;
	}

	if (Frames.ContainsByPredicate(
		[Widget](const FUIStackFrame& Frame) { return Frame.Widget == Widget; }))
	{
		return nullptr;
	}

	Container->AddChild(Widget);

	const int32 InsertAt = FindInsertIndex(Layer);
	FUIStackFrame Frame;
	Frame.Widget = Widget;
	Frame.Layer = Layer;
	Frame.bBlocksInput = bBlocksInput;
	Frame.bHandlesBack = bHandlesBack;
	Frame.VisibilityBeforeCoverage = Widget->GetVisibility();
	Frames.Insert(MoveTemp(Frame), InsertAt);

	if (bRefresh)
	{
		CheckShow();
	}
	return Widget;
}

bool UUILayerStack::PopTop(EUILayer Layer, EUIWidgetCloseReason Reason)
{
	for (int32 i = Frames.Num() - 1; i >= 0; --i)
	{
		if (Frames[i].Layer == Layer)
		{
			if (!RemoveFrameAt(i, Reason))
			{
				return false;
			}
			CheckShow();
			return true;
		}
	}
	return false;
}

bool UUILayerStack::Remove(EUILayer Layer, UUserWidget* Widget, EUIWidgetCloseReason Reason)
{
	if (!Widget)
	{
		return false;
	}

	const int32 Index = Frames.IndexOfByPredicate(
		[Layer, Widget](const FUIStackFrame& Frame)
		{
			return Frame.Layer == Layer && Frame.Widget == Widget;
		});
	if (Index == INDEX_NONE)
	{
		return false;
	}

	if (!RemoveFrameAt(Index, Reason))
	{
		return false;
	}
	CheckShow();
	return true;
}

bool UUILayerStack::RemoveFrameAt(int32 Index, EUIWidgetCloseReason Reason)
{
	if (!Frames.IsValidIndex(Index))
	{
		return false;
	}

	if (Frames[Index].bIsClosing)
	{
		return false;
	}

	Frames[Index].bIsClosing = true;
	const TObjectPtr<UUserWidget> Widget = Frames[Index].Widget;
	const EUILayer Layer = Frames[Index].Layer;
	const bool bWasActive = Frames[Index].bIsActive;
	WidgetRemovingEvent.Broadcast(Widget, Layer, Reason);

	Index = Frames.IndexOfByPredicate(
		[Widget](const FUIStackFrame& Frame) { return Frame.Widget == Widget; });
	if (Index == INDEX_NONE)
	{
		return false;
	}
	Frames.RemoveAt(Index);

	if (Widget)
	{
		if (bWasActive)
		{
			DeactivateIfScreen(Widget);
			WidgetActivationChangedEvent.Broadcast(Widget, Layer, false);
		}
		Widget->RemoveFromParent();
		WidgetRemovedEvent.Broadcast(Widget, Layer);
	}
	return true;
}

void UUILayerStack::ClearLayer(EUILayer Layer, EUIWidgetCloseReason Reason)
{
	bool bChanged = false;
	for (int32 i = Frames.Num() - 1; i >= 0; --i)
	{
		if (Frames[i].Layer == Layer)
		{
			bChanged |= RemoveFrameAt(i, Reason);
		}
	}
	if (bChanged)
	{
		CheckShow();
	}
}

void UUILayerStack::ClearAll(EUIWidgetCloseReason Reason)
{
	if (Frames.IsEmpty())
	{
		return;
	}

	for (int32 Index = Frames.Num() - 1; Index >= 0; --Index)
	{
		RemoveFrameAt(Index, Reason);
	}
	CheckShow();
}

bool UUILayerStack::PopTopMost(EUIWidgetCloseReason Reason)
{
	for (int32 Index = Frames.Num() - 1; Index >= 0; --Index)
	{
		const FUIStackFrame& Frame = Frames[Index];
		if (Frame.Widget && Frame.bHandlesBack && !Frame.bHiddenByCoverage
			&& Frame.Widget->GetVisibility() != ESlateVisibility::Collapsed
			&& Frame.Widget->GetVisibility() != ESlateVisibility::Hidden)
		{
			if (!RemoveFrameAt(Index, Reason))
			{
				return false;
			}
			CheckShow();
			return true;
		}
	}
	return false;
}

UUserWidget* UUILayerStack::GetTop(EUILayer Layer) const
{
	for (int32 i = Frames.Num() - 1; i >= 0; --i)
	{
		if (Frames[i].Layer == Layer)
		{
			return Frames[i].Widget;
		}
	}
	return nullptr;
}

UUserWidget* UUILayerStack::GetTopMost() const
{
	return Frames.IsEmpty() ? nullptr : Frames.Last().Widget.Get();
}

bool UUILayerStack::IsLayerActive(EUILayer Layer) const
{
	return GetTop(Layer) != nullptr;
}

bool UUILayerStack::IsWidgetActive(const UUserWidget* Widget) const
{
	const FUIStackFrame* Frame = Frames.FindByPredicate(
		[Widget](const FUIStackFrame& Candidate) { return Candidate.Widget == Widget; });
	return Frame && Frame->bIsActive;
}

void UUILayerStack::CheckShow()
{
	if (bIsRefreshing)
	{
		bRefreshRequested = true;
		return;
	}

	TGuardValue<bool> RefreshGuard(bIsRefreshing, true);
	do
	{
		bRefreshRequested = false;
		CheckShowOnce();
	}
	while (bRefreshRequested);
}

void UUILayerStack::CheckShowOnce()
{
	auto WantsToRender = [](const FUIStackFrame& Frame)
	{
		if (!Frame.Widget)
		{
			return false;
		}
		const ESlateVisibility DesiredVisibility = Frame.bHiddenByCoverage
			? Frame.VisibilityBeforeCoverage
			: Frame.Widget->GetVisibility();
		return DesiredVisibility != ESlateVisibility::Collapsed
			&& DesiredVisibility != ESlateVisibility::Hidden;
	};

	// Phase 1 (mark): a frame is hidden if some VISIBLE frame above it belongs to a
	// layer configured (in the coverage matrix) to cover this frame's layer. Walk
	// top-down so "above is visible" is already resolved when we reach a lower frame.
	TArray<bool, TInlineAllocator<16>> ShouldShow;
	ShouldShow.SetNum(Frames.Num());

	for (int32 i = Frames.Num() - 1; i >= 0; --i)
	{
		bool bHidden = false;
		if (CoverageConfig)
		{
			// Check every frame above this one; only visible covers count.
			for (int32 j = i + 1; j < Frames.Num(); ++j)
			{
				if (ShouldShow[j] && WantsToRender(Frames[j])
					&& CoverageConfig->CanCover(Frames[j].Layer, Frames[i].Layer))
				{
					bHidden = true;
					break;
				}
			}
		}
		ShouldShow[i] = !bHidden;
	}

	int32 TopInteractiveIndex = INDEX_NONE;
	for (int32 i = Frames.Num() - 1; i >= 0; --i)
	{
		if (ShouldShow[i] && Frames[i].bBlocksInput && WantsToRender(Frames[i]))
		{
			TopInteractiveIndex = i;
			break;
		}
	}

	bool bPlainWidgetBecameActive = false;

	// Phase 2 (apply, bottom-up): collapse hidden and restore framework-collapsed
	// frames. Only the overall visible top remains active in CommonUI, otherwise a
	// covered screen can keep consuming input through the action router.
	for (int32 i = 0; i < Frames.Num(); ++i)
	{
		FUIStackFrame& Frame = Frames[i];
		UUserWidget* Widget = Frame.Widget;
		if (!Widget)
		{
			continue;
		}

		if (ShouldShow[i])
		{
			if (Frame.bHiddenByCoverage)
			{
				Widget->SetVisibility(Frame.VisibilityBeforeCoverage);
				Frame.bHiddenByCoverage = false;
			}
		}
		else if (!Frame.bHiddenByCoverage)
		{
			Frame.VisibilityBeforeCoverage = Widget->GetVisibility();
			Widget->SetVisibility(ESlateVisibility::Collapsed);
			Frame.bHiddenByCoverage = true;
		}

		const bool bShouldBeActive = i == TopInteractiveIndex;
		if (Frame.bIsActive != bShouldBeActive)
		{
			if (UCommonActivatableWidget* Screen = Cast<UCommonActivatableWidget>(Widget))
			{
				if (bShouldBeActive)
				{
					Screen->ActivateWidget();
				}
				else
				{
					Screen->DeactivateWidget();
				}
			}

			// Activate/deactivate callbacks are allowed to close or reorder widgets.
			// Do not touch the potentially invalid reference after such a mutation.
			if (!Frames.IsValidIndex(i) || Frames[i].Widget != Widget)
			{
				bRefreshRequested = true;
				return;
			}

			Frame.bIsActive = bShouldBeActive;
			WidgetActivationChangedEvent.Broadcast(Widget, Frame.Layer, bShouldBeActive);
			bPlainWidgetBecameActive = bShouldBeActive && !Cast<UCommonActivatableWidget>(Widget);
		}
	}

	// Plain UUserWidget screens are outside CommonUI's router; focus the visible top.
	if (bPlainWidgetBecameActive && TopInteractiveIndex != INDEX_NONE)
	{
		if (Frames.IsValidIndex(TopInteractiveIndex))
		{
			if (UUserWidget* Top = Frames[TopInteractiveIndex].Widget)
			{
				if (!Cast<UCommonActivatableWidget>(Top) && Top->GetOwningPlayer())
				{
					Top->SetFocus();
				}
			}
		}
	}
}
