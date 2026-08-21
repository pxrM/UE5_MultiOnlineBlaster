// Copyright TikiStar. All Rights Reserved.

#include "ViewModels/UIViewModelBase.h"
#include "UIFrameworkCoreModule.h"

void UUIViewModelBase::Initialize()
{
	if (bInitialized)
	{
		return;
	}
	bInitialized = true;
	BP_OnInitialize();
}

void UUIViewModelBase::Shutdown()
{
	if (!bInitialized)
	{
		return;
	}
	bInitialized = false;
	BP_OnShutdown();
}

bool UUIViewModelBase::HasNotifyField(FName FieldName) const
{
	return GetFieldNotificationDescriptor().GetField(GetClass(), FieldName).IsValid();
}

TArray<FName> UUIViewModelBase::GetNotifyFieldNames() const
{
	TArray<FName> Names;
	GetFieldNotificationDescriptor().ForEachField(GetClass(),
		[&Names](const UE::FieldNotification::FFieldId FieldId)
		{
			Names.Add(FieldId.GetName());
			return true;
		});
	return Names;
}

bool UUIViewModelBase::NotifyFieldChanged(FName FieldName)
{
	const UE::FieldNotification::FFieldId FieldId =
		GetFieldNotificationDescriptor().GetField(GetClass(), FieldName);
	if (!FieldId.IsValid())
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("NotifyFieldChanged: '%s' is not a FieldNotify field of '%s'."),
			*FieldName.ToString(), *GetClass()->GetName());
		return false;
	}

	BroadcastFieldValueChanged(FieldId);
	return true;
}

bool UUIViewModelBase::ResolveNotifyField(
	FName FieldName,
	const TCHAR* ExpectedTypeForLog,
	UE::FieldNotification::FFieldId& OutFieldId,
	FProperty*& OutProperty)
{
	OutProperty = nullptr;
	OutFieldId = GetFieldNotificationDescriptor().GetField(GetClass(), FieldName);
	if (!OutFieldId.IsValid())
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("Set%s: '%s' is not a FieldNotify field of '%s'."),
			ExpectedTypeForLog, *FieldName.ToString(), *GetClass()->GetName());
		return false;
	}

	// A FieldNotify id can also name a getter function; only properties are writable.
	OutProperty = GetClass()->FindPropertyByName(FieldName);
	if (!OutProperty)
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("Set%s: FieldNotify field '%s' on '%s' is not a property and cannot be written."),
			ExpectedTypeForLog, *FieldName.ToString(), *GetClass()->GetName());
		return false;
	}
	return true;
}

template <typename PropertyType, typename ValueType>
bool UUIViewModelBase::SetFieldValue(FName FieldName, const ValueType& Value, const TCHAR* TypeNameForLog)
{
	UE::FieldNotification::FFieldId FieldId;
	FProperty* Property = nullptr;
	if (!ResolveNotifyField(FieldName, TypeNameForLog, FieldId, Property))
	{
		return false;
	}

	PropertyType* Typed = CastField<PropertyType>(Property);
	if (!Typed)
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("Set%s: field '%s' on '%s' is a %s, not the expected type."),
			TypeNameForLog, *FieldName.ToString(), *GetClass()->GetName(), *Property->GetClass()->GetName());
		return false;
	}

	ValueType* ValuePtr = Typed->template ContainerPtrToValuePtr<ValueType>(this);
	if (*ValuePtr == Value)
	{
		return false;
	}

	*ValuePtr = Value;
	BroadcastFieldValueChanged(FieldId);
	return true;
}

bool UUIViewModelBase::SetFieldText(FName FieldName, const FText& Value)
{
	UE::FieldNotification::FFieldId FieldId;
	FProperty* Property = nullptr;
	if (!ResolveNotifyField(FieldName, TEXT("FieldText"), FieldId, Property))
	{
		return false;
	}

	FTextProperty* Typed = CastField<FTextProperty>(Property);
	if (!Typed)
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("SetFieldText: field '%s' on '%s' is a %s, not FText."),
			*FieldName.ToString(), *GetClass()->GetName(), *Property->GetClass()->GetName());
		return false;
	}

	// UMVVMViewModelBase's own FText setter uses IdenticalTo, which by default only
	// compares the internal data pointer. That is fine for native code reusing one
	// FText, but a script runtime builds a fresh FText per call, so it would never
	// compare equal and every write would broadcast. Compare lexically instead.
	FText* ValuePtr = Typed->ContainerPtrToValuePtr<FText>(this);
	if (ValuePtr->IdenticalTo(Value) || ValuePtr->EqualTo(Value))
	{
		return false;
	}

	*ValuePtr = Value;
	BroadcastFieldValueChanged(FieldId);
	return true;
}

bool UUIViewModelBase::SetFieldString(FName FieldName, const FString& Value)
{
	return SetFieldValue<FStrProperty, FString>(FieldName, Value, TEXT("FieldString"));
}

bool UUIViewModelBase::SetFieldName(FName FieldName, FName Value)
{
	return SetFieldValue<FNameProperty, FName>(FieldName, Value, TEXT("FieldName"));
}

bool UUIViewModelBase::SetFieldInt(FName FieldName, int32 Value)
{
	return SetFieldValue<FIntProperty, int32>(FieldName, Value, TEXT("FieldInt"));
}

bool UUIViewModelBase::SetFieldInt64(FName FieldName, int64 Value)
{
	return SetFieldValue<FInt64Property, int64>(FieldName, Value, TEXT("FieldInt64"));
}

bool UUIViewModelBase::SetFieldBool(FName FieldName, bool Value)
{
	UE::FieldNotification::FFieldId FieldId;
	FProperty* Property = nullptr;
	if (!ResolveNotifyField(FieldName, TEXT("FieldBool"), FieldId, Property))
	{
		return false;
	}

	FBoolProperty* Typed = CastField<FBoolProperty>(Property);
	if (!Typed)
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("SetFieldBool: field '%s' on '%s' is a %s, not bool."),
			*FieldName.ToString(), *GetClass()->GetName(), *Property->GetClass()->GetName());
		return false;
	}

	// Bitfield bools have no addressable bool, so go through the property accessors.
	void* ValuePtr = Typed->ContainerPtrToValuePtr<void>(this);
	if (Typed->GetPropertyValue(ValuePtr) == Value)
	{
		return false;
	}

	Typed->SetPropertyValue(ValuePtr, Value);
	BroadcastFieldValueChanged(FieldId);
	return true;
}

bool UUIViewModelBase::SetFieldFloat(FName FieldName, double Value)
{
	UE::FieldNotification::FFieldId FieldId;
	FProperty* Property = nullptr;
	if (!ResolveNotifyField(FieldName, TEXT("FieldFloat"), FieldId, Property))
	{
		return false;
	}

	// Accept both float and double properties: script runtimes have one number type,
	// so forcing callers to pick an overload by storage width would be a trap.
	if (FFloatProperty* AsFloat = CastField<FFloatProperty>(Property))
	{
		float* ValuePtr = AsFloat->ContainerPtrToValuePtr<float>(this);
		const float Narrowed = static_cast<float>(Value);
		if (*ValuePtr == Narrowed)
		{
			return false;
		}
		*ValuePtr = Narrowed;
		BroadcastFieldValueChanged(FieldId);
		return true;
	}

	if (FDoubleProperty* AsDouble = CastField<FDoubleProperty>(Property))
	{
		double* ValuePtr = AsDouble->ContainerPtrToValuePtr<double>(this);
		if (*ValuePtr == Value)
		{
			return false;
		}
		*ValuePtr = Value;
		BroadcastFieldValueChanged(FieldId);
		return true;
	}

	UE_LOG(LogUIFramework, Warning,
		TEXT("SetFieldFloat: field '%s' on '%s' is a %s, not float or double."),
		*FieldName.ToString(), *GetClass()->GetName(), *Property->GetClass()->GetName());
	return false;
}

bool UUIViewModelBase::SetFieldObject(FName FieldName, UObject* Value)
{
	UE::FieldNotification::FFieldId FieldId;
	FProperty* Property = nullptr;
	if (!ResolveNotifyField(FieldName, TEXT("FieldObject"), FieldId, Property))
	{
		return false;
	}

	FObjectPropertyBase* Typed = CastField<FObjectPropertyBase>(Property);
	if (!Typed)
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("SetFieldObject: field '%s' on '%s' is a %s, not an object reference."),
			*FieldName.ToString(), *GetClass()->GetName(), *Property->GetClass()->GetName());
		return false;
	}

	if (Value && !Value->IsA(Typed->PropertyClass))
	{
		UE_LOG(LogUIFramework, Warning,
			TEXT("SetFieldObject: '%s' is a %s, which field '%s' on '%s' cannot hold (expects %s)."),
			*Value->GetName(), *Value->GetClass()->GetName(), *FieldName.ToString(),
			*GetClass()->GetName(), *Typed->PropertyClass->GetName());
		return false;
	}

	void* ValuePtr = Typed->ContainerPtrToValuePtr<void>(this);
	if (Typed->GetObjectPropertyValue(ValuePtr) == Value)
	{
		return false;
	}

	Typed->SetObjectPropertyValue(ValuePtr, Value);
	BroadcastFieldValueChanged(FieldId);
	return true;
}
