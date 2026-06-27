#include "UMGStateConfigTabSummoner.h"

#include "SUIStateConfigPanel.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/AppStyle.h"
#include "WidgetBlueprintEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "UMGStateConfigTabSummoner"

const FName FUMGStateConfigTabSummoner::TabID(TEXT("UMGStateConfig"));

FUMGStateConfigTabSummoner::FUMGStateConfigTabSummoner(TSharedPtr<FWidgetBlueprintEditor> InWidgetEditor)
	: FWorkflowTabFactory(TabID, InWidgetEditor)
	, WidgetEditor(InWidgetEditor)
{
	TabLabel = LOCTEXT("TabLabel", "UI 状态配置");
	TabIcon = FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings");
	bIsSingleton = true;
	ViewMenuDescription = LOCTEXT("ViewMenuDescription", "UI 状态配置");
	ViewMenuTooltip = LOCTEXT("ViewMenuTooltip", "显示 UI 状态配置面板");
}

TSharedRef<SDockTab> FUMGStateConfigTabSummoner::SpawnTab(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedRef<SDockTab> Tab = FWorkflowTabFactory::SpawnTab(Info);
	Tab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this, &FUMGStateConfigTabSummoner::HandleTabClosed));
	return Tab;
}

TSharedRef<SWidget> FUMGStateConfigTabSummoner::CreateTabBody(const FWorkflowTabSpawnInfo& Info) const
{
	TSharedPtr<FWidgetBlueprintEditor> WidgetEditorPtr = WidgetEditor.Pin();
	return SNew(SUIStateConfigPanel, WidgetEditorPtr);
}

void FUMGStateConfigTabSummoner::HandleTabClosed(TSharedRef<SDockTab> ClosedTab) const
{
	TSharedPtr<FWidgetBlueprintEditor> WidgetEditorPtr = WidgetEditor.Pin();
	if (WidgetEditorPtr.IsValid())
	{
		WidgetEditorPtr->RefreshPreview();
		WidgetEditorPtr->InvalidatePreview(true);
		FSlateApplication::Get().InvalidateAllWidgets(false);
	}
}

#undef LOCTEXT_NAMESPACE
