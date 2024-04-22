// ExtendMenuBase.h

#pragma once

#include "Modules/ModuleInterface.h"

class FExtendMenuBase : public IModuleInterface
{
public:
	
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual ~FExtendMenuBase() {}
	
private:
	/** FExtend��չ��ʽ��ں��� */
	void ExtendMenuByFExtend();

	/** ��������չ������һ���˵���ť */
	void MakeExistingMainMenuEntry(FMenuBuilder& MenuBuilder);
	/** ��������չ�����ɲ˵���ť�ĵ����¼� */
	void ExistingMainMenuEntryAction();

	/** ����һ��MenuBar�˵���ť */
	void MakeNewMenuBarEntry(FMenuBarBuilder& MenuBarBuilder);
	/** ������չ������һ���˵���ť */
	void MakeNewMainMenuEntry(FMenuBuilder& MenuBuilder);
	/** ������չ�����ɲ˵���ť�ĵ����¼� */
	void NewMainMenuEntryAction();

	/** ����һ��ToolBar��ť */
	void MakeToolBarEntry(FToolBarBuilder& ToolBarBuilder);
	/** ��ToolBar��ť���ʱ�����¼� */
	void ToolBarEntryAction();




	/** FExtend��չContentBrowser��ں��� */
	void ExtendContentBrowserByFExtend();

	/** ��չ�ʲ���������Ĳ˵� */
	TSharedRef<FExtender> ExtendAssetContextMenu(const TArray<FString>& SelectedPaths);
	/** ����һ���ʲ���������Ĳ˵���ť */
	void MakeAssetContextMenuEntry(FMenuBuilder& MenuBuilder);
	/** ���ʲ���������Ĳ˵���ť�����¼� */
	void AssetContextMenuEntryAction();

	/** ��չѡ���ʲ������Ĳ˵� */
	TSharedRef<FExtender> ExtendAssetViewContextMenu(const TArray<FAssetData>& SelectedAssets);
	/** ����һ��ѡ���ʲ������Ĳ˵���ť */
	void MakeAssetViewContextMenuEntry(FMenuBuilder& MenuBuilder);
	/** ��ѡ���ʲ������Ĳ˵���ť�����¼� */
	void AssetViewContextMenuEntryAction();




	/** ��չ�ӿڲ˵���ں��� */
	void ExtendViewportByFExtend();
	/** ��չ�ӿ��� Actor �˵� */
	TSharedRef<FExtender> ExtendViewportSelectedActorMenu(const TSharedRef<FUICommandList> UICommandList, const TArray<AActor*> SelectedActors);
	/** ����һ���ӿ��� Actor �˵���ť */
	void MakeViewportActorMenuEntry(FMenuBuilder& MenuBuilder);
	/** �˵������¼� */
	void ViewportActorMenuEntryAction();




	/** UToolMenus��չ��ʽ��ں��� */
	void ExtendByUToolMenus();

	/** NewMenu2Button�����¼� */
	void NewMenu2ButtonAction();

	/** ��FLevelEditorModule�������չ��� */
	void OnLevelEditorCreatedEvent(TSharedPtr<class ILevelEditor> Editor);



	/** Styleʾ����� */
	void StyleSample();
	void StyleButtonAction();
};