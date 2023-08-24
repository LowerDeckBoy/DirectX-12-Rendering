#include "Engine/Engine.hpp"

// https://en.wikipedia.org/wiki/Cube_mapping
// https://en.wikipedia.org/wiki/Global_illumination

// TODO:
// https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_4/nf-dxgi1_4-idxgiadapter3-queryvideomemoryinfo
// https://developer.nvidia.com/dx12-dos-and-donts

// https://pl.wikipedia.org/wiki/BRDF
// https://en.wikipedia.org/wiki/Bidirectional_reflectance_distribution_function

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	
	Engine* app = new Engine(hInstance);
	try {
		app->Initialize();
		app->Run();
	}
	catch (...) {
		::MessageBoxA(nullptr, "Failed to run!", "Error", MB_OK);
		delete app;
		return -1;
	}

	delete app;
	return 0;
}
