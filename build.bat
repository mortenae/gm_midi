@echo off
call "%vs110comntools%vsvars32.bat"
cl /nologo /LD /GL /O2 /Igmod-module-base\include /Fegmcl_midi_win32.dll main.cpp /link /ltcg kernel32.lib user32.lib winmm.lib
if errorlevel 1 pause
copy "gmcl_midi_win32.dll" "C:\Program Files (x86)\Steam\SteamApps\animorten\garrysmod\garrysmod\lua\bin"
