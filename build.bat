@echo off
chcp 65001 > nul
call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" >nul 2>&1
if errorlevel 1 (
    for /f "delims=" %%i in ('where /r "C:\Program Files" VsDevCmd.bat 2^>nul') do (
        call "%%i" >nul 2>&1
        goto build
    )
)
:build
cd /d "c:\forClass\25B\25B_GE_CG"
msbuild 25B_GE_CG.vcxproj /p:Configuration=Debug /p:Platform=x64 /v:minimal /nologo 2>&1

