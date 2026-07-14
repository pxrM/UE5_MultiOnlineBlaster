// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Templates/SubclassOf.h"
#include "UIViewModelWidgetBase.generated.h"

class UUIViewModelBase;

/**
 * Widget base that owns its view model's lifecycle: on init it creates an instance
 * of ViewModelClass, calls Initialize(), and injects it into this widget's MVVM
 * bindings; on destruct it calls Shutdown(). Concrete widgets just set ViewModelClass
 * (in C++ defaults or the WBP) — no manual create/inject wiring per screen.
 *
 * The VM class is declared on the WIDGET (here), not the registry — placement and
 * data binding are separate concerns. See Docs/DESIGN.md section 2.
 *
 * WBP pairing: reparent to a subclass, set ViewModelClass, and in the View Bindings
 * panel add a viewmodel of that class with Creation Type = Manual (this base injects).
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKWIDGETS_API UUIViewModelWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	/** The live view model instance, or nullptr if none was created. */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel")
	UUIViewModelBase* GetViewModel() const { return ViewModel; }

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;

	/** Create the view model instance. Override to customize construction / dependencies. */
	virtual UUIViewModelBase* CreateViewModel();

	/** Wire the instance into MVVM bindings. Override (C++ or BP) for custom injection. */
	UFUNCTION(BlueprintNativeEvent, Category = "UI|ViewModel")
	void InjectViewModel(UUIViewModelBase* InViewModel);
	virtual void InjectViewModel_Implementation(UUIViewModelBase* InViewModel);

	/** View model class this widget creates and binds. Set in defaults or the WBP. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI|ViewModel")
	TSubclassOf<UUIViewModelBase> ViewModelClass;

	/** The owned view model instance. */
	UPROPERTY(BlueprintReadOnly, Category = "UI|ViewModel")
	TObjectPtr<UUIViewModelBase> ViewModel;
};
