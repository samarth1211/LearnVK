
del *.obj *.exe *.txt

::cl /c /EHsc PhysicalDevices.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"PhysicalDevices.obj"
cl /c /EHsc PhysicalDevices.cpp /I "%VULKAN_SDK%\Include" /Fo:"PhysicalDevices.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link PhysicalDevices.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"PhysicalDevices.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del PhysicalDevices.obj

PhysicalDevices.exe 
::PhysicalDevices.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
