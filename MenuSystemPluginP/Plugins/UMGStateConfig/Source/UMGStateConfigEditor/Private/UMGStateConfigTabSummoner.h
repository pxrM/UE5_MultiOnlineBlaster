#pragma once

#include "CoreMinimal.h"
#include "WorkflowOrientedApp/WorkflowTabFactory.h"

class FWidgetBlueprintEditor;

struct FUMGStateConfigTabSummoner : public FWorkflowTabFactory
{
public:
	static const FName TabID;

	FUMGStateConfigTabSummoner(TSharedPtr<FWidgetBlueprintEditor> InWidgetEditor);

	virtual TSharedRef<SDockTab> SpawnTab(const FWorkflowTabSpawnInfo& Info) const override;
	virtual TSharedRef<SWidget> CreateTabBody(const FWorkflowTabSpawnInfo& Info) const override;

private:
	TWeakPtr<FWidgetBlueprintEditor> WidgetEditor;
};
