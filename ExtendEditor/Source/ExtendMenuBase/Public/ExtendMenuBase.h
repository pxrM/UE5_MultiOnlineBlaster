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

};