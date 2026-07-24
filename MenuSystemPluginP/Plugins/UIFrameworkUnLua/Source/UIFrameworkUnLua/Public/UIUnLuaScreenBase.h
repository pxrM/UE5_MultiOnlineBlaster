// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UnLuaInterface.h"
#include "Widgets/UIActivatableScreenBase.h"
#include "UIUnLuaScreenBase.generated.h"

/**
 * UIFramework screen base with static UnLua binding.
 *
 * Set LuaModuleName on the native or Blueprint class defaults. The path is
 * relative to Content/Script and uses dots, for example UI.MainMenu.
 * UIFramework lifecycle events inherited through IUIManagedWidget can then be
 * implemented directly by the bound Lua module.
 */
UCLASS(Abstract, Blueprintable)
class UIFRAMEWORKUNLUA_API UUIUnLuaScreenBase : public UUIActivatableScreenBase, public IUnLuaInterface
{
	GENERATED_BODY()

public:
	virtual FString GetModuleName_Implementation() const override;

	UFUNCTION(BlueprintPure, Category = "UI|UnLua")
	bool HasLuaModule() const { return !LuaModuleName.IsEmpty(); }

protected:
	/** Module path relative to Content/Script, without the .lua suffix. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "UI|UnLua", meta = (DisplayName = "Lua Module Name"))
	FString LuaModuleName;
};
