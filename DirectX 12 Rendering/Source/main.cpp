//#include "Core/App.hpp"
#include "Engine/Engine.hpp"

//https://learn.microsoft.com/en-us/windows/win32/direct3d12/command-queues-and-command-lists

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	Engine app(hInstance);
	try {
		app.Initialize();
		app.Run();
	}
	catch (...) {
		return -1;
	}

	return 0;
}