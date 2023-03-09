
del *.obj *.exe *.txt

::cl /c /EHsc DeviceMemory.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"DeviceMemory.obj"
cl /c /EHsc DeviceMemory.cpp /I "%VULKAN_SDK%\Include" /Fo:"DeviceMemory.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link DeviceMemory.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"DeviceMemory.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del DeviceMemory.obj

DeviceMemory.exe 
::DeviceMemory.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
