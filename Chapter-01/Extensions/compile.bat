
del *.obj *.exe *.txt

cl /c /EHsc SetExtensions.cpp /I "%VULKAN_SDK%\Include" /Fo:"SetExtensions.obj"

link SetExtensions.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /OUT:"SetExtensions.exe"

del SetExtensions.obj

SetExtensions.exe
