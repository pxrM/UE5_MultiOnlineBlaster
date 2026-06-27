// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once
#include "FAVSElement.h"

class FAVS_API FAVSElementSetting : public FAVSElement
{
public:
	virtual void AddRemoveBtn(TSharedRef<SHorizontalBox> _HorizontalBox) override;
	virtual TSharedPtr<SButton> AddOpenAssetBtn(TSharedRef<SHorizontalBox> _HorizontalBox) override;
	virtual FReply OnEyeClicked() override;
	virtual FReply OpenAsset() override;
	virtual FReply CloseAsset() override;
};
