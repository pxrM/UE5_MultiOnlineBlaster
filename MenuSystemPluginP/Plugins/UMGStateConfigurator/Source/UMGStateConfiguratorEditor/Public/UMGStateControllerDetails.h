#pragma once

#include "CoreMinimal.h"
#include "IDetailCustomization.h"

class FUMGStateControllerDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void DrawCategoryUI(TSharedPtr<IPropertyHandle> CategoryHandle, IDetailLayoutBuilder& DetailBuilder);
};