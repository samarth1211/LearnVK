
del *.obj *.exe *.txt

::cl /c /EHsc RecordingCommandBuffer.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"RecordingCommandBuffer.obj"
cl /c /EHsc RecordingCommandBuffer.cpp /I "%VULKAN_SDK%\Include" /Fo:"RecordingCommandBuffer.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link RecordingCommandBuffer.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"RecordingCommandBuffer.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del RecordingCommandBuffer.obj

RecordingCommandBuffer.exe 
::RecordingCommandBuffer.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
