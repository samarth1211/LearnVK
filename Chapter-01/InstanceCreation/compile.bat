
del *.obj *.exe *.txt

cl /c /EHsc InstanceCreation.cpp /I "%VULKAN_SDK%\Include" /Fo:"InstanceCreation.obj"

link InstanceCreation.obj /LIBPATH:"%VULKAN_SDK%\Lib" vulkan-1.lib VkLayer_utils.lib /OUT:"InstanceCreation.exe"

del InstanceCreation.obj

InstanceCreation.exe
