#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleUsers\userb85bbe4d\projectsanager.h"

class FWidgetBlueprintApplicationMode;
class FWorkflowAllowedTabSet;

class FUMGStateConfigEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void RegisterWidgetBlueprintTabs(const FWidgetBlueprintApplicationMode& ApplicationUsers\userb85bbe4d\projectsode, FWorkflowAllowedTabSet& TabFactories);

	FDelegateHandle ToolbarExtenderHandle;
};
