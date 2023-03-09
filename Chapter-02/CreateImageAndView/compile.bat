
del *.obj *.exe *.txt

::cl /c /EHsc CreateImageApp.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"CreateImageApp.obj"
cl /c /EHsc CreateImageApp.cpp /I "%VULKAN_SDK%\Include" /Fo:"CreateImageApp.obj"
IF %ERRORLEVEL% NEQ 0 goto ERROR

link CreateImageApp.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"CreateImageApp.exe"
IF %ERRORLEVEL% NEQ 0 goto ERROR

del CreateImageApp.obj

CreateImageApp.exe 
::CreateImageApp.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto LEAVEBAT

:ERROR
    echo "Something failed"
    goto LEAVEBAT

:LEAVEBAT
