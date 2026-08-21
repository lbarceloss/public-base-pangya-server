@echo off
setlocal
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars32.bat" >nul
set SRC=C:\Users\lbarc\Desktop\Abrir Map Completo\SuperSS-Dev\Server Lib\Projeto IOCP
cd /d "%~dp0"
cl /nologo /EHsc /std:c++17 /MD /DWIN32 /D_WINDOWS /DWINDOWS_IGNORE_PACKING_MISMATCH /D_CRT_SECURE_NO_WARNINGS ^
   /I"%SRC%" ^
   test_sendq.cpp ^
   "%SRC%\SOCKET\session.cpp" ^
   "%SRC%\UTIL\buffer.cpp" ^
   "%SRC%\UTIL\exception.cpp" ^
   "%SRC%\UTIL\message.cpp" ^
   "%SRC%\PACKET\packet.cpp" ^
   "%SRC%\CRYPT\crypt.cpp" ^
   "%SRC%\COMPRESS\compress.cpp" ^
   "%SRC%\UTIL\hex_util.cpp" ^
   "%SRC%\UTIL\util_time.cpp" ^
   "%SRC%\COMPRESS\minilzo.c" ^
   /Fe:test_sendq.exe
endlocal
