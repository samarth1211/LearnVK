
del *.obj *.exe *.txt

::cl /c /EHsc CreateBufferApp.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"CreateBufferApp.obj"
cl /c /EHsc CreateBufferApp.cpp /I "%VULKAN_SDK%\Include" /Fo:"CreateBufferApp.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link CreateBufferApp.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"CreateBufferApp.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del CreateBufferApp.obj

CreateBufferApp.exe 
::CreateBufferApp.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
