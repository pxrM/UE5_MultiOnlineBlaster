#pragma once

#include "Modules/ModuleManager.h"

class FWidgetAnimTimelineEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
