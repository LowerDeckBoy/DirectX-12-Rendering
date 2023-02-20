#pragma once
#include <DirectXMath.h>

struct Vertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TexCoord;
};

struct VertexUV
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT2 TexCoord;
};
struct CubeVertex
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
};