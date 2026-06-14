@echo off
setlocal

set UE_ROOT=C:\Program Files\Epic Games\UE_5.4
set PROJECT=D:\BackUp\Study\CS\SourceTree\ProjectNayuta\ProjectNayuta.uproject
set OUTDIR=D:\BackUp\Study\CS\SourceTree\ProjectNayuta

if not exist "%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" (
  echo UnrealBuildTool not found at %UE_ROOT%
  pause
  exit /b 1
)

if not exist "C:\Program Files\LLVM\bin\clang-cl.exe" (
  echo LLVM clang-cl not found. Install LLVM 16.0.0 ^(UE 5.4 preferred^).
  echo Example: winget install LLVM.LLVM --version 16.0.0
  pause
  exit /b 1
)

set "PATH=C:\Program Files\LLVM\bin;%PATH%"

echo Generating compile_commands.json for clangd...
echo Close Unreal Editor before running if you want to avoid file locks.
echo.

"%UE_ROOT%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" ^
  -Mode=GenerateClangDatabase ^
  -Project="%PROJECT%" ^
  ProjectNayutaEditor Win64 Development ^
  -OutputDir="%OUTDIR%" ^
  -OutputFilename=compile_commands.json ^
  -NoExecCodeGenActions

if %ERRORLEVEL% NEQ 0 (
  echo.
  echo FAILED with exit code %ERRORLEVEL%
  pause
  exit /b %ERRORLEVEL%
)

echo.
echo Done: %OUTDIR%\compile_commands.json
echo Restart clangd in Cursor: clangd: Restart language server
pause
