#include "WidgetAnimTimelineEditor.h"

#include "BlueprintEditorContext.h"
#include "BlueprintModes/WidgetBlueprintApplicationModes.h"
#include "Framework/Docking/TabManager.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "SWidgetAnimTimelinePanel.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintEditor.h"
#include "WidgetAnimTimelineEditorUtils.h"
#include "WidgetAnimTimelineEntryCustomization.h"
#include "WidgetAnimTimelinePhaseCustomization.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "FWidgetAnimTimelineEditorModule"

namespace WidgetAnimTimelineEditor
{
	static const FName TimelineTabId(TEXT("WidgetAnimTimelineEditor"));
	static const FName WidgetBlueprintEditorToolbarName(TEXT("AssetEditor.WidgetBlueprintEditor.ToolBar"));
	static const FName ToolbarButtonName(TEXT("OpenWidgetAnimTimeline"));
	static const FName ToolbarSectionName(TEXT("WidgetTools"));
	static TWeakObjectPtr<UWidgetBlueprint> PendingWidgetBlueprint;
	static TSharedPtr<IPropertyHandle> PendingPhaseHandle;
	static TWeakPtr<SDockTab> ActiveTimelineTab;
	static int32 PendingPhaseIndex = 0;

	static TSharedRef<SWidget> CreateTimelineContent()
	{
		if (PendingWidgetBlueprint.IsValid())
		{
			return SNew(SWidgetAnimTimelinePanel)
				.WidgetBlueprint(PendingWidgetBlueprint.Get())
				.PhaseIndex(PendingPhaseIndex);
		}

		return SNew(SWidgetAnimTimelinePanel)
			.PhaseHandle(PendingPhaseHandle);
	}

	static TSharedRef<SDockTab> SpawnTimelineTab(const FSpawnTabArgs& Args)
	{
		TSharedRef<SDockTab> Tab = SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			.Label(LOCTEXT("WidgetAnimTimelineTabLabel", "Widget Anim Timeline"))
			[
				CreateTimelineContent()
			];

		ActiveTimelineTab = Tab;
		Tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateLambda([](TSharedRef<SDockTab> ClosedTab)
		{
			ActiveTimelineTab.Reset();
		}));

		return Tab;
	}

	static void RegisterTimelineTabSpawner()
	{
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(TimelineTabId, FOnSpawnTab::CreateStatic(&SpawnTimelineTab))
			.SetDisplayName(LOCTEXT("WidgetAnimTimelineTabSpawner", "Widget Anim Timeline"))
			.SetTooltipText(LOCTEXT("WidgetAnimTimelineTabSpawnerTooltip", "Open the Widget Anim Timeline editor."))
			.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "Sequencer.TabIcon"))
			.SetMenuType(ETabSpawnerMenuType::Hidden);
	}

	static void UnregisterTimelineTabSpawner()
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TimelineTabId);
		ActiveTimelineTab.Reset();
		PendingPhaseHandle.Reset();
	}

	static void AddToolbarExtension(UToolMenu* Toolbar)
	{
		if (Toolbar == nullptr)
		{
			return;
		}

		FToolMenuSection& Section = Toolbar->FindOrAddSection(ToolbarSectionName);
		Section.InsertPosition = FToolMenuInsert("SourceControl", EToolMenuInsertType::After);
		Section.AddDynamicEntry(ToolbarButtonName, FNewToolMenuSectionDelegate::CreateLambda([](FToolMenuSection& InSection)
		{
			UBlueprintEditorToolMenuContext* Context = InSection.FindContext<UBlueprintEditorToolMenuContext>();
			if (Context == nullptr)
			{
				return;
			}

			TSharedPtr<FWidgetBlueprintEditor> WidgetBlueprintEditor = StaticCastSharedPtr<FWidgetBlueprintEditor>(Context->BlueprintEditor.Pin());
			if (!WidgetBlueprintEditor.IsValid() || WidgetBlueprintEditor->GetWidgetBlueprintObj() == nullptr)
			{
				return;
			}

			TWeakPtr<FWidgetBlueprintEditor> WeakWidgetBlueprintEditor = WidgetBlueprintEditor;
			FToolMenuEntry& Entry = InSection.AddEntry(FToolMenuEntry::InitToolBarButton(
				ToolbarButtonName,
				FUIAction(FExecuteAction::CreateLambda([WeakWidgetBlueprintEditor]()
				{
					TSharedPtr<FWidgetBlueprintEditor> WidgetBlueprintEditor = WeakWidgetBlueprintEditor.Pin();
					if (WidgetBlueprintEditor.IsValid())
					{
						FWidgetAnimTimelineEditorModule::OpenTimelineForWidgetBlueprint(WidgetBlueprintEditor->GetWidgetBlueprintObj(), 0);
					}
				})),
				LOCTEXT("OpenWidgetAnimTimeline", "Widget Timeline"),
				LOCTEXT("OpenWidgetAnimTimelineToolTip", "Open the Widget Anim Timeline editor for this Widget Blueprint."),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Sequencer.TabIcon")
			));
			Entry.InsertPosition = FToolMenuInsert("OpenWidgetReflector", EToolMenuInsertType::After);
		}));
	}

	static void ExtendWidgetBlueprintToolbars()
	{
		const FName DesignerToolbarName = *(WidgetBlueprintEditorToolbarName.ToString() + TEXT(".") + FWidgetBlueprintApplicationModes::DesignerMode.ToString());
		if (!UToolMenus::Get()->IsMenuRegistered(DesignerToolbarName))
		{
			UToolMenus::Get()->RegisterMenu(DesignerToolbarName, WidgetBlueprintEditorToolbarName, EMultiBoxType::ToolBar);
		}

		AddToolbarExtension(UToolMenus::Get()->FindMenu(DesignerToolbarName));
	}

	static void RemoveWidgetBlueprintToolbarExtensions()
	{
		const FName DesignerToolbarName = *(WidgetBlueprintEditorToolbarName.ToString() + TEXT(".") + FWidgetBlueprintApplicationModes::DesignerMode.ToString());
		if (UToolMenus::Get()->IsMenuRegistered(DesignerToolbarName))
		{
			UToolMenus::Get()->RemoveEntry(DesignerToolbarName, ToolbarSectionName, ToolbarButtonName);
		}
	}
}

void FWidgetAnimTimelineEditorModule::OpenTimelineForWidgetBlueprint(UWidgetBlueprint* WidgetBlueprint, int32 PhaseIndex)
{
	if (WidgetBlueprint == nullptr)
	{
		return;
	}

	WidgetAnimTimelineEditor::PendingWidgetBlueprint = WidgetBlueprint;
	WidgetAnimTimelineEditor::PendingPhaseHandle.Reset();
	WidgetAnimTimelineEditor::PendingPhaseIndex = FMath::Max(0, PhaseIndex);

	if (TSharedPtr<SDockTab> ExistingTab = WidgetAnimTimelineEditor::ActiveTimelineTab.Pin())
	{
		ExistingTab->SetContent(WidgetAnimTimelineEditor::CreateTimelineContent());
		ExistingTab->ActivateInParent(ETabActivationCause::SetDirectly);
		return;
	}

	FGlobalTabmanager::Get()->TryInvokeTab(WidgetAnimTimelineEditor::TimelineTabId);
}

void FWidgetAnimTimelineEditorModule::OpenTimelineForPhaseHandle(TSharedPtr<IPropertyHandle> PhaseHandle)
{
	if (!PhaseHandle.IsValid())
	{
		return;
	}

	WidgetAnimTimelineEditor::PendingPhaseHandle = PhaseHandle;
	WidgetAnimTimelineEditor::PendingWidgetBlueprint = FWidgetAnimTimelineEditorUtils::ResolveWidgetBlueprint(PhaseHandle);
	WidgetAnimTimelineEditor::PendingPhaseIndex = PhaseHandle->GetIndexInArray();
	if (WidgetAnimTimelineEditor::PendingPhaseIndex == INDEX_NONE)
	{
		WidgetAnimTimelineEditor::PendingPhaseIndex = 0;
	}

	if (TSharedPtr<SDockTab> ExistingTab = WidgetAnimTimelineEditor::ActiveTimelineTab.Pin())
	{
		ExistingTab->SetContent(WidgetAnimTimelineEditor::CreateTimelineContent());
		ExistingTab->ActivateInParent(ETabActivationCause::SetDirectly);
		return;
	}

	FGlobalTabmanager::Get()->TryInvokeTab(WidgetAnimTimelineEditor::TimelineTabId);
}

void FWidgetAnimTimelineEditorModule::StartupModule()
{
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("WidgetAnimTimelineEntry", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWidgetAnimTimelineEntryCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomPropertyTypeLayout("WidgetAnimTimelinePhase", FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FWidgetAnimTimelinePhaseCustomization::MakeInstance));
	PropertyEditorModule.NotifyCustomizationModuleChanged();

	WidgetAnimTimelineEditor::RegisterTimelineTabSpawner();
	WidgetAnimTimelineEditor::ExtendWidgetBlueprintToolbars();
}

void FWidgetAnimTimelineEditorModule::ShutdownModule()
{
	WidgetAnimTimelineEditor::UnregisterTimelineTabSpawner();

	if (UObjectInitialized() && UToolMenus::IsToolMenuUIEnabled())
	{
		WidgetAnimTimelineEditor::RemoveWidgetBlueprintToolbarExtensions();
	}

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("WidgetAnimTimelineEntry");
		PropertyEditorModule.UnregisterCustomPropertyTypeLayout("WidgetAnimTimelinePhase");
		PropertyEditorModule.NotifyCustomizationModuleChanged();
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FWidgetAnimTimelineEditorModule, WidgetAnimTimelineEditor)
