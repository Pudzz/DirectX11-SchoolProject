#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "Appsys.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) 
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	bool result;
	Appsys application(hInstance);
		
	// Init and run the system.
	result = application.Initialize();
	if (result)
		application.Run();	
	
	application.Shutdown();
	
	return 0;
}