// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "UIViewModelBase.generated.h"

/**
 * Base view model for the framework's MVVM layer. Extends UMVVMViewModelBase so it
 * plugs into UMG's view binding, and adds Initialize/Shutdown lifecycle hooks.
 *
 * A view model holds UI-facing state + commands and never references concrete
 * widgets — the View binds to it, not the other way round. Use the engine macro
 * UE_MVVM_SET_PROPERTY_VALUE in setters so bound widgets refresh automatically.
 * See Docs/DESIGN.md section 2 (layering).
 *
 * Lives in the Widgets module (not Core) because UMVVMViewModelBase transitively
 * depends on UMG; see Docs/DESIGN.md section 3.
 */
UCLASS(Blueprintable, Abstract)
class UIFRAMEWORKWIDGETS_API UUIViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** Called once after creation. Override to fetch initial data / subscribe to events. */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel")
	virtual void Initialize();

	/** Called before disposal. Override to unsubscribe / release resources. */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel")
	virtual void Shutdown();

	/** True between Initialize and Shutdown. */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel")
	bool IsInitialized() const { return bInitialized; }

protected:
	/** Blueprint hook mirroring Initialize, for view models authored in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|ViewModel", meta = (DisplayName = "On Initialize"))
	void BP_OnInitialize();

	/** Blueprint hook mirroring Shutdown. */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|ViewModel", meta = (DisplayName = "On Shutdown"))
	void BP_OnShutdown();

private:
	bool bInitialized = false;
};
