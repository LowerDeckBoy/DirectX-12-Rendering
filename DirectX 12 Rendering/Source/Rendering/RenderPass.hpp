#pragma once
#include <d3d12.h>
#include "DeferredContext.hpp"


//https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist4-beginrenderpass
class RenderPass
{
public:
	// TODO:
	
	void GBufferPass();
	void LightingPass();
	void ShadowPass();

	std::unique_ptr<DeferredContext> m_DeferredContext;

};
