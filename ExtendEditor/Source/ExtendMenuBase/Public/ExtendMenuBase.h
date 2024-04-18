// ExtendMenuBase.h

#pragma once

#include "Modules/ModuleInterface.h"

class FExtendMenuBase : public IModuleInterface
{
public:
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual ~FExtendMenuBase() {}
	
};