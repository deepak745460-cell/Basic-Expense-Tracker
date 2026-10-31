@echo off
echo Compiling...
set PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%
g++ main.cpp -o main.exe -I"C:\msys64\ucrt64\include" -lws2_32 -lpthread -lmswsock -static-libgcc -static-libstdc++ -static
if %errorlevel% == 0 (
    echo.
    echo SUCCESS! main.exe ready to run.
) else (
    echo.
    echo FAILED! Check errors above.
)
pause
