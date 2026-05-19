#include "UMGStateConfigToolbar.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "UMGStateConfigTabSummoner.h"
#include "WidgetBlueprintEditor.h"

#define LOCTEXT_NAMESPACE "UMGStateConfigToolbar"

TSharedRef<FExtender> FUMGStateConfigToolbar::CreateToolbarExtender(const TSharedRef<FUICommandList> CommandList, TSharedRef<FWidgetBlueprintEditor> WidgetEditor)
{
	TSharedRef<FExtender> Extender = MakeShared<FExtender>();
	Extender->AddToolBarExtension(
		TEXT("Asset"),
		EExtensionHook::After,
		CommandList,
		FToolBarExtensionDelegate::CreateStatic(&FUMGStateConfigToolbar::FillToolbar, TWeakPtr<FWidgetBlueprintEditor>(WidgetEditor)));
	return Extender;
}

void FUMGStateConfigToolbar::FillToolbar(FToolBarBuilder& ToolbarBuilder, TWeakPtr<FWidgetBlueprintEditor> WidgetEditor)
{
	ToolbarBuilder.AddToolBarButton(
		FUIAction(FExecuteAction::CreateStatic(&FUMGStateConfigToolbar::OpenStateConfigTab, WidgetEditor)),
		NAME_None,
		LOCTEXT("ToolbarButtonLabel", "UI 状态配置"),
		LOCTEXT("ToolbarButtonTooltip", "打开当前 Widget Blueprint 的 UI 状态配置面板"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Settings"));
}

void FUMGStateConfigToolbar::OpenStateConfigTab(TWeakPtr<FWidgetBlueprintEditor> WidgetEditor)
{
	TSharedPtr<FWidgetBlueprintEditor> WidgetEditorPtr = WidgetEditor.Pin();
	if (!WidgetEditorPtr.IsValid() || !WidgetEditorPtr->GetTabManager().IsValid())
	{
		return;
	}

	WidgetEditorPtr->GetTabManager()->TryInvokeTab(FUMGStateConfigTabSummoner::TabID);
}

#undef LOCTEXT_NAMESPACE
