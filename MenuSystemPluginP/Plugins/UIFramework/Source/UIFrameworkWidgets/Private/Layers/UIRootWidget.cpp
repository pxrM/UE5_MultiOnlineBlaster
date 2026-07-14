// Copyright TikiStar. All Rights Reserved.

#include "UIRootWidget.h"
#include "UIFrameworkCoreModule.h"
#include "Components/PanelWidget.h"
#include "CommonActivatableWidget.h"

namespace
{
	/** Activate a widget if it is a CommonUI activatable screen (so the action router tracks it). */
	void ActivateIfScreen(UUserWidget* Widget)
	{
		if (UCommonActivatableWidget* Screen = Cast<UCommonActivatableWidget>(Widget))
		{
			Screen->ActivateWidget();
		}
	}

	/** Deactivate a widget if it is a CommonUI activatable screen. */
	void DeactivateIfScreen(UUserWidget* Widget)
	{
		if (UCommonActivatableWidget* Screen = Cast<UCommonActivatableWidget>(Widget))
		{
			Screen->DeactivateWidget();
		}
	}
}

void UUIRootWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	// Seed a stack for every bound layer so lookups never miss.
	for (EUILayer Layer : GetLayersTopDown())
	{
		LayerStacks.FindOrAdd(Layer);
	}
}

UPanelWidget* UUIRootWidget::GetLayerContainer(EUILayer Layer) const
{
	switch (Layer)
	{
	case EUILayer::GameHUD: return Layer_GameHUD;
	case EUILayer::Menu:    return Layer_Menu;
	case EUILayer::Popup:   return Layer_Popup;
	case EUILayer::Modal:   return Layer_Modal;
	case EUILayer::Tooltip: return Layer_Tooltip;
	default:                return nullptr;
	}
}

UUserWidget* UUIRootWidget::PushToLayer(EUILayer Layer, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushToLayer: null WidgetClass."));
		return nullptr;
	}

	UPanelWidget* Container = GetLayerContainer(Layer);
	if (!Container)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushToLayer: layer %d has no bound container."), static_cast<int32>(Layer));
		return nullptr;
	}

	UUserWidget* NewWidget = CreateWidget<UUserWidget>(this, WidgetClass);
	if (!NewWidget)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushToLayer: CreateWidget failed."));
		return nullptr;
	}

	return PushWidget(Layer, NewWidget);
}

UUserWidget* UUIRootWidget::PushWidget(EUILayer Layer, UUserWidget* Widget)
{
	if (!Widget)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushWidget: null widget."));
		return nullptr;
	}

	UPanelWidget* Container = GetLayerContainer(Layer);
	if (!Container)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushWidget: layer %d has no bound container."), static_cast<int32>(Layer));
		return nullptr;
	}

	// Deactivate the screen currently on top of this layer before covering it.
	TArray<TObjectPtr<UUserWidget>>& Stack = LayerStacks.FindOrAdd(Layer);
	if (Stack.Num() > 0)
	{
		DeactivateIfScreen(Stack.Last());
	}

	Container->AddChild(Widget);
	Stack.Add(Widget);

	// Activate if it's a CommonUI screen (drives focus/input via the action router);
	// otherwise focus it directly so gamepad / keyboard navigation lands on it.
	if (Cast<UCommonActivatableWidget>(Widget))
	{
		ActivateIfScreen(Widget);
	}
	else
	{
		Widget->SetFocus();
	}

	return Widget;
}

bool UUIRootWidget::PopFromLayer(EUILayer Layer)
{
	TArray<TObjectPtr<UUserWidget>>* Stack = LayerStacks.Find(Layer);
	if (!Stack || Stack->Num() == 0)
	{
		return false;
	}

	UUserWidget* Top = Stack->Last();
	Stack->Pop();

	if (Top)
	{
		DeactivateIfScreen(Top);
		Top->RemoveFromParent();
	}

	// Re-activate / focus the newly exposed top of this layer, if any.
	if (Stack->Num() > 0)
	{
		if (UUserWidget* NewTop = Stack->Last())
		{
			if (Cast<UCommonActivatableWidget>(NewTop))
			{
				ActivateIfScreen(NewTop);
			}
			else
			{
				NewTop->SetFocus();
			}
		}
	}

	return true;
}

void UUIRootWidget::PopAllFromLayer(EUILayer Layer)
{
	TArray<TObjectPtr<UUserWidget>>* Stack = LayerStacks.Find(Layer);
	if (!Stack)
	{
		return;
	}

	for (int32 i = Stack->Num() - 1; i >= 0; --i)
	{
		if (UUserWidget* Widget = (*Stack)[i])
		{
			Widget->RemoveFromParent();
		}
	}
	Stack->Reset();
}

bool UUIRootWidget::RemoveWidget(EUILayer Layer, UUserWidget* Widget)
{
	if (!Widget)
	{
		return false;
	}

	TArray<TObjectPtr<UUserWidget>>* Stack = LayerStacks.Find(Layer);
	if (!Stack)
	{
		return false;
	}

	const int32 Index = Stack->IndexOfByKey(Widget);
	if (Index == INDEX_NONE)
	{
		return false;
	}

	Stack->RemoveAt(Index);
	Widget->RemoveFromParent();
	return true;
}

UUserWidget* UUIRootWidget::GetTopWidget(EUILayer Layer) const
{
	const TArray<TObjectPtr<UUserWidget>>* Stack = LayerStacks.Find(Layer);
	if (Stack && Stack->Num() > 0)
	{
		return Stack->Last();
	}
	return nullptr;
}

bool UUIRootWidget::IsLayerActive(EUILayer Layer) const
{
	const TArray<TObjectPtr<UUserWidget>>* Stack = LayerStacks.Find(Layer);
	return Stack && Stack->Num() > 0;
}

bool UUIRootWidget::HandleBackAction()
{
	// Highest active layer consumes the back action first.
	for (EUILayer Layer : GetLayersTopDown())
	{
		if (IsLayerActive(Layer))
		{
			return PopFromLayer(Layer);
		}
	}
	return false;
}

const TArray<EUILayer>& UUIRootWidget::GetLayersTopDown()
{
	// Top to bottom: Tooltip is above Modal is above Popup ... GameHUD is bottom.
	static const TArray<EUILayer> Layers = {
		EUILayer::Tooltip,
		EUILayer::Modal,
		EUILayer::Popup,
		EUILayer::Menu,
		EUILayer::GameHUD
	};
	return Layers;
}
