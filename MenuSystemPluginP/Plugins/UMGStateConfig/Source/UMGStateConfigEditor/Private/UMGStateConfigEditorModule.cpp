#include "UMGStateConfigEditorModule.h"

#include "BlueprintModes/WidgetBlueprintApplicationMode.h"
#include "UMGEditorModule.h"
#include "UMGStateConfigTabSummoner.h"
#include "UMGStateConfigToolbar.h"
#include "WorkflowOrientedApp/WorkflowTabManager.h"

void FUMGStateConfigEditorModule::StartupModule()
{
	IUMGEditorModule& UMGEditorModule = FModuleManager::LoadModuleChecked<IUMGEditorModule>(TEXT("UMGEditor"));
	ToolbarExtenderHandle = UMGEditorModule.AddWidgetEditorToolbarExtender(IUMGEditorModule::FWidgetEditorToolbarExtender::CreateStatic(&FUMGStateConfigToolbar::CreateToolbarExtender));
	UMGEditorModule.OnRegisterTabsForEditor().AddRaw(this, &FUMGStateConfigEditorModule::RegisterWidgetBlueprintTabs);
}

void FUMGStateConfigEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded(TEXT("UMGEditor")))
	{
		IUMGEditorModule& UMGEditorModule = FModuleManager::GetModuleChecked<IUMGEditorModule>(TEXT("UMGEditor"));
		UMGEditorModule.RemoveWidgetEditorToolbarExtender(ToolbarExtenderHandle);
		UMGEditorModule.OnRegisterTabsForEditor().RemoveAll(this);
	}
}

void FUMGStateConfigEditorModule::RegisterWidgetBlueprintTabs(const FWidgetBlueprintApplicationMode& ApplicationMode, FWorkflowAllowedTabSet& TabFactories)
{
	TabFactories.RegisterFactory(MakeShareable(new FUMGStateConfigTabSummoner(ApplicationMode.GetBlueprintEditor())));
}

IMPLEMENT_MODULE(FUMGStateConfigEditorModule, UMGStateConfigEditor)
