// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ViewModels/UIViewModelBase.h"
#include "UIViewModelTestTypes.generated.h"

/**
 * View model with one FieldNotify property per type the script-facing SetField* API
 * supports, plus a couple of negative cases. Exists only so automation tests can
 * exercise NotifyFieldChanged / SetField* against real reflection data — a
 * FieldNotify field cannot be synthesized at runtime.
 *
 * Public rather than private so the script adapter plugins' tests can share it.
 */
UCLASS()
class UIFRAMEWORKWIDGETS_API UUIViewModelTestSubject : public UUIViewModelBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	FText Label;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	FString Note;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	FName Tag;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	int32 Score = 0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	int64 BigScore = 0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	float Ratio = 0.f;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	double Precise = 0.0;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	bool bEnabled = false;

	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	TObjectPtr<UObject> Payload = nullptr;

	/** Typed object field, to verify SetFieldObject rejects an incompatible class. */
	UPROPERTY(BlueprintReadWrite, FieldNotify, Category = "Test")
	TObjectPtr<UUIViewModelBase> TypedPayload = nullptr;

	/** Declared but NOT FieldNotify, so the SetField* API must refuse it. */
	UPROPERTY(BlueprintReadWrite, Category = "Test")
	int32 Untracked = 0;

	/**
	 * Subscribe to every FieldNotify field and count broadcasts per field name, so a
	 * test can assert "the binding was told" rather than only checking the stored
	 * value. Kept here so tests in adapter plugins do not each need a direct
	 * ModelViewViewModel dependency to reach AddFieldValueChangedDelegate.
	 */
	void StartCountingBroadcasts();

	/** Broadcasts seen for FieldName since StartCountingBroadcasts. */
	int32 GetBroadcastCount(FName FieldName) const;

private:
	TMap<FName, int32> BroadcastCounts;
};
