
del *.obj *.exe *.txt

::cl /c /EHsc ObtainQueue.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"ObtainQueue.obj"
cl /c /EHsc ObtainQueue.cpp /I "%VULKAN_SDK%\Include" /Fo:"ObtainQueue.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link ObtainQueue.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"ObtainQueue.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del ObtainQueue.obj

ObtainQueue.exe 
::ObtainQueue.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
