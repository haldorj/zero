#include <Application.h>

int main()
{
	Zero::Application application;

	application.Init();
	application.Run();
	application.Cleanup();

	return 0;
}