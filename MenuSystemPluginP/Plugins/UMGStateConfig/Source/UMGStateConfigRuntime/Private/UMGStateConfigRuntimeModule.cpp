#include "Modules/ModuleManager.h"
#include "UMGStateConfigData.h"

DEFINE_LOG_CATEGORY(LogUMGStateConfig);

class FUMGStateConfigRuntimeModule : public IModuleInterface
{
};

IMPLEMENT_MODULE(FUMGStateConfigRuntimeModule, UMGStateConfigRuntime)
