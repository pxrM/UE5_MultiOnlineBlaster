@echo off

:: 配置区
set "UE_EDITOR=C:\Program Files\Epic Games\UE_5.3\Engine\Binaries\Win64\UnrealEditor.exe"
set "DEFAULT_PROJECT=E:\Project\UE\UE5_MultiOnlineBlaster\Blaster\Blaster.uproject"

:: 使用传入参数或默认项目
set "PROJECT_PATH=%~1"
if "%PROJECT_PATH%"=="" set "PROJECT_PATH=%DEFAULT_PROJECT%"

:: 检查文件是否存在
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

:: 启动编辑器（异步）
echo Launching Unreal Editor with LLM memory tracing...
:: start "" "%UE_EDITOR%" "%PROJECT_PATH%" -LLM -LLMCSV -LLMPeriodicCSV=10 -Trace=Memory
start "" "%UE_EDITOR%" "%PROJECT_PATH%" -LLM -Trace=Memory

:: 可选：自动退出（不 pause）
:: 如果需要看启动日志，取消下面的注释
pause