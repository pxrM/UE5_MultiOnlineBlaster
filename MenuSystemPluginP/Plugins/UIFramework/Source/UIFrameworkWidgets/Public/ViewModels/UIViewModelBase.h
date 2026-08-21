// Copyright TikiStar. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "FieldNotificationId.h"
#include "MVVMViewModelBase.h"
#include "UIViewModelBase.generated.h"

/**
 * Base view model for the framework's MVVM layer. Extends UMVVMViewModelBase so it
 * plugs into UMG's view binding, and adds Initialize/Shutdown lifecycle hooks.
 *
 * A view model holds UI-facing state + commands and never references concrete
 * widgets — the View binds to it, not the other way round. Use the engine macro
 * UE_MVVM_SET_PROPERTY_VALUE in setters so bound widgets refresh automatically.
 * See Docs/DESIGN.md section 2 (layering).
 *
 * Lives in the Widgets module (not Core) because UMVVMViewModelBase transitively
 * depends on UMG; see Docs/DESIGN.md section 3.
 *
 * Script runtimes need the SetField* / NotifyFieldChanged API below. A script that
 * assigns a property through reflection writes the memory directly and never reaches
 * the setter, so no field-changed broadcast happens and bound widgets go stale.
 */
UCLASS(Blueprintable, Abstract)
class UIFRAMEWORKWIDGETS_API UUIViewModelBase : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** Called once after creation. Override to fetch initial data / subscribe to events. */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel")
	virtual void Initialize();

	/** Called before disposal. Override to unsubscribe / release resources. */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel")
	virtual void Shutdown();

	/** True between Initialize and Shutdown. */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel")
	bool IsInitialized() const { return bInitialized; }

	/**
	 * Broadcast that FieldName changed, refreshing every binding that reads it.
	 *
	 * For callers that already wrote the property by other means — a script runtime
	 * assigning through reflection, or a batch of native writes notified once at the
	 * end. Native code with access to the field id should prefer
	 * UE_MVVM_SET_PROPERTY_VALUE, which skips the by-name lookup and the broadcast
	 * when the value did not actually change.
	 *
	 * @return false if the class declares no FieldNotify field by that name.
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel|Scripting")
	bool NotifyFieldChanged(FName FieldName);

	/** True if this class declares a FieldNotify field by that name. */
	UFUNCTION(BlueprintPure, Category = "UI|ViewModel|Scripting")
	bool HasNotifyField(FName FieldName) const;

	/** Every FieldNotify field name this class declares. Useful for tooling and tests. */
	UFUNCTION(BlueprintPure, Category = "UI|ViewModel|Scripting")
	TArray<FName> GetNotifyFieldNames() const;

	/**
	 * Set a named property and broadcast the change, skipping the broadcast when the
	 * value is unchanged. This is the write path script runtimes should use.
	 *
	 * The property must be a FieldNotify field of a matching type; mismatches fail
	 * rather than reinterpreting memory.
	 *
	 * @return true if the value changed (and a broadcast happened).
	 */
	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel|Scripting")
	bool SetFieldText(FName FieldName, const FText& Value);

	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel|Scripting")
	bool SetFieldString(FName FieldName, const FString& Value);

	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel|Scripting")
	bool SetFieldName(FName FieldName, FName Value);

	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel|Scripting")
	bool SetFieldInt(FName FieldName, int32 Value);

	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel|Scripting")
	bool SetFieldInt64(FName FieldName, int64 Value);

	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel|Scripting")
	bool SetFieldFloat(FName FieldName, double Value);

	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel|Scripting")
	bool SetFieldBool(FName FieldName, bool Value);

	UFUNCTION(BlueprintCallable, Category = "UI|ViewModel|Scripting")
	bool SetFieldObject(FName FieldName, UObject* Value);

protected:
	/** Blueprint hook mirroring Initialize, for view models authored in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|ViewModel", meta = (DisplayName = "On Initialize"))
	void BP_OnInitialize();

	/** Blueprint hook mirroring Shutdown. */
	UFUNCTION(BlueprintImplementableEvent, Category = "UI|ViewModel", meta = (DisplayName = "On Shutdown"))
	void BP_OnShutdown();

private:
	/**
	 * Resolve FieldName to a FieldNotify field id plus the FProperty backing it.
	 * Logs and returns false when either half is missing, so every SetField* shares
	 * one diagnostic path.
	 */
	bool ResolveNotifyField(
		FName FieldName,
		const TCHAR* ExpectedTypeForLog,
		UE::FieldNotification::FFieldId& OutFieldId,
		FProperty*& OutProperty);

	/**
	 * Shared body for the numeric/simple SetField* overloads: resolve, cast the
	 * property to PropertyType, compare, assign, broadcast.
	 */
	template <typename PropertyType, typename ValueType>
	bool SetFieldValue(FName FieldName, const ValueType& Value, const TCHAR* TypeNameForLog);

	bool bInitialized = false;
};
