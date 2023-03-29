cls

del *.exe *.txt

::cl /c /EHsc CmdWin.cpp /I "%VULKAN_SDK%\Include" /D "_UNICODE" /D "UNICODE" /D "DEBUG" /D "_DEBUG" /Fo:"CmdWin.obj"
cl /c /EHsc CmdWin.cpp /I "%VULKAN_SDK%\Include" /Fo:"CmdWin.obj"
IF %ERRORLEVEL% NEQ 0 goto error

link CmdWin.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /MACHINE:X64 /SUBSYSTEM:CONSOLE /OUT:"CmdWin.exe"
IF %ERRORLEVEL% NEQ 0 goto error

del CmdWin.obj

CmdWin.exe 
::CmdWin.exe 2>&1>log.txt
if %ERRORLEVEL% EQU 0 goto leavebat


:error
    echo "Something failed"
    goto leavebat

:leavebat
