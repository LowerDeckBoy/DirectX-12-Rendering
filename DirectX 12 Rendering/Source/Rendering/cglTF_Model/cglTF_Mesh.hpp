#pragma once
#include "../../Graphics/Vertex.hpp"
#include "../../Graphics/Texture.hpp"
#include "../../Graphics/Buffer.hpp"
#include <vector>
#include <DirectXMath.h>
using namespace DirectX;

//class Texture;

namespace cglTF 
{
	class Material
	{
	public:
		XMFLOAT4 BaseColorFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };
		XMFLOAT4 EmissiveFactor{ XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) };

		float MetallicFactor{ 1.0f };
		float RoughnessFactor{ 1.0f };
		float AlphaCutoff{ 0.5f };

		Texture* BaseColorTexture{ nullptr };
		Texture* NormalTexture	 { nullptr };
		Texture* MetallicTexture { nullptr };
		Texture* EmissiveTexture { nullptr };
	};
	
	class Primitive {
	public:
		uint32_t FirstIndexLocation{ 0 };
		uint32_t BaseVertexLocation{ 0 };

		uint32_t IndexCount{ 0 };
		uint32_t VertexCount{ 0 };
		uint32_t StartVertexLocation{ 0 };
		bool bHasIndices{ false };

		Material Material;
	};

	class Mesh
	{
	public:
		Mesh() = default;
		std::string Name;
		XMMATRIX Matrix{ XMMatrixIdentity() };
		std::vector<Primitive*> Primitives;
	
		//Material Material;
	};

	class Node
	{
	public:
		Node* Parent;
		std::vector<Node*> Children;
		std::string Name{ "None Given" };

		Mesh* Mesh{ nullptr };

		XMMATRIX Matrix		{ XMMatrixIdentity() };
		XMMATRIX LocalMatrix{ XMMatrixIdentity() };
		XMFLOAT3 Translation{ XMFLOAT3(0.0f, 0.0f, 0.0f) };
		XMFLOAT4 Rotation	{ XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
		XMFLOAT3 Scale		{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
		
	};
}
