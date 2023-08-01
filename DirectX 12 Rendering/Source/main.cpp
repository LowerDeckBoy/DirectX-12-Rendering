#include "Engine/Engine.hpp"

// https://learn.microsoft.com/en-us/windows/win32/direct3d12/command-queues-and-command-lists

// https://en.wikipedia.org/wiki/Cube_mapping
// https://en.wikipedia.org/wiki/Global_illumination

// https://github.com/ocornut/imgui/discussions/4942
// https://github.com/GPUOpen-LibrariesAndSDKs/Capsaicin

// TODO:
// https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_4/nf-dxgi1_4-idxgiadapter3-queryvideomemoryinfo
// https://developer.nvidia.com/dx12-dos-and-donts

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
