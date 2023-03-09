
del *.obj *.exe *.txt

::cl /c /EHsc practice.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /Fo:"practice.obj"
cl /c /EHsc practice.cpp /I "%VULKAN_SDK%\Include" /Fo:"practice.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link practice.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"practice.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del practice.obj

practice.exe 
::practice.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
