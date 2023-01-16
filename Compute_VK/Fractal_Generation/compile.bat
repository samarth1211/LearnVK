del *.exe *.spv
glslangValidator -V Adder.comp -o Comp.spv
IF %ERRORLEVEL% NEQ 0 goto error

cl /nologo /EHsc Source.cpp /I "%VULKAN_SDK%\Include" /link /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib
IF %ERRORLEVEL% EQ 0 goto leavebat 
IF %ERRORLEVEL% NEQ 0 goto error 

:error
    echo "Something failed"
    goto leavebat

:leavebat

del *.obj
