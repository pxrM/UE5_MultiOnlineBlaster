#include "Modules/ModuleManager.h"
#include "UMGStateConfigData.h"
#include "UMGStateConfigPropertyRuntimeLibrary.h"

DEFINE_LOG_CATEGORY(LogUMGStateConfig);

class FUMGStateConfigRuntimeModule : public IModuleInterface
{
public:
	virtual void ShutdownModule() override
	{
		// Clear process-wide static caches on module unload
		FUMGStateConfigPropertyRuntimeLibrary::ResetCaches();
	}
};

IMPLEMENT_MODULE(FUMGStateConfigRuntimeModule, UMGStateConfigRuntime)
