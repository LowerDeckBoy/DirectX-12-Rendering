#include "Engine/Engine.hpp"

// https://en.wikipedia.org/wiki/Cube_mapping
// https://en.wikipedia.org/wiki/Global_illumination

// https://pl.wikipedia.org/wiki/BRDF
// https://en.wikipedia.org/wiki/Bidirectional_reflectance_distribution_function

// https://media.disneyanimation.com/uploads/production/publication_asset/48/asset/s2012_pbs_disney_brdf_notes_v3.pdf

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int)
{
	
	Engine* app = new Engine(hInstance);
	try {
		app->Initialize();
		app->Run();
	}
	catch (...) {
		delete app;
		return -1;
	}

	delete app;
	//_CrtDumpMemoryLeaks();
	return 0;
}
