#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vk_engine.h>

int main(int argc, char* argv[])
{
	Engine engine;

	engine.init();

	engine.run();

	engine.cleanup();

	return 0;
}