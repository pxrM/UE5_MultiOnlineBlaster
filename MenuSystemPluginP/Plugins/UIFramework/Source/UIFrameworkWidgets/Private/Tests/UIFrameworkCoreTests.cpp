#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Components/CanvasPanel.h"
#include "CommonUserWidget.h"
#include "Engine/GameInstance.h"

#include "UICoverageConfig.h"
#include "UILayerStack.h"
#include "UIManagerSubsystem.h"
#include "UIWidgetCache.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkCoverageConfigTest,
	"TikiStar.UIFramework.Core.CoverageConfig",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkCoverageConfigTest::RunTest(const FString& Parameters)
{
	UUICoverageConfig* Config = NewObject<UUICoverageConfig>();
	Config->Rules.Add({ EUILayer::Dock, EUILayer::FullWindow });
	Config->Rules.Add({ EUILayer::FullWindow, EUILayer::PopupWindow });

	TestTrue(TEXT("FullWindow covers Dock"), Config->CanCover(EUILayer::FullWindow, EUILayer::Dock));
	TestTrue(TEXT("PopupWindow covers FullWindow"), Config->CanCover(EUILayer::PopupWindow, EUILayer::FullWindow));
	TestFalse(TEXT("Coverage is directional"), Config->CanCover(EUILayer::Dock, EUILayer::FullWindow));
	TestFalse(TEXT("Unconfigured layers remain visible"), Config->CanCover(EUILayer::Notification, EUILayer::PopupWindow));
	TestTrue(TEXT("Tips is a valid layer"), IsValidUILayer(EUILayer::Tips));
	TestFalse(TEXT("Max is only a sentinel"), IsValidUILayer(EUILayer::Max));
	TArray<FString> ValidationErrors;
	Config->ValidateRules(ValidationErrors);
	TestTrue(TEXT("Valid coverage rules pass validation"), ValidationErrors.IsEmpty());
	Config->Rules.Add({ EUILayer::Dock, EUILayer::Dock });
	Config->Rules.Add({ EUILayer::Dock, EUILayer::FullWindow });
	ValidationErrors.Reset();
	Config->ValidateRules(ValidationErrors);
	TestEqual(TEXT("Coverage validation reports self and duplicate rules"), ValidationErrors.Num(), 2);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkWidgetCacheTest,
	"TikiStar.UIFramework.Core.WidgetCache",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkWidgetCacheTest::RunTest(const FString& Parameters)
{
	UUIWidgetCache* Cache = NewObject<UUIWidgetCache>();
	FUIWidgetEntry Entry;
	Entry.CachePolicy = EUICachePolicy::KeepPersistent;
	Entry.WidgetClass = UCommonUserWidget::StaticClass();

	const FGameplayTag Key;
	UUserWidget* First = NewObject<UCommonUserWidget>();
	UUserWidget* Second = NewObject<UCommonUserWidget>();
	UUserWidget* Third = NewObject<UCommonUserWidget>();

	Cache->ReturnInstance(Key, First, Entry);
	Cache->ReturnInstance(Key, Second, Entry);
	TestEqual(TEXT("Multiple closed instances are retained per key"), Cache->GetNumInstances(Key), 2);

	Cache->ReturnInstance(Key, Second, Entry);
	TestEqual(TEXT("Returning the same instance twice does not duplicate it"), Cache->GetNumInstances(Key), 2);
	TestTrue(TEXT("Cache is LIFO"), Cache->TakeInstance(Key, Entry) == Second);
	TestTrue(TEXT("Older instance remains available"), Cache->TakeInstance(Key, Entry) == First);
	TestEqual(TEXT("Taking every instance removes the bucket"), Cache->GetNumInstances(Key), 0);

	Entry.MaxCachedInstances = 4;
	Cache->ReturnInstance(Key, First, Entry);
	Cache->ReturnInstance(Key, Second, Entry);
	Entry.MaxCachedInstances = 1;
	Cache->ReturnInstance(Key, Third, Entry);
	TestEqual(TEXT("Cache respects the per-key instance limit"), Cache->GetNumInstances(Key), 1);
	TestTrue(TEXT("Cache limit keeps the newest instance"), Cache->TakeInstance(Key, Entry) == Third);

	Entry.MaxCachedInstances = 8;
	Entry.WidgetClass = UCommonUserWidget::StaticClass();
	Cache->ReturnInstance(Key, First, Entry);
	Entry.WidgetClass = UUserWidget::StaticClass();
	TestNull(TEXT("Cache rejects an instance from a different WidgetClass"), Cache->TakeInstance(Key, Entry));

	Entry.CachePolicy = EUICachePolicy::Transient;
	Cache->ReturnInstance(Key, First, Entry);
	TestEqual(TEXT("Transient instances are not retained"), Cache->GetNumInstances(Key), 0);

	Entry.CachePolicy = EUICachePolicy::KeepPersistent;
	Cache->ReturnInstance(Key, First, Entry);
	Cache->ResolveClass(Key, Entry);
	Cache->Remove(Key);
	TestEqual(TEXT("Per-key invalidation removes closed instances"), Cache->GetNumInstances(Key), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkRuntimeRegistryTest,
	"TikiStar.UIFramework.Core.RuntimeRegistry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkRuntimeRegistryTest::RunTest(const FString& Parameters)
{
	UGameInstance* GameInstance = NewObject<UGameInstance>();
	UUIManagerSubsystem* Manager = NewObject<UUIManagerSubsystem>(GameInstance);
	const FGameplayTag Key = FGameplayTag::RequestGameplayTag(TEXT("UI.Test"), false);
	TestTrue(TEXT("Test GameplayTag is configured"), Key.IsValid());

	FUIWidgetEntry Entry;
	Entry.WidgetClass = UCommonUserWidget::StaticClass();
	Entry.Layer = EUILayer::PopupWindow;
	Entry.CachePolicy = EUICachePolicy::KeepPersistent;
	FString Error;
	TestTrue(TEXT("A valid runtime entry registers"), Manager->RegisterRuntimeEntry(Key, Entry, true, Error));
	TestTrue(TEXT("Successful registration has no error"), Error.IsEmpty());
	TestEqual(TEXT("Runtime entry count increments"), Manager->GetRuntimeEntryCount(), 1);

	Entry.Layer = EUILayer::Max;
	TestFalse(TEXT("An invalid runtime entry is rejected"), Manager->RegisterRuntimeEntry(Key, Entry, true, Error));
	TestTrue(TEXT("Rejected registration explains the failure"), !Error.IsEmpty());
	TestEqual(TEXT("Rejected replacement preserves the previous entry"), Manager->GetRuntimeEntryCount(), 1);

	TestTrue(TEXT("Runtime entry can be removed"), Manager->UnregisterRuntimeEntry(Key));
	TestEqual(TEXT("Runtime entry count returns to zero"), Manager->GetRuntimeEntryCount(), 0);
	TestFalse(TEXT("Removing a missing runtime entry reports false"), Manager->UnregisterRuntimeEntry(Key));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkLayerStackTest,
	"TikiStar.UIFramework.Core.LayerStack",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkLayerStackTest::RunTest(const FString& Parameters)
{
	UUICoverageConfig* Config = NewObject<UUICoverageConfig>();
	Config->Rules.Add({ EUILayer::Dock, EUILayer::FullWindow });

	UUILayerStack* Stack = NewObject<UUILayerStack>();
	Stack->SetCoverageConfig(Config);

	UCanvasPanel* DockPanel = NewObject<UCanvasPanel>();
	UCanvasPanel* FullWindowPanel = NewObject<UCanvasPanel>();
	UCanvasPanel* TipsPanel = NewObject<UCanvasPanel>();
	UUserWidget* DockWidget = NewObject<UCommonUserWidget>();
	UUserWidget* FullWindowWidget = NewObject<UCommonUserWidget>();
	UUserWidget* TipsWidget = NewObject<UCommonUserWidget>();
	const ESlateVisibility DockInitialVisibility = DockWidget->GetVisibility();

	int32 RemovalCount = 0;
	int32 ActivationChangeCount = 0;
	UUserWidget* LastRemoved = nullptr;
	UUserWidget* LastActivated = nullptr;
	EUIWidgetCloseReason LastCloseReason = EUIWidgetCloseReason::Requested;
	Stack->OnWidgetRemoving().AddLambda(
		[&LastCloseReason](UUserWidget* Widget, EUILayer Layer, EUIWidgetCloseReason Reason)
		{
			LastCloseReason = Reason;
		});
	Stack->OnWidgetRemoved().AddLambda(
		[&RemovalCount, &LastRemoved](UUserWidget* Widget, EUILayer Layer)
		{
			++RemovalCount;
			LastRemoved = Widget;
		});
	Stack->OnWidgetActivationChanged().AddLambda(
		[&ActivationChangeCount, &LastActivated](UUserWidget* Widget, EUILayer Layer, bool bIsActive)
		{
			++ActivationChangeCount;
			if (bIsActive)
			{
				LastActivated = Widget;
			}
		});

	TestTrue(TEXT("Dock pushes successfully"), Stack->Push(DockPanel, EUILayer::Dock, DockWidget) == DockWidget);
	TestTrue(TEXT("FullWindow pushes successfully"), Stack->Push(FullWindowPanel, EUILayer::FullWindow, FullWindowWidget) == FullWindowWidget);
	TestEqual(TEXT("Two frames are tracked"), Stack->Num(), 2);
	TestEqual(TEXT("FullWindow coverage collapses Dock"), DockWidget->GetVisibility(), ESlateVisibility::Collapsed);
	TestTrue(TEXT("Overall top is FullWindow"), Stack->GetTopMost() == FullWindowWidget);
	TestTrue(TEXT("FullWindow is the active frame"), Stack->IsWidgetActive(FullWindowWidget));
	TestFalse(TEXT("Covered Dock is inactive"), Stack->IsWidgetActive(DockWidget));
	TestTrue(TEXT("Non-blocking Tips pushes successfully"),
		Stack->Push(TipsPanel, EUILayer::Tips, TipsWidget, true, false, false) == TipsWidget);
	TestTrue(TEXT("Tips is visually top-most"), Stack->GetTopMost() == TipsWidget);
	TestTrue(TEXT("Non-blocking Tips does not steal activation"), Stack->IsWidgetActive(FullWindowWidget));
	TestNull(TEXT("A widget cannot be pushed twice"), Stack->Push(DockPanel, EUILayer::Dock, DockWidget));
	TestNull(TEXT("Max cannot be pushed"), Stack->Push(FullWindowPanel, EUILayer::Max, NewObject<UCommonUserWidget>()));

	TestTrue(TEXT("Back pops the overall top"), Stack->PopTopMost());
	TestTrue(TEXT("Back skips Tips that does not handle Back"), Stack->GetTopMost() == TipsWidget);
	TestEqual(TEXT("Back reason reaches Removing"), LastCloseReason, EUIWidgetCloseReason::Back);
	TestTrue(TEXT("Removal event identifies the popped widget"), LastRemoved == FullWindowWidget);
	TestEqual(TEXT("One removal event fired"), RemovalCount, 1);
	TestEqual(TEXT("Dock restores its initial visibility"), DockWidget->GetVisibility(), DockInitialVisibility);
	TestTrue(TEXT("Dock reactivates after Back"), LastActivated == DockWidget && Stack->IsWidgetActive(DockWidget));

	DockWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	TestTrue(TEXT("FullWindow can be pushed again"), Stack->Push(FullWindowPanel, EUILayer::FullWindow, FullWindowWidget) == FullWindowWidget);
	TestTrue(TEXT("Second back succeeds"), Stack->PopTopMost());
	TestEqual(
		TEXT("Coverage restores the widget's previous visibility"),
		DockWidget->GetVisibility(),
		ESlateVisibility::HitTestInvisible);

	Stack->ClearAll();
	TestEqual(TEXT("ClearAll uses RootTeardown by default"), LastCloseReason, EUIWidgetCloseReason::RootTeardown);
	TestEqual(TEXT("ClearAll removes remaining frames"), Stack->Num(), 0);
	TestEqual(TEXT("Every pop and ClearAll emits removal"), RemovalCount, 4);
	TestTrue(TEXT("Activation transitions were emitted"), ActivationChangeCount >= 5);
	return true;
}

#endif
