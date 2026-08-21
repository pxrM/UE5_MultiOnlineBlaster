// Copyright TikiStar. All Rights Reserved.

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "UIViewModelTestTypes.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkViewModelNotifyTest,
	"TikiStar.UIFramework.ViewModel.FieldNotify",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkViewModelNotifyTest::RunTest(const FString& Parameters)
{
	UUIViewModelTestSubject* VM = NewObject<UUIViewModelTestSubject>();

	// Count broadcasts per field so "did the binding get told" is observable rather
	// than inferred from the stored value.
	VM->StartCountingBroadcasts();
	const auto Count = [VM](const TCHAR* Field)
	{
		return VM->GetBroadcastCount(FName(Field));
	};

	// --- discovery -------------------------------------------------------------
	TestTrue(TEXT("FieldNotify property is discoverable by name"), VM->HasNotifyField(TEXT("Score")));
	TestFalse(TEXT("A plain UPROPERTY is not a notify field"), VM->HasNotifyField(TEXT("Untracked")));
	TestFalse(TEXT("An unknown name is not a notify field"), VM->HasNotifyField(TEXT("Nonexistent")));
	TestTrue(TEXT("Notify field names include the declared fields"),
		VM->GetNotifyFieldNames().Contains(FName(TEXT("Label"))));

	// --- each supported type round-trips and broadcasts exactly once ------------
	TestTrue(TEXT("SetFieldText stores the value"), VM->SetFieldText(TEXT("Label"), FText::FromString(TEXT("Hello"))));
	TestEqual(TEXT("Label holds the new text"), VM->Label.ToString(), FString(TEXT("Hello")));
	TestEqual(TEXT("Label broadcast once"), Count(TEXT("Label")), 1);

	TestTrue(TEXT("SetFieldString stores the value"), VM->SetFieldString(TEXT("Note"), TEXT("note")));
	TestEqual(TEXT("Note holds the new string"), VM->Note, FString(TEXT("note")));

	TestTrue(TEXT("SetFieldName stores the value"), VM->SetFieldName(TEXT("Tag"), TEXT("tag")));
	TestEqual(TEXT("Tag holds the new name"), VM->Tag, FName(TEXT("tag")));

	TestTrue(TEXT("SetFieldInt stores the value"), VM->SetFieldInt(TEXT("Score"), 42));
	TestEqual(TEXT("Score holds the new int"), VM->Score, 42);

	TestTrue(TEXT("SetFieldInt64 stores the value"), VM->SetFieldInt64(TEXT("BigScore"), 1ll << 40));
	TestEqual(TEXT("BigScore holds the new int64"), VM->BigScore, static_cast<int64>(1ll << 40));

	TestTrue(TEXT("SetFieldBool stores the value"), VM->SetFieldBool(TEXT("bEnabled"), true));
	TestTrue(TEXT("bEnabled holds the new bool"), VM->bEnabled);

	TestTrue(TEXT("SetFieldObject stores the value"), VM->SetFieldObject(TEXT("Payload"), VM));
	TestTrue(TEXT("Payload holds the new object"), VM->Payload == VM);

	// SetFieldFloat accepts both storage widths so script callers need one entry point.
	TestTrue(TEXT("SetFieldFloat writes a float property"), VM->SetFieldFloat(TEXT("Ratio"), 0.5));
	TestEqual(TEXT("Ratio holds the new float"), VM->Ratio, 0.5f);
	TestTrue(TEXT("SetFieldFloat writes a double property"), VM->SetFieldFloat(TEXT("Precise"), 0.25));
	TestEqual(TEXT("Precise holds the new double"), VM->Precise, 0.25);

	// --- unchanged writes must not broadcast -----------------------------------
	const int32 ScoreBroadcasts = Count(TEXT("Score"));
	TestFalse(TEXT("Rewriting the same int reports no change"), VM->SetFieldInt(TEXT("Score"), 42));
	TestEqual(TEXT("An unchanged int does not broadcast"), Count(TEXT("Score")), ScoreBroadcasts);

	const int32 LabelBroadcasts = Count(TEXT("Label"));
	// A freshly built FText with the same content is a different object, so this also
	// covers the case a script runtime always hits.
	TestFalse(TEXT("Rewriting equal text reports no change"),
		VM->SetFieldText(TEXT("Label"), FText::FromString(TEXT("Hello"))));
	TestEqual(TEXT("Equal text does not broadcast"), Count(TEXT("Label")), LabelBroadcasts);
	TestFalse(TEXT("Rewriting the very same FText reports no change"),
		VM->SetFieldText(TEXT("Label"), VM->Label));
	TestEqual(TEXT("Identical text does not broadcast"), Count(TEXT("Label")), LabelBroadcasts);
	TestTrue(TEXT("Different text still reports a change"),
		VM->SetFieldText(TEXT("Label"), FText::FromString(TEXT("Goodbye"))));
	TestEqual(TEXT("Different text broadcasts"), Count(TEXT("Label")), LabelBroadcasts + 1);

	// --- rejections ------------------------------------------------------------
	AddExpectedError(TEXT("is not a FieldNotify field"), EAutomationExpectedErrorFlags::Contains, 2);
	TestFalse(TEXT("A non-notify property is refused"), VM->SetFieldInt(TEXT("Untracked"), 7));
	TestEqual(TEXT("The refused write did not land"), VM->Untracked, 0);
	TestFalse(TEXT("An unknown field is refused"), VM->SetFieldInt(TEXT("Nonexistent"), 7));

	AddExpectedError(TEXT("not the expected type"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("A type mismatch is refused rather than reinterpreted"),
		VM->SetFieldInt(TEXT("Note"), 7));
	TestEqual(TEXT("The mismatched write left the string intact"), VM->Note, FString(TEXT("note")));

	AddExpectedError(TEXT("cannot hold"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("An incompatible object class is refused"),
		VM->SetFieldObject(TEXT("TypedPayload"), NewObject<UDataTable>()));
	TestTrue(TEXT("The refused object write left the field null"), VM->TypedPayload == nullptr);

	// --- explicit notify -------------------------------------------------------
	const int32 BeforeManual = Count(TEXT("Score"));
	VM->Score = 99; // simulates a script writing the property through reflection
	TestEqual(TEXT("A raw write does not broadcast on its own"), Count(TEXT("Score")), BeforeManual);
	TestTrue(TEXT("NotifyFieldChanged accepts a real field"), VM->NotifyFieldChanged(TEXT("Score")));
	TestEqual(TEXT("NotifyFieldChanged broadcasts"), Count(TEXT("Score")), BeforeManual + 1);

	AddExpectedError(TEXT("is not a FieldNotify field"), EAutomationExpectedErrorFlags::Contains, 1);
	TestFalse(TEXT("NotifyFieldChanged rejects an unknown field"), VM->NotifyFieldChanged(TEXT("Nonexistent")));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FUIFrameworkViewModelLifecycleTest,
	"TikiStar.UIFramework.ViewModel.Lifecycle",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUIFrameworkViewModelLifecycleTest::RunTest(const FString& Parameters)
{
	UUIViewModelTestSubject* VM = NewObject<UUIViewModelTestSubject>();
	TestFalse(TEXT("A fresh view model is not initialized"), VM->IsInitialized());

	VM->Initialize();
	TestTrue(TEXT("Initialize marks the view model initialized"), VM->IsInitialized());
	VM->Initialize();
	TestTrue(TEXT("Initialize is idempotent"), VM->IsInitialized());

	VM->Shutdown();
	TestFalse(TEXT("Shutdown clears the initialized flag"), VM->IsInitialized());
	VM->Shutdown();
	TestFalse(TEXT("Shutdown is idempotent"), VM->IsInitialized());

	// A cached screen can be reconstructed, which re-runs the create/init path.
	VM->Initialize();
	TestTrue(TEXT("A shut-down view model can be initialized again"), VM->IsInitialized());
	return true;
}

#endif
