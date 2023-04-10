#pragma once
#include "../../Graphics/Texture.hpp"
#include "../../Graphics/Vertex.hpp"
#include <DirectXMath.h>
using namespace DirectX;

namespace model 
{
	struct Material
	{
		XMFLOAT4 BaseColorFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
		XMFLOAT4 EmissiveFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };

		float MetallicFactor{ 1.0f };
		float RoughnessFactor{ 1.0f };
		float AlphaCutoff{ 0.5f };

		Texture* BaseColorTexture{ nullptr };
		Texture* NormalTexture{ nullptr };
		Texture* MetallicRoughnessTexture{ nullptr };
		Texture* EmissiveTexture{ nullptr };
	};

	struct Mesh
	{
		std::string_view Name;
		XMMATRIX Matrix{ XMMatrixIdentity() };

		uint32_t FirstIndexLocation{ 0 };
		uint32_t BaseVertexLocation{ 0 };

		uint32_t IndexCount{ 0 };
		uint32_t VertexCount{ 0 };
		uint32_t StartVertexLocation{ 0 };
		bool bHasIndices{ false };

	};

	struct Node
	{
		Node* Parent{ nullptr };
		std::vector<Node*> Children;
		std::string Name;

		XMMATRIX Matrix{ XMMatrixIdentity() };
		XMFLOAT3 Translation{ XMFLOAT3(0.0f, 0.0f, 0.0f) };
		XMFLOAT4 Rotation{ XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
		XMFLOAT3 Scale{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
	};
}