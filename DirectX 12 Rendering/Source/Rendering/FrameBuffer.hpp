#pragma once
#include "../Graphics/Buffer/ConstantBuffer.hpp"


struct FrameResources
{
	XMMATRIX View;
	XMMATRIX Projection;
	XMMATRIX InversedView;
	XMMATRIX InversedProjection;
	XMFLOAT2 RenderTargetDims;
	float NearZ;
	float FarZ;
	float TotalTime;
	float DeltaTime;
	XMFLOAT4 Ambient;
};

class FrameBuffer
{



};

