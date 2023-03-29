cls

del *.exe *.txt

::cl /c /EHsc PresentationExtension.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /D "DEBUG" /D "_DEBUG" /Fo:"PresentationExtension.obj"
cl /c /EHsc PresentationExtension.cpp /I "%VULKAN_SDK%\Include" /Fo:"PresentationExtension.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link PresentationExtension.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"PresentationExtension.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del PresentationExtension.obj

PresentationExtension.exe 
::PresentationExtension.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
