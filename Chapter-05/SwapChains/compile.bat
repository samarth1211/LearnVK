cls

del *.exe *.txt

::cl /c /EHsc SwapChains.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /D "DEBUG" /D "_DEBUG" /Fo:"SwapChains.obj"
cl /c /EHsc SwapChains.cpp /I "%VULKAN_SDK%\Include" /Fo:"SwapChains.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link SwapChains.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"SwapChains.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del SwapChains.obj

SwapChains.exe 
::SwapChains.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
