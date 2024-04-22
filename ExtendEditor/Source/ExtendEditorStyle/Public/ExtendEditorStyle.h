// ExtendEditorStyle.h

#pragma once

#include "Modules/ModuleInterface.h"


class FExtendEditorStyle : public IModuleInterface
{
public:

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual ~FExtendEditorStyle() {}


private:

	void Initialize();
	TSharedRef<class FSlateStyleSet> CreateSlateStyleSet();


public:
	static EXTENDEDITORSTYLE_API FName GetStyleSetName();


private:
	inline static TSharedPtr<FSlateStyleSet> StyleSet = nullptr;
	inline static const FName StyleSetName = FName("ExtendEditorStyle");

};