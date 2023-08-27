#pragma once

// Const Types
struct cbPerObject
{
	XMMATRIX WVP{ XMMatrixIdentity() };
	XMMATRIX World{ XMMatrixIdentity() };
	//float padding[32]{};
};

struct cbCamera
{
	alignas(16) XMFLOAT3 CameraPosition;
};

struct SceneConstData
{
	XMVECTOR Position;
	XMMATRIX View;
	XMMATRIX Projection;
	XMMATRIX InversedView;
	XMMATRIX InversedProjection;
	XMFLOAT2 ScreenDimension;
};

// Mainly for glTF model purposes
struct cbMaterial
{
	XMFLOAT4 CameraPosition{ XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f) };

	XMFLOAT4 BaseColorFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0) };
	XMFLOAT4 EmissiveColorFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0) };

	float MetallicFactor{ 1.0f };
	float RougnessFactor{ 1.0f };
	float AlphaCutoff{ 0.5f };
	BOOL bDoubleSided{ FALSE };

	int32_t BaseColorIndex{ -1 };
	int32_t NormalIndex{ -1 };
	int32_t MetallicRoughnessIndex{ -1 };
	int32_t EmissiveIndex{ -1 };
};

struct cbLights
{
	std::array<XMFLOAT4, 4> LightPositions;
	std::array<XMFLOAT4, 4> LightColors;
	uint32_t LightsCount{ 4 };
};
