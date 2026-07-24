// Copyright TikiStar. All Rights Reserved.

#include "UIRootWidget.h"
#include "UILayerStack.h"
#include "UISettings.h"
#include "UICoverageConfig.h"
#include "UIFrameworkCoreModule.h"
#include "Components/PanelWidget.h"

void UUIRootWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (!Stack)
	{
		Stack = NewObject<UUILayerStack>(this);
		Stack->OnWidgetRemoved().AddUObject(this, &UUIRootWidget::HandleStackWidgetRemoved);
		Stack->OnWidgetRemoving().AddUObject(this, &UUIRootWidget::HandleStackWidgetRemoving);
		Stack->OnWidgetActivationChanged().AddUObject(this, &UUIRootWidget::HandleStackWidgetActivationChanged);

		// Wire the coverage matrix from settings (may be unset -> no cross-layer hiding).
		if (const UUISettings* Settings = GetDefault<UUISettings>())
		{
			Stack->SetCoverageConfig(Settings->CoverageConfig.LoadSynchronous());
		}
	}
}

UPanelWidget* UUIRootWidget::GetLayerContainer(EUILayer Layer) const
{
	switch (Layer)
	{
	case EUILayer::PersistentSystem: return Layer_PersistentSystem;
	case EUILayer::Loading:          return Layer_Loading;
	case EUILayer::Background:       return Layer_Background;
	case EUILayer::Dock:             return Layer_Dock;
	case EUILayer::FullWindow:       return Layer_FullWindow;
	case EUILayer::PopupWindow:      return Layer_PopupWindow;
	case EUILayer::Guide:            return Layer_Guide;
	case EUILayer::Notification:     return Layer_Notification;
	case EUILayer::Tips:             return Layer_Tips;
	case EUILayer::Max:
	default:                         return nullptr;
	}
}

UUserWidget* UUIRootWidget::PushToLayer(EUILayer Layer, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!WidgetClass)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushToLayer: null WidgetClass."));
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
	UUserWidget* Result = PushWidgetDeferred(Layer, Widget);
	if (Result)
	{
		RefreshStack();
	}
	return Result;
}

UUserWidget* UUIRootWidget::PushWidgetDeferred(EUILayer Layer, UUserWidget* Widget, bool bBlocksInput, bool bHandlesBack)
{
	if (!Widget || !IsValidUILayer(Layer))
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushWidget: null widget or invalid layer %d."), static_cast<int32>(Layer));
		return nullptr;
	}

	UPanelWidget* Container = GetLayerContainer(Layer);
	if (!Container)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushWidget: layer %d has no bound container."), static_cast<int32>(Layer));
		return nullptr;
	}

	return Stack ? Stack->Push(Container, Layer, Widget, false, bBlocksInput, bHandlesBack) : nullptr;
}

void UUIRootWidget::RefreshStack()
{
	if (Stack)
	{
		Stack->Refresh();
	}
}

bool UUIRootWidget::PopFromLayer(EUILayer Layer)
{
	return Stack ? Stack->PopTop(Layer, EUIWidgetCloseReason::LayerPop) : false;
}

void UUIRootWidget::PopAllFromLayer(EUILayer Layer)
{
	if (Stack)
	{
		Stack->ClearLayer(Layer, EUIWidgetCloseReason::LayerClear);
	}
}

bool UUIRootWidget::RemoveWidget(EUILayer Layer, UUserWidget* Widget)
{
	return RemoveWidgetWithReason(Layer, Widget, EUIWidgetCloseReason::Requested);
}

bool UUIRootWidget::RemoveWidgetWithReason(EUILayer Layer, UUserWidget* Widget, EUIWidgetCloseReason Reason)
{
	return Stack ? Stack->Remove(Layer, Widget, Reason) : false;
}

UUserWidget* UUIRootWidget::GetTopWidget(EUILayer Layer) const
{
	return Stack ? Stack->GetTop(Layer) : nullptr;
}

UUserWidget* UUIRootWidget::GetTopMostWidget() const
{
	return Stack ? Stack->GetTopMost() : nullptr;
}

bool UUIRootWidget::IsLayerActive(EUILayer Layer) const
{
	return Stack ? Stack->IsLayerActive(Layer) : false;
}

bool UUIRootWidget::IsWidgetActive(const UUserWidget* Widget) const
{
	return Stack ? Stack->IsWidgetActive(Widget) : false;
}

bool UUIRootWidget::HandleBackAction()
{
	return Stack ? Stack->PopTopMost(EUIWidgetCloseReason::Back) : false;
}

void UUIRootWidget::ClearAllWidgets(EUIWidgetCloseReason Reason)
{
	if (Stack)
	{
		Stack->ClearAll(Reason);
	}
}

void UUIRootWidget::HandleStackWidgetRemoved(UUserWidget* Widget, EUILayer Layer)
{
	WidgetRemovedEvent.Broadcast(Widget, Layer);
}

void UUIRootWidget::HandleStackWidgetRemoving(UUserWidget* Widget, EUILayer Layer, EUIWidgetCloseReason Reason)
{
	WidgetRemovingEvent.Broadcast(Widget, Layer, Reason);
}

void UUIRootWidget::HandleStackWidgetActivationChanged(UUserWidget* Widget, EUILayer Layer, bool bIsActive)
{
	WidgetActivationChangedEvent.Broadcast(Widget, Layer, bIsActive);
}
