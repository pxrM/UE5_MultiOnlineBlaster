#include "UMGStateConfiguratorEditor.h"

#include "UMGStateController.h"
#include "UMGStateControllerDetails.h"
#include "Blueprint/UserWidget.h"
#include "Components/Widget.h"

#define LOCTEXT_NAMESPACE "FUMGStateConfiguratorEditorModule"

void FUMGStateConfiguratorEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"UIPropertyOverride", 
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUIPropertyOverrideCustomization::MakeInstance)
	);

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddStatic(&FUMGStateConfiguratorEditorModule::OnObjectPropertyChanged);
}

void FUMGStateConfiguratorEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("UIPropertyOverride");
	}
	
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
}

void FUMGStateConfiguratorEditorModule::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	UWidget* ChangedWidget = Cast<UWidget>(Object);
	if (!ChangedWidget) return;

	UUserWidget* OuterUserWidget = ChangedWidget->GetTypedOuter<UUserWidget>();
	if (!OuterUserWidget) return;

	// 查找所有的 StateController 组件
	TArray<UObject*> SubObjects;
	GetObjectsWithOuter(OuterUserWidget, SubObjects);
	for (UObject* SubObject : SubObjects)
	{
		UUMGStateController* Controller = Cast<UUMGStateController>(SubObject);
		if (!Controller) return;
		// 遍历所有状态组，看谁开启了录制
		for (FUIStateGroup& Group : Controller->StateGroups)
		{
			if (Group.bRecordMode)
			{
				FString ValueStr;
				void* ValuePtr = Event.Property->ContainerPtrToValuePtr<void>(ChangedWidget);
				Event.Property->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, ChangedWidget, 0);
				Controller->UpdateRecordedPropertyToGroup(Group, ChangedWidget->GetFName(), Event.Property->GetFName(), ValueStr);

				UE_LOG(LogTemp, Log, TEXT("Auto-Recorded to State [%s]: %s.%s"), *Group.StateName, *ChangedWidget->GetName(), *Event.Property->GetName());
			}
		}
	}
}

IMPLEMENT_MODULE(FUMGStateConfiguratorEditorModule, UMGStateConfigurator)
