#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class IPropertyHandle;
class UWidgetBlueprint;

class FWidgetAnimTimelineEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void OpenTimelineForWidgetBlueprint(UWidgetBlueprint* WidgetBlueprint, int32 PhaseIndex);
	static void OpenTimelineForPhaseHandle(TSharedPtr<IPropertyHandle> PhaseHandle);
};
