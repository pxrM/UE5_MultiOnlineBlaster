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
	// ע�Ḹ���ж����ѡ�������
	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	// ����һ�����صĹ������˵���𣬲����丳ֵ�� WorkspaceMenuCategory �������Ա����ʹ�á�
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(INVTEXT("CustomNormalDistributionTabs"));

	// ע������Ϊ "CustomNormalDistributionPDFTab" ��ѡ���������
	// ������ʹ�� FOnSpawnTab::CreateLambda ������һ�������������ú�������һ�� SDockTab �ؼ���
	// ���а�����һ�� SCustomNDWidget �ؼ���������ʾ�Զ�����̬�ֲ��ĸ����ܶȺ�����
	// ͬʱ����һЩ�����󶨵� SCustomNDWidget �ؼ��������ϣ��Ա��ڱ༭���и���������ݡ�
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
		// ����ѡ�����ʾ����Ϊ "PDF"�������ܶȺ�������������ƽ���ѡ���ǩ����ʾ��
		.SetDisplayName(INVTEXT("PDF"))
			// ��ѡ����鵽֮ǰ�����Ĺ������˵�����С�
			.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	// ���� CustomNormalDistribution DetailsView
	// ͨ�� FModuleManager ��ȡ�����Ա༭��ģ������ã�������洢�� PropertyEditorModule �����С����Ա༭��ģ�鸺��������Ա༭����صĹ��ܺͽ��档
	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	// ���������������Ա༭����ϸ��ͼ�Ľṹ��
	FDetailsViewArgs DetailsViewArgs;
	// ������ϸ��ͼ�е���������ֻ��ʾ����ֵ��
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	// �����Ա༭��ģ��� CreateDetailView ����������һ����ϸ��ͼ���������ò��� DetailsViewArgs ���ݸ���
	TSharedRef<IDetailsView> DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	// ���� DetailsView �� SetObjects ��������Ҫ��ʾ�ͱ༭�Ķ��󴫵ݸ�����
	DetailsView->SetObjects(TArray<UObject*>{ NormalDistribution });

	// ע�� CustomNormalDistribution DetailsView TabSpawner
	// ע���˵ڶ���ѡ���������������ʾ�Զ�����̬�ֲ��������ϸ��Ϣ��
	// �����������д�����һ������ DetailsView �� SDockTab �ؼ���DetailsView ������ʾ�Զ�����̬�ֲ���������ԡ�
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
	// �ӱ༭����Ҫ�༭�Ķ���InObjects�б��еĵ�һ������ͨ����һ��UCustomNormalDistribution���͵Ķ���ת��ΪUCustomNormalDistribution����
	NormalDistribution = Cast<UCustomNormalDistribution>(InObjects[0]);

	// ʹ�� FTabManager::NewLayout ����һ���µĲ��֣���ָ���˲��ֵ�����
	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout("CustomNormalDistributionEditorLayout")->AddArea(
		// ���һ����Ҫ����Primary Area����ʾ�༭������Ҫ��������
		FTabManager::NewPrimaryArea()
		// ������Ҫ����ķ���Ϊ��ֱ����
		->SetOrientation(Orient_Vertical)
		// ����Ҫ������зָ
		->Split(
			// ����һ���µķָ��������ڽ���Ҫ����ָ�������֡�
			FTabManager::NewSplitter()
			// ���õ�һ���֣�PDF Tab���Ĵ�Сϵ��Ϊ 0.6��ռ��������Ҫ����� 60%��
			->SetSizeCoefficient(0.6f)
			// ���÷ָ����ķ���Ϊˮƽ����
			->SetOrientation(Orient_Horizontal)
			// �ٴζԵ�һ���ֽ��зָ
			->Split(
				// ����һ���µĶ�ջ�����ڴ��ѡ���
				FTabManager::NewStack()
				// ���õ�һ��ѡ��Ĵ�Сϵ��Ϊ 0.8��ռ�ݵ�һ���ֵ� 80%��
				->SetSizeCoefficient(0.8f)
				// ���һ����ջ�����һ����Ϊ "CustomNormalDistributionPDFTab" ��ѡ�������������Ϊ�Ѵ�״̬��
				->AddTab("CustomNormalDistributionPDFTab", ETabState::OpenedTab)
			) // ��ɵ�һ���ֵ����á�
			// �ٴζ���Ҫ������зָ
			->Split(
				// ������һ����ջ�����ڴ��ѡ���
				FTabManager::NewStack()
				// ���õڶ���ѡ��Ĵ�Сϵ��Ϊ 0.2��ռ�ݵڶ����ֵ� 20%��
				->SetSizeCoefficient(0.2f)
				// �ڶ�����ջ�����һ����Ϊ "CustomNormalDistributionDetailsTab" ��ѡ�������������Ϊ�Ѵ�״̬��
				->AddTab("CustomNormalDistributionDetailsTab", ETabState::OpenedTab)
			) // ��ɵڶ����ֵ����á�
		)
		// �Ա༭��������������зָ
		->Split(
			// ����һ����ջ�����ڴ��ѡ���
			FTabManager::NewStack()
			// ���ö�ջ�Ĵ�Сϵ��Ϊ 0.4��ռ��ʣ��ռ�� 40%��
			->SetSizeCoefficient(0.4f)
			// ���ջ�����һ����Ϊ "OutputLog" ��ѡ�������������Ϊ�Ѵ�״̬��
			->AddTab("OutputLog", ETabState::OpenedTab)
		) // ���������������á�
	);
	// ��ʼ���༭����ָ���༭����ģʽ����ʶ�����֡��Ƿ�֧�ֽ���ʽ�ظ����ء��Ƿ�֧����ʱ�༭ģʽ����������Ķ���������Ϊ�������ݸ��༭����
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
