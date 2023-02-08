#include "Source/Core/App.hpp"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	App app(hInstance);
	try {
		app.Initialize();
		app.Run();
	}
	catch (...) {
		return -1;
	}

	return 0;
}