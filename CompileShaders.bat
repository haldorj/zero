@echo off

"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/gradient.comp" -o "shaders/compiled/gradient.comp.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/gradient_color.comp" -o "shaders/compiled/gradient_color.comp.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/sky.comp" -o "shaders/compiled/sky.comp.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/plain_shader.vert" -o "shaders/compiled/plain_shader.vert.spv"
"%VULKAN_SDK%\Bin\glslangValidator.exe" -V "shaders/plain_shader.frag" -o "shaders/compiled/plain_shader.frag.spv"


pause
