
del *.obj *.exe *.txt

cl /c /EHsc SetLayers.cpp /I "%VULKAN_SDK%\Include" /Fo:"SetLayers.obj"

link SetLayers.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /OUT:"SetLayers.exe"

del SetLayers.obj

SetLayers.exe
