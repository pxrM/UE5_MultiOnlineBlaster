#pragma once

#include "CoreMinimal.h"

class FExtender;
class FToolBarBuilder;
class FUICommandList;
class FWidgetBlueprintEditor;

class FUMGStateConfigToolbar
{
public:
	static TSharedRef<FExtender> CreateToolbarExtender(const TSharedRef<FUICommandList> CommandList, TSharedRef<FWidgetBlueprintEditor> WidgetEditor);

private:
	static void FillToolbar(FToolBarBuilder& ToolbarBuilder, TWeakPtr<FWidgetBlueprintEditor> WidgetEditor);
	static void OpenStateConfigTab(TWeakPtr<FWidgetBlueprintEditor> WidgetEditor);
};
