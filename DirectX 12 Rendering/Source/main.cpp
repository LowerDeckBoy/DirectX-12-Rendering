#include "Engine/Engine.hpp"

// https://learn.microsoft.com/en-us/windows/win32/direct3d12/command-queues-and-command-lists

// https://en.wikipedia.org/wiki/Cube_mapping
// https://en.wikipedia.org/wiki/Global_illumination

// TODO:
// https://alextardif.com/Bindless.html
// https://alain.xyz/blog/raw-directx12#initialize-api

// https://github.com/ocornut/imgui/discussions/4942

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
