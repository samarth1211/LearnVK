
del *.obj *.exe *.txt

::cl /c /EHsc SubmitCommandBuffer.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"SubmitCommandBuffer.obj"
cl /c /EHsc SubmitCommandBuffer.cpp /I "%VULKAN_SDK%\Include" /Fo:"SubmitCommandBuffer.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link SubmitCommandBuffer.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"SubmitCommandBuffer.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del SubmitCommandBuffer.obj

SubmitCommandBuffer.exe 
::SubmitCommandBuffer.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
