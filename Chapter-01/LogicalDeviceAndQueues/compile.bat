
del *.obj *.exe *.txt

::cl /c /EHsc LogicalDeviceAndQueues.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"LogicalDeviceAndQueues.obj"
cl /c /EHsc LogicalDeviceAndQueues.cpp /I "%VULKAN_SDK%\Include" /Fo:"LogicalDeviceAndQueues.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link LogicalDeviceAndQueues.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"LogicalDeviceAndQueues.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del LogicalDeviceAndQueues.obj

LogicalDeviceAndQueues.exe 
::LogicalDeviceAndQueues.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
