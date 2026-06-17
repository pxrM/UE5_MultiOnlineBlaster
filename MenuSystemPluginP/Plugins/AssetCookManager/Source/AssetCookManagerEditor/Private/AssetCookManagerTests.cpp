#include "Misc/AutomationTest.h"

#include "AssetCookRuleManager.h"
#include "Settings/ProjectPackagingSettings.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
	/**
	 * RAII helper: snapshot the packaging dir lists, clear them for a clean test
	 * slate, and restore on scope exit. Operates on the in-memory settings only —
	 * never saves to DefaultGame.ini, so tests have no disk side effects.
	 */
	struct FScopedCookRules
	{
		UProjectPackagingSettings* Settings;
		TArray<FDirectoryPath> SavedAlways;
		TArray<FDirectoryPath> SavedNever;

		FScopedCookRules()
		{
			Settings = GetMutableDefault<UProjectPackagingSettings>();
			SavedAlways = Settings->DirectoriesToAlwaysCook;
			SavedNever = Settings->DirectoriesToNeverCook;
			Settings->DirectoriesToAlwaysCook.Reset();
			Settings->DirectoriesToNeverCook.Reset();
		}

		~FScopedCookRules()
		{
			Settings->DirectoriesToAlwaysCook = SavedAlways;
			Settings->DirectoriesToNeverCook = SavedNever;
		}

		void AddNever(const FString& Path)
		{
			FDirectoryPath Dir;
			Dir.Path = Path;
			Settings->DirectoriesToNeverCook.Add(Dir);
		}

		void AddAlways(const FString& Path)
		{
			FDirectoryPath Dir;
			Dir.Path = Path;
			Settings->DirectoriesToAlwaysCook.Add(Dir);
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetCookPathTest,
	"AssetCookManager.PathConversion",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAssetCookPathTest::RunTest(const FString& Parameters)
{
	FString Rel;

	TestTrue(TEXT("/Game/UI resolves"), FAssetCookRuleManager::ToContentRelative(TEXT("/Game/UI"), Rel));
	TestEqual(TEXT("/Game/UI -> UI"), Rel, FString(TEXT("UI")));

	TestTrue(TEXT("/Game resolves"), FAssetCookRuleManager::ToContentRelative(TEXT("/Game"), Rel));
	TestEqual(TEXT("/Game -> empty"), Rel, FString());

	TestFalse(TEXT("/Engine not under /Game"), FAssetCookRuleManager::ToContentRelative(TEXT("/Engine/X"), Rel));

	TestEqual(TEXT("UI -> /Game/UI"), FAssetCookRuleManager::ToPackagePath(TEXT("UI")), FString(TEXT("/Game/UI")));
	TestEqual(TEXT("empty -> /Game"), FAssetCookRuleManager::ToPackagePath(TEXT("")), FString(TEXT("/Game")));
	TestEqual(TEXT("plugin root passthrough"), FAssetCookRuleManager::ToPackagePath(TEXT("/Interchange/X")), FString(TEXT("/Interchange/X")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetCookInheritTest,
	"AssetCookManager.RuleInheritance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAssetCookInheritTest::RunTest(const FString& Parameters)
{
	FScopedCookRules Rules;
	Rules.AddNever(TEXT("A"));

	TestEqual(TEXT("A is Never"),
		static_cast<int32>(FAssetCookRuleManager::GetRule(TEXT("/Game/A"))),
		static_cast<int32>(ECookRuleType::NeverCook));

	const FResolvedCookRule Child = FAssetCookRuleManager::ResolveRule(TEXT("/Game/A/B"));
	TestEqual(TEXT("child inherits Never"), static_cast<int32>(Child.Rule), static_cast<int32>(ECookRuleType::NeverCook));
	TestTrue(TEXT("child marked inherited"), Child.bInherited);
	TestEqual(TEXT("source dir is /Game/A"), Child.SourceDir, FString(TEXT("/Game/A")));

	// Legacy absolute-form entry ("/Game/C") must also be recognized.
	Rules.AddAlways(TEXT("/Game/C"));
	TestEqual(TEXT("absolute C recognized"),
		static_cast<int32>(FAssetCookRuleManager::GetRule(TEXT("/Game/C"))),
		static_cast<int32>(ECookRuleType::AlwaysCook));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetCookRedundancyTest,
	"AssetCookManager.Redundancy",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FAssetCookRedundancyTest::RunTest(const FString& Parameters)
{
	FScopedCookRules Rules;
	Rules.AddNever(TEXT("A"));
	Rules.AddNever(TEXT("A/B")); // Same as inherited from A — redundant.

	TArray<FString> Redundant;
	FAssetCookRuleManager::GetRedundantDirectories(Redundant);

	TestTrue(TEXT("A/B is redundant"), Redundant.Contains(TEXT("/Game/A/B")));
	TestFalse(TEXT("A is not redundant"), Redundant.Contains(TEXT("/Game/A")));

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
