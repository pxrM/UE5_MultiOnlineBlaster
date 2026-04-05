#include "UMGStateConfiguratorEditor.h"
#include "SUMGStateEditorWindow.h"
#include "UIPropertyOverrideCustomization.h"
#include "UMGStateController.h"
#include "UMGStateControllerDetails.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/Widget.h"
#include "WidgetBlueprint.h"
#include "WorkspaceMenuStructure.h"
#include "WorkspaceMenuStructureModule.h"
#include "ToolMenus.h"
#include "LevelEditor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Subsystems/AssetEditorSubsystem.h"

#define LOCTEXT_NAMESPACE "FUMGStateConfiguratorEditorModule"

void FUMGStateConfiguratorEditorModule::StartupModule()
{
	IModuleInterface::StartupModule();
	
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomPropertyTypeLayout(
		"UIPropertyOverride", 
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FUIPropertyOverrideCustomization::MakeInstance)
	);
	// 这个可能不会触发（如果 UUMGStateController 不是直接选中的对象）
	// PropertyModule.RegisterCustomClassLayout(
	// 	UUMGStateController::StaticClass()->GetFName(),
	// 	FOnGetDetailCustomizationInstance::CreateStatic(&FUMGStateControllerDetails::MakeInstance)
	// 	);

	FCoreUObjectDelegates::OnObjectPropertyChanged.AddStatic(&FUMGStateConfiguratorEditorModule::OnObjectPropertyChanged);
	
	RegisterTabSpawner();
	ExtendWidgetBlueprintMenu();
	UE_LOG(LogTemp, Log, TEXT("UMGStateConfiguratorEditor module started"));
}

void FUMGStateConfiguratorEditorModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout("UIPropertyOverride");
		PropertyModule.UnregisterCustomClassLayout("UMGStateControllerDetails");
	}
	
	FCoreUObjectDelegates::OnObjectPropertyChanged.RemoveAll(this);
	
	UnregisterTabSpawner();
	UE_LOG(LogTemp, Log, TEXT("UMGStateConfiguratorEditor module shutdown"));
}

void FUMGStateConfiguratorEditorModule::RegisterTabSpawner()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		StateEditorTabName,
		FOnSpawnTab::CreateStatic(&FUMGStateConfiguratorEditorModule::SpawnStateEditorTab)
	)
	.SetDisplayName(LOCTEXT("StateEditorTabTitle", "UI State Editor"))
	.SetMenuType(ETabSpawnerMenuType::Enabled)
	.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"));
}

void FUMGStateConfiguratorEditorModule::UnregisterTabSpawner()
{
	if (FSlateApplication::IsInitialized())
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(StateEditorTabName);
	}
}

TSharedRef<SDockTab> FUMGStateConfiguratorEditorModule::SpawnStateEditorTab(const FSpawnTabArgs& Args)
{
	UUserWidget* EditingWidget = nullptr;
	UUMGStateController* EditingController = nullptr;
	GetCurrentEditingContext(EditingWidget, EditingController);

	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SUMGStateEditorWindow)
			.StateController(EditingController)
			.PreviewWidget(EditingWidget)
		];
}

void FUMGStateConfiguratorEditorModule::ExtendWidgetBlueprintMenu()
{
	// 添加到主菜单
	if (UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window"))
	{
		FToolMenuSection& Section = Menu->FindOrAddSection("GetStarted");
		Section.AddMenuEntry("OpenUMGStateEditor",
		                     LOCTEXT("OpenUMGStateEditor", "UI State Editor"),
		                     LOCTEXT("OpenUMGStateEditorTooltip", "Open the UI State Configuration window"),
		                     FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports"),
		                     FUIAction(FExecuteAction::CreateLambda([]()
		                     {
		                     	FGlobalTabmanager::Get()->TryInvokeTab(FUMGStateConfiguratorEditorModule::StateEditorTabName);
		                     }))
		);
	}
	// 添加到 Widget 蓝图编辑器的工具栏
	if (UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("AssetEditor.WidgetBlueprintEditor.ToolBar"))
	{
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
		Section.AddEntry(FToolMenuEntry::InitToolBarButton(
			"OpenStateEditor",
			FUIAction(FExecuteAction::CreateLambda([]()
			{
				FGlobalTabmanager::Get()->TryInvokeTab(FUMGStateConfiguratorEditorModule::StateEditorTabName);
			})),
			LOCTEXT("OpenStateEditor", "State Editor"),
			LOCTEXT("OpenStateEditorTooltip", "Open UI State Configuration"),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Viewports")
		));
	}
}

void FUMGStateConfiguratorEditorModule::OpenStateEditorWindow()
{
	FGlobalTabmanager::Get()->TryInvokeTab(StateEditorTabName);
}

void FUMGStateConfiguratorEditorModule::GetCurrentEditingContext(UUserWidget*& OutWidget,
	UUMGStateController*& OutController)
{
	OutWidget = nullptr;
	OutController = nullptr;

	// 查找当前正在编辑的 Widget Blueprint 资产
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	if (!AssetEditorSubsystem) return;

	// 遍历所有打开的资产编辑器，找到 UUserWidget
	for (FAssetData AssetData : AssetEditorSubsystem->GetAllEditedAssets())
	{
		UObject* Asset = AssetData.GetAsset();
		if (!Asset) continue;

		UBlueprint* Blueprint = Cast<UBlueprint>(Asset);
		if (!Blueprint || !Blueprint->GeneratedClass) continue;

		UUserWidget* DefaultWidget = Cast<UUserWidget>(Blueprint->GeneratedClass->GetDefaultObject());
		if (!DefaultWidget) continue;

		// 查找 StateController
		for (TFieldIterator<FObjectProperty> It(DefaultWidget->GetClass()); It; ++It)
		{
			FObjectProperty* ObjProp = *It;
			if (ObjProp->PropertyClass && ObjProp->PropertyClass->IsChildOf(UUMGStateController::StaticClass()))
			{
				UObject* ControllerObj = ObjProp->GetObjectPropertyValue(
					ObjProp->ContainerPtrToValuePtr<void>(DefaultWidget));
				if (UUMGStateController* Controller = Cast<UUMGStateController>(ControllerObj))
				{
					OutWidget = DefaultWidget;
					OutController = Controller;
					return;
				}
			}
		}
	}
}

// 记录操作
void FUMGStateConfiguratorEditorModule::OnObjectPropertyChanged(UObject* Object, FPropertyChangedEvent& Event)
{
	if (Event.ChangeType == EPropertyChangeType::Interactive)
	{
		return;
	}
	UWidget* ChangedWidget = Cast<UWidget>(Object);
	if (!ChangedWidget) return;

	UUserWidget* OuterUserWidget = ChangedWidget->GetTypedOuter<UUserWidget>();
	if (!OuterUserWidget) return;

	// 判断是否存在 MemberProperty。
	// 如果是修改结构体内部（如 Brush.ResourceObject），MemberProperty 指向 Brush，Property 指向 ResourceObject。
	// 我们需要获取的是 Brush 的值，而不是 ResourceObject 的值。
	FProperty* PropToRecord = Event.MemberProperty ? Event.MemberProperty : Event.Property;
	// 安全检查：确保这个属性确实属于当前的 Widget 类 (防止一些特殊情况下的属性不匹配) 
	if (!PropToRecord || !ChangedWidget->GetClass()->IsChildOf(PropToRecord->GetOwner<UClass>())) return;
	FName PropertyToSave = PropToRecord->GetFName();
	// 查找所有的 StateController 组件
	TArray<UObject*> SubObjects;
	GetObjectsWithOuter(OuterUserWidget, SubObjects);
	
	for (UObject* SubObject : SubObjects)
	{
		UUMGStateController* Controller = Cast<UUMGStateController>(SubObject);
		if (!Controller) continue;

		bool bChangedInThisController = false;
		for (int32 CatIdx = 0; CatIdx < Controller->StateCategories.Num(); ++CatIdx)
		{
			FUIStateCategory& Category = Controller->StateCategories[CatIdx];
			for (int32 StateIdx = 0; StateIdx < Category.States.Num(); ++StateIdx)
			{
				FUIStateGroup& Group = Category.States[StateIdx];
				if (Group.bRecordMode)
				{
					// 1. 获取顶层属性的值 (比如整个 Brush 结构体的数据)
					void* ValuePtr = PropToRecord->ContainerPtrToValuePtr<void>(ChangedWidget);
					// 2. 导出为字符串
					FString ValueStr;
					PropToRecord->ExportTextItem_Direct(ValueStr, ValuePtr, nullptr, ChangedWidget, PPF_SimpleObjectText);
					// 3. 开启编辑器事务，支持撤回 (Undo)
					const FScopedTransaction Transaction(FText::FromString("Auto Record UMG State"));
					Controller->Modify();
					// 3. 保存记录。注意：这里传入的是 PropToRecord->GetFName() (即 "Brush")，而不是内部的 "ResourceObject"
					Controller->UpdateRecordedPropertyToCategory(CatIdx, StateIdx, ChangedWidget->GetFName(), PropertyToSave, ValueStr);
					bChangedInThisController = true;
					UE_LOG(LogTemp, Log, TEXT("Auto-Recorded to State [%s]: %s.%s"), *Group.StateName, *ChangedWidget->GetName(), *Event.Property->GetName());
				}
			}
		}
		if (bChangedInThisController)
		{
			SyncToBlueprintAsset(OuterUserWidget, Controller);
		}
	}
}

void FUMGStateConfiguratorEditorModule::SyncToBlueprintAsset(UUserWidget* PreviewWidget, const UUMGStateController* PreviewController)
{
	if (!PreviewWidget || !PreviewController) return;
	UBlueprint* WidgetBP = Cast<UBlueprint>(PreviewWidget->GetClass()->ClassGeneratedBy);
	if (!WidgetBP || !WidgetBP->GeneratedClass) return;

	UUserWidget* CDOWidget = Cast<UUserWidget>(WidgetBP->GeneratedClass->GetDefaultObject());
	if (!CDOWidget) return;
    
	for (TFieldIterator<FObjectProperty> It(PreviewWidget->GetClass()); It; ++It)
	{
		const FObjectProperty* ObjProp = *It;
		if (ObjProp->GetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(PreviewWidget)) == PreviewController)
		{
			UObject* CDOObj = ObjProp->GetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(CDOWidget));
			if (UUMGStateController* CDOController = Cast<UUMGStateController>(CDOObj))
			{
				CDOController->Modify();
				CDOController->StateCategories = PreviewController->StateCategories;
				WidgetBP->MarkPackageDirty();
				break;
			}
		}
	}
}

const FName FUMGStateConfiguratorEditorModule::StateEditorTabName(TEXT("UMGStateEditor"));

IMPLEMENT_MODULE(FUMGStateConfiguratorEditorModule, UMGStateConfiguratorEditor)
