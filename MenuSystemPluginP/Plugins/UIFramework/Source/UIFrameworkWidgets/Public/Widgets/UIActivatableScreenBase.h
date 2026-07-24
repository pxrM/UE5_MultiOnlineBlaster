// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "Input/CommonUIInputTypes.h"
#include "Templates/SubclassOf.h"
#include "Management/UIManagedWidget.h"
#include "UIActivatableScreenBase.generated.h"

class UWidget;
class UUIViewModelBase;

/**
 * Base class for full-screen / panel UI. Derives from UCommonActivatableWidget so
 * CommonUI's action router gives it back handling, gamepad focus and input routing —
 * WITHOUT using CommonUI's activatable stack. The framework's own UUIRootWidget keeps
 * the layer stacks and activates/deactivates these screens on push/pop.
 *
 * Also hosts a view model (same reconstruct-safe lifecycle as
 * UUIViewModelWidgetBase, shared via UIFrameworkVM). Use this for screens; use
 * UUIViewModelWidgetBase for leaf widgets that don't need activation.
 *
 * WBP pairing: reparent the screen WBP to a subclass; set ViewModelClass, optionally
 * bind DesiredFocusWidget, and choose InputMode.
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUIActivatableScreenBase : public UCommonActivatableWidget, public IUIManagedWidget
{
	GENERATED_BODY()

public:
	/** The live view model instance, or nullptr if none was created. */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel")
	UUIViewModelBase* GetViewModel() const { return ViewModel; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// CommonUI: input routing while active. Focus target uses the inherited
	// DesiredFocusWidget (bind a widget named "DesiredFocusWidget" in the WBP).
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;

	/** Create the view model instance. Override to customize construction / dependencies. */
	virtual UUIViewModelBase* CreateViewModel();

	/** Wire the instance into MVVM bindings. Override (C++ or BP) for custom injection. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|ViewModel")
	void InjectViewModel(UUIViewModelBase* InViewModel);
	virtual void InjectViewModel_Implementation(UUIViewModelBase* InViewModel);

	/** View model class this screen creates and binds. Set in defaults or the WBP. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|ViewModel")
	TSubclassOf<UUIViewModelBase> ViewModelClass;

	/** The owned view model instance. */
	UPROPERTY(BlueprintReadOnly, Category = "UI|ViewModel")
	TObjectPtr<UUIViewModelBase> ViewModel;

	/** Input routing mode applied by the action router while this screen is active. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|Input")
	ECommonInputMode InputMode = ECommonInputMode::Menu;
};
