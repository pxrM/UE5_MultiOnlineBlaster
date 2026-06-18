#pragma once

#include "Modules/ModuleManager.h"

class FWidgetAnimTimelineModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
