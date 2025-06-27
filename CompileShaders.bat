@echo off

"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/gradient.comp" -o "shaders/compiled/gradient.comp.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/gradient_color.comp" -o "shaders/compiled/gradient_color.comp.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/sky.comp" -o "shaders/compiled/sky.comp.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/textured.vert" -o "shaders/compiled/textured.vert.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/textured.frag" -o "shaders/compiled/textured.frag.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/plain.frag" -o "shaders/compiled/plain.frag.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/phongvk.vert" -o "shaders/compiled/phongvk.vert.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/phongvk.frag" -o "shaders/compiled/phongvk.frag.spv"

"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/Vulkan/default_vk.vert" -o "shaders/Vulkan/Compiled/default_vk.vert.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/Vulkan/default_vk.frag" -o "shaders/Vulkan/Compiled/default_vk.frag.spv"

"%VULKAN_SDK%\Bin\spirv-val.exe" --target-env vulkan1.3 --scalar-block-layout "shaders/Vulkan/Compiled/default_vk.vert.spv"
"%VULKAN_SDK%\Bin\spirv-val.exe" --target-env vulkan1.3 --scalar-block-layout "shaders/Vulkan/Compiled/default_vk.frag.spv"

pause
