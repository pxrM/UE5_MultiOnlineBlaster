// Copyright TikiStar. All Rights Reserved.

#include "UILayerSubsystem.h"
#include "UIFrameworkCoreModule.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"

UUILayerSubsystem* UUILayerSubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (UGameInstance* GameInstance = World->GetGameInstance())
	{
		return GameInstance->GetSubsystem<UUILayerSubsystem>();
	}
	return nullptr;
}

UUIRootWidget* UUILayerSubsystem::InitializeRoot(TSubclassOf<UUIRootWidget> RootClass, APlayerController* OwningPlayer, int32 ZOrder)
{
	if (!RootClass)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("InitializeRoot: null RootClass."));
		return nullptr;
	}

	// Replace any existing root.
	TeardownRoot();

	UGameInstance* GameInstance = GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("InitializeRoot: no GameInstance."));
		return nullptr;
	}

	Root = OwningPlayer
		? CreateWidget<UUIRootWidget>(OwningPlayer, RootClass)
		: CreateWidget<UUIRootWidget>(GameInstance, RootClass);

	if (!Root)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("InitializeRoot: CreateWidget failed."));
		return nullptr;
	}

	Root->OnWidgetRemoved().AddUObject(this, &UUILayerSubsystem::HandleRootWidgetRemoved);
	Root->OnWidgetRemoving().AddUObject(this, &UUILayerSubsystem::HandleRootWidgetRemoving);
	Root->OnWidgetActivationChanged().AddUObject(this, &UUILayerSubsystem::HandleRootWidgetActivationChanged);
	Root->AddToViewport(ZOrder);
	return Root;
}

void UUILayerSubsystem::TeardownRoot()
{
	TeardownRootWithReason(EUIWidgetCloseReason::RootTeardown);
}

void UUILayerSubsystem::TeardownRootWithReason(EUIWidgetCloseReason Reason)
{
	if (Root)
	{
		Root->ClearAllWidgets(Reason);
		Root->OnWidgetRemoved().RemoveAll(this);
		Root->OnWidgetRemoving().RemoveAll(this);
		Root->OnWidgetActivationChanged().RemoveAll(this);
		Root->RemoveFromParent();
		Root = nullptr;
	}
}

void UUILayerSubsystem::HandleRootWidgetRemoved(UUserWidget* Widget, EUILayer Layer)
{
	WidgetRemovedEvent.Broadcast(Widget, Layer);
}

void UUILayerSubsystem::HandleRootWidgetRemoving(UUserWidget* Widget, EUILayer Layer, EUIWidgetCloseReason Reason)
{
	WidgetRemovingEvent.Broadcast(Widget, Layer, Reason);
}

void UUILayerSubsystem::HandleRootWidgetActivationChanged(UUserWidget* Widget, EUILayer Layer, bool bIsActive)
{
	WidgetActivationChangedEvent.Broadcast(Widget, Layer, bIsActive);
}

UUserWidget* UUILayerSubsystem::PushToLayer(EUILayer Layer, TSubclassOf<UUserWidget> WidgetClass)
{
	if (!Root)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushToLayer: no root. Initialize the root first."));
		return nullptr;
	}
	return Root->PushToLayer(Layer, WidgetClass);
}

UUserWidget* UUILayerSubsystem::PushWidget(EUILayer Layer, UUserWidget* Widget)
{
	if (!Root)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushWidget: no root. Initialize the root first."));
		return nullptr;
	}
	return Root->PushWidget(Layer, Widget);
}

UUserWidget* UUILayerSubsystem::PushWidgetDeferred(EUILayer Layer, UUserWidget* Widget, bool bBlocksInput, bool bHandlesBack)
{
	if (!Root)
	{
		UE_LOG(LogUIFramework, Warning, TEXT("PushWidgetDeferred: no root. Initialize the root first."));
		return nullptr;
	}
	return Root->PushWidgetDeferred(Layer, Widget, bBlocksInput, bHandlesBack);
}

void UUILayerSubsystem::RefreshStack()
{
	if (Root)
	{
		Root->RefreshStack();
	}
}

bool UUILayerSubsystem::PopFromLayer(EUILayer Layer)
{
	return Root ? Root->PopFromLayer(Layer) : false;
}

void UUILayerSubsystem::PopAllFromLayer(EUILayer Layer)
{
	if (Root)
	{
		Root->PopAllFromLayer(Layer);
	}
}

bool UUILayerSubsystem::RemoveWidget(EUILayer Layer, UUserWidget* Widget)
{
	return Root ? Root->RemoveWidget(Layer, Widget) : false;
}

bool UUILayerSubsystem::RemoveWidgetWithReason(EUILayer Layer, UUserWidget* Widget, EUIWidgetCloseReason Reason)
{
	return Root ? Root->RemoveWidgetWithReason(Layer, Widget, Reason) : false;
}

bool UUILayerSubsystem::IsWidgetActive(const UUserWidget* Widget) const
{
	return Root ? Root->IsWidgetActive(Widget) : false;
}

bool UUILayerSubsystem::HandleBackAction()
{
	return Root ? Root->HandleBackAction() : false;
}

void UUILayerSubsystem::Deinitialize()
{
	TeardownRootWithReason(EUIWidgetCloseReason::SubsystemShutdown);
	Super::Deinitialize();
}
