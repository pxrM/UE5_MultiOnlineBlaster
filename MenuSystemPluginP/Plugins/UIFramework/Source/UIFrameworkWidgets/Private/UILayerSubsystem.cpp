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

	Root->AddToViewport(ZOrder);
	return Root;
}

void UUILayerSubsystem::TeardownRoot()
{
	if (Root)
	{
		Root->RemoveFromParent();
		Root = nullptr;
	}
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

bool UUILayerSubsystem::HandleBackAction()
{
	return Root ? Root->HandleBackAction() : false;
}

void UUILayerSubsystem::Deinitialize()
{
	TeardownRoot();
	Super::Deinitialize();
}
