
del *.obj *.exe *.txt

::cl /c /EHsc RecyclingCommandBuffer.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"RecyclingCommandBuffer.obj"
cl /c /EHsc RecyclingCommandBuffer.cpp /I "%VULKAN_SDK%\Include" /Fo:"RecyclingCommandBuffer.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link RecyclingCommandBuffer.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"RecyclingCommandBuffer.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del RecyclingCommandBuffer.obj

RecyclingCommandBuffer.exe 
::RecyclingCommandBuffer.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
