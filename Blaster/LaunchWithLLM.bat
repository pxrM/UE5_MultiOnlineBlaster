@echo off

:: ������
set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe"
set "DEFAULT_PROJECT=E:\Project\UE\UE5_MultiOnlineBlaster\Blaster\Blaster.uproject"

:: ʹ�ô��������Ĭ����Ŀ
set "PROJECT_PATH=%~1"
if "%PROJECT_PATH%"=="" set "PROJECT_PATH=%DEFAULT_PROJECT%"

:: ����ļ��Ƿ����
if not exist "%UE_EDITOR%" (
    echo ERROR: Unreal Editor not found!
    echo Path: %UE_EDITOR%
    pause
    exit /b 1
)

if not exist "%PROJECT_PATH%" (
    echo ERROR: Project file not found!
    echo Path: %PROJECT_PATH%
    pause
    exit /b 1
)

:: �����༭�����첽��
echo Launching Unreal Editor with LLM memory tracing...
:: start "" "%UE_EDITOR%" "%PROJECT_PATH%" -LLM -LLMCSV -LLMPeriodicCSV=10 -Trace=Memory
start "" "%UE_EDITOR%" "%PROJECT_PATH%" -LLM -Trace=Memory

:: ��ѡ���Զ��˳����� pause��
:: �����Ҫ��������־��ȡ�������ע��
pause