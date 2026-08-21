@echo off
REM Inicia o backend de calculo Smart Calculator (escuta em 127.0.0.1:12345).
REM A DLL SMARTCALCULATORLIB.dll do Game Server conecta nele pro comando "calc".
cd /d "%~dp0"
echo Iniciando Smart Calculator backend (porta 12345)... feche esta janela pra parar.
node smart.js
pause
