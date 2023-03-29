cls

del *.exe *.txt

::cl /c /EHsc PerformPresentation.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /D "DEBUG" /D "_DEBUG" /Fo:"PerformPresentation.obj"
cl /c /EHsc PerformPresentation.cpp /I "%VULKAN_SDK%\Include" /Fo:"PerformPresentation.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link PerformPresentation.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"PerformPresentation.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del PerformPresentation.obj

PerformPresentation.exe 
::PerformPresentation.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
