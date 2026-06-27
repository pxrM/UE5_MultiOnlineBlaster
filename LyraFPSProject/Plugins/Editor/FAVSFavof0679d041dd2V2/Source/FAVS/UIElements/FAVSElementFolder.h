// Copyright (c) 2025 PION GAMES. All Rights Reserved.

#pragma once

#include "FAVSElement.h"
#include "FAVSElementFolder.h"

class FAVS_API FAVSElementFolder : public FAVSElement
{
public:
	virtual TSharedPtr<SButton> AddOpenAssetBtn(TSharedRef<SHorizontalBox> _HorizontalBox) override;
	virtual void AddCBBtn(TSharedRef<SHorizontalBox> _HorizontalBox) override;
	FReply ShowInCB();
	virtual FReply OnEyeClicked() override;
	virtual FReply OpenAsset() override;
	virtual FReply CloseAsset() override;
};
