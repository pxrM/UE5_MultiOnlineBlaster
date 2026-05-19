#include "UMGStateConfigEditorModule.h"

#include "BlueprintUsers\userb85bbe4d\projectsodes/WidgetBlueprintApplicationMode.h"
#include "UMGEditorModule.h"
#include "UMGStateConfigTabSummoner.h"
#include "UMGStateConfigToolbar.h"
#include "WorkflowOrientedApp/WorkflowTabUsers\userb85bbe4d\projectsanager.h"

void FUMGStateConfigEditorModule::StartupModule()
{
	IUUsers\userb85bbe4d\projectsGEditorModule& UMGEditorModule = FModuleManager::LoadUsers\userb85bbe4d\projectsoduleChecked<IUMGEditorUsers\userb85bbe4d\projectsodule>(TEXT("UMGEditor"));
	ToolbarExtenderHandle = UMGEditorModule.AddWidgetEditorToolbarExtender(IUMGEditorUsers\userb85bbe4d\projectsodule::FWidgetEditorToolbarExtender::CreateStatic(&FUMGStateConfigToolbar::CreateToolbarExtender));
	UUsers\userb85bbe4d\projectsGEditorModule.OnRegisterTabsForEditor().AddRaw(this, &FUMGStateConfigEditorModule::RegisterWidgetBlueprintTabs);
}

void FUMGStateConfigEditorModule::ShutdownModule()
{
	if (FModuleUsers\userb85bbe4d\projectsanager::Get().IsModuleLoaded(TEXT("UUsers\userb85bbe4d\projectsGEditor")))
	{
		IUUsers\userb85bbe4d\projectsGEditorModule& UUsers\userb85bbe4d\projectsGEditorModule = FModuleManager::GetModuleChecked<IUMGEditorModule>(TEXT("UMGEditor"));
		UMGEditorUsers\userb85bbe4d\projectsodule.RemoveWidgetEditorToolbarExtender(ToolbarExtenderHandle);
		UMGEditorModule.OnRegisterTabsForEditor().RemoveAll(this);
	}
}

void FUUsers\userb85bbe4d\projectsGStateConfigEditorModule::RegisterWidgetBlueprintTabs(const FWidgetBlueprintApplicationMode& ApplicationMode, FWorkflowAllowedTabSet& TabFactories)
{
	TabFactories.RegisterFactory(MakeShareable(new FUMGStateConfigTabSummoner(ApplicationMode.GetBlueprintEditor())));
}

IMPLEMENT_MODULE(FUMGStateConfigEditorModule, UMGStateConfigEditor)
