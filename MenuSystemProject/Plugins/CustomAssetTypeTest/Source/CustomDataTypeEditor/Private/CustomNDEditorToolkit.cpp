// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomNDEditorToolkit.h"
#include "CustomNormalDistribution.h"
#include "Modules/ModuleManager.h"
#include "CustomNDWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

FCustomNDEditorToolkit::FCustomNDEditorToolkit()
{
}

FCustomNDEditorToolkit::~FCustomNDEditorToolkit()
{
}

void FCustomNDEditorToolkit::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	// 注册父类中定义的选项卡生成器
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// 创建一个本地的工作区菜单类别，并将其赋值给 WorkspaceMenuCategory 变量，以便后续使用。
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(INVTEXT("CustomNormalDistributionTabs"));

	// 注册了名为 "CustomNormalDistributionPDFTab" 的选项卡生成器。
	// 生成器使用 FOnSpawnTab::CreateLambda 创建了一个匿名函数，该函数返回一个 SDockTab 控件，
	// 其中包含了一个 SCustomNDWidget 控件，用于显示自定义正态分布的概率密度函数。
	// 同时，将一些函数绑定到 SCustomNDWidget 控件的属性上，以便在编辑器中更新相关数据。
	InTabManager->RegisterTabSpawner("CustomNormalDistributionPDFTab", FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
		{
			return SNew(SDockTab)
				[
					SNew(SCustomNDWidget)
						.Mean(this, &FCustomNDEditorToolkit::GetMean)
						.StandardDeviation(this, &FCustomNDEditorToolkit::GetStandarDeviation)
						.OnMeanChanged(this, &FCustomNDEditorToolkit::SetMean)
						.OnStandardDeviationChanged(this, &FCustomNDEditorToolkit::SetStandarDeviation)
				];
		}))
		// 设置选项卡的显示名称为 "PDF"（概率密度函数），这个名称将在选项卡标签中显示。
		.SetDisplayName(INVTEXT("PDF"))
			// 将选项卡分组到之前创建的工作区菜单类别中。
			.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	// 创建 CustomNormalDistribution DetailsView
	// 通过 FModuleManager 获取了属性编辑器模块的引用，并将其存储在 PropertyEditorModule 变量中。属性编辑器模块负责管理属性编辑器相关的功能和界面。
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	// 创建用于配置属性编辑器详细视图的结构体
	FDetailsViewArgs DetailsViewArgs;
	// 隐藏详细视图中的名称区域，只显示属性值。
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	// 用属性编辑器模块的 CreateDetailView 函数创建了一个详细视图，并将配置参数 DetailsViewArgs 传递给它
	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	// 调用 DetailsView 的 SetObjects 函数，将要显示和编辑的对象传递给它。
	DetailsView->SetObjects(TArray<UObject*>{ NormalDistribution });

	// 注册 CustomNormalDistribution DetailsView TabSpawner
	// 注册了第二个选项卡生成器，用于显示自定义正态分布对象的详细信息。
	// 在匿名函数中创建了一个包含 DetailsView 的 SDockTab 控件，DetailsView 用于显示自定义正态分布对象的属性。
	InTabManager->RegisterTabSpawner("CustomNormalDistributionDetailsTab",
		FOnSpawnTab::CreateLambda([=](const FSpawnTabArgs&)
			{
				return SNew(SDockTab)
					[
						DetailsView
					];
			}))
		.SetDisplayName(INVTEXT("Details"))
				.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FCustomNDEditorToolkit::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);
	TabManager->UnregisterTabSpawner("CustomNormalDistributionPDFTab");
	TabManager->UnregisterTabSpawner("CustomNormalDistributionDetailsTab");
}

void FCustomNDEditorToolkit::InitEditor(const TArray<UObject*>& InObjects)
{
	// 从编辑器将要编辑的对象InObjects列表中的第一个对象（通常是一个UCustomNormalDistribution类型的对象）转换为UCustomNormalDistribution类型
	NormalDistribution = Cast<UCustomNormalDistribution>(InObjects[0]);

	// 使用 FTabManager::NewLayout 创建一个新的布局，并指定了布局的名称
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("CustomNormalDistributionEditorLayout")->AddArea(
		// 添加一个主要区域（Primary Area）表示编辑器的主要工作区域。
		FTabManager::NewPrimaryArea()
		// 设置主要区域的方向为垂直方向。
		->SetOrientation(Orient_Vertical)
		// 对主要区域进行分割。
		->Split(
			// 创建一个新的分割器，用于将主要区域分割成两部分。
			FTabManager::NewSplitter()
			// 设置第一部分（PDF Tab）的大小系数为 0.6，占据整个主要区域的 60%。
			->SetSizeCoefficient(0.6f)
			// 设置分割器的方向为水平方向。
			->SetOrientation(Orient_Horizontal)
			// 再次对第一部分进行分割。
			->Split(
				// 创建一个新的堆栈，用于存放选项卡。
				FTabManager::NewStack()
				// 设置第一个选项卡的大小系数为 0.8，占据第一部分的 80%。
				->SetSizeCoefficient(0.8f)
				// 向第一个堆栈中添加一个名为 "CustomNormalDistributionPDFTab" 的选项卡，并将其设置为已打开状态。
				->AddTab("CustomNormalDistributionPDFTab", ETabState::OpenedTab)
			) // 完成第一部分的设置。
			// 再次对主要区域进行分割。
			->Split(
				// 创建另一个堆栈，用于存放选项卡。
				FTabManager::NewStack()
				// 设置第二个选项卡的大小系数为 0.2，占据第二部分的 20%。
				->SetSizeCoefficient(0.2f)
				// 第二个堆栈中添加一个名为 "CustomNormalDistributionDetailsTab" 的选项卡，并将其设置为已打开状态。
				->AddTab("CustomNormalDistributionDetailsTab", ETabState::OpenedTab)
			) // 完成第二部分的设置。
		)
		// 对编辑器的整体区域进行分割。
		->Split(
			// 创建一个堆栈，用于存放选项卡。
			FTabManager::NewStack()
			// 设置堆栈的大小系数为 0.4，占据剩余空间的 40%。
			->SetSizeCoefficient(0.4f)
			// 向堆栈中添加一个名为 "OutputLog" 的选项卡，并将其设置为已打开状态。
			->AddTab("OutputLog", ETabState::OpenedTab)
		) // 结束整个布局设置。
	);
	// 初始化编辑器，指定编辑器的模式、标识、布局、是否支持交互式重复加载、是否支持临时编辑模式，并将传入的对象数组作为参数传递给编辑器。
	FAssetEditorToolkit::InitAssetEditor(EToolkitMode::Standalone, {}, "CustomNormalDistributionEditor", Layout, true, true, InObjects);
}

float FCustomNDEditorToolkit::GetMean() const
{
	return NormalDistribution->Mean;
}

float FCustomNDEditorToolkit::GetStandarDeviation() const
{
	return NormalDistribution->StandardDeviation;
}

void FCustomNDEditorToolkit::SetMean(float InMean)
{
	NormalDistribution->Modify();
	NormalDistribution->Mean = InMean;
}

void FCustomNDEditorToolkit::SetStandarDeviation(float InStandardDeviation)
{
	NormalDistribution->Modify();
	NormalDistribution->StandardDeviation = InStandardDeviation;
}
