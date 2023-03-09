
del *.obj *.exe *.txt

::cl /c /EHsc CreateCommandBuffers.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"CreateCommandBuffers.obj"
cl /c /EHsc CreateCommandBuffers.cpp /I "%VULKAN_SDK%\Include" /Fo:"CreateCommandBuffers.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link CreateCommandBuffers.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"CreateCommandBuffers.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del CreateCommandBuffers.obj

CreateCommandBuffers.exe 
::CreateCommandBuffers.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
