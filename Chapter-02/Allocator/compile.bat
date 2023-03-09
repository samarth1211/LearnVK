
del *.obj *.exe *.txt

::cl /c /EHsc AllocatorApp.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"AllocatorApp.obj"
cl /c /EHsc AllocatorApp.cpp Allocator.cpp /I "%VULKAN_SDK%\Include" /Fo:"AllocatorApp.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link AllocatorApp.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"AllocatorApp.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del AllocatorApp.obj

AllocatorApp.exe 
::AllocatorApp.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
