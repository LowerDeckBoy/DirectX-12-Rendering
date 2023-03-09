#pragma once

struct Mesh;

// 
namespace model
{
	class Node
	{
	public:
		Node() = default;
		Node(const Node& rhs)
		{
			Parent = rhs.Parent;
			Children = rhs.Children;
			Index = rhs.Index;
			Name = rhs.Name;
			Mesh = rhs.Mesh;
			Matrix = rhs.Matrix;
			Translation = rhs.Translation;
			Rotation = rhs.Rotation;
			Scale = rhs.Scale;
			auto msg{ "Node: " + Name + " copied.\n" };
			::OutputDebugStringA(msg.c_str());
		}
		Node& operator=(const Node& rhs)
		{
			//Node* newNode{ new Node() };
			//newNode->Parent = rhs.Parent;
			//newNode->Children = rhs.Children;
			//newNode->Index = rhs.Index;
			//newNode->Name = rhs.Name;
			//newNode->Mesh = rhs.Mesh;
			//newNode->Matrix = rhs.Matrix;
			//newNode->Translation = rhs.Translation;
			//newNode->Rotation = rhs.Rotation;
			//newNode->Scale = rhs.Scale;
			//return *newNode;
			auto msg{ "Node: " + Name + " copied.\n" };
			::OutputDebugStringA(msg.c_str());
			return *this;
		}
		~Node()
		{
			Release();
		}

		Node* Parent{ nullptr };
		std::vector<Node*> Children;
		uint32_t Index{ 0 };
		std::string Name{ "None given" };
		Mesh* Mesh{ nullptr };
		DirectX::XMMATRIX Matrix{ DirectX::XMMatrixIdentity() };
		DirectX::XMFLOAT3 Translation{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f) };
		DirectX::XMFLOAT4 Rotation{ DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
		DirectX::XMFLOAT3 Scale{ DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f) };

		DirectX::XMMATRIX GetLocalMatrix()
		{
			DirectX::XMVECTOR scale{ XMLoadFloat3(&Scale) };
			DirectX::XMVECTOR rotation{ XMLoadFloat4(&Rotation) };
			DirectX::XMVECTOR translation{ XMLoadFloat3(&Translation) };

			return (Matrix * XMMatrixScalingFromVector(scale) * XMMatrixRotationQuaternion(rotation) * XMMatrixTranslationFromVector(translation));
		}

		DirectX::XMMATRIX GetMatrix()
		{
			//DirectX::XMMATRIX matrix{ Matrix };
			DirectX::XMMATRIX matrix{ GetLocalMatrix() };
			model::Node* parent{ Parent };
			while (parent != nullptr)
			{
				matrix = parent->GetLocalMatrix() * matrix;
				//matrix = parent->Matrix * matrix;
				parent = parent->Parent;
			}

			return matrix;
		}

		void Release()
		{
			for (auto& child : Children)
				delete child;

			//delete Mesh;
			delete Parent;
		}
	};
}
