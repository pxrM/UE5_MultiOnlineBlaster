#pragma once

class ISlateStyle;
class FSlateStyleSet;

class FUMGReflectorStyle
{
public:
	static void Initialize();
	static void Shutdown();
	static void ReloadTextureResources();
	static const ISlateStyle& Get();
	static FName GetStyleName();

private:
	static TSharedPtr<FSlateStyleSet> Create();
	static TSharedPtr<FSlateStyleSet> StyleInstance;
};
