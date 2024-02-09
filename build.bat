echo off
cls
echo on
cl /Gr /EHsc /O2 /Os .\src\*.cpp /I .\src\headers /link /OUT:".\bin\hc.exe" user32.lib gdi32.lib
del *.obj
.\bin\hc.exe