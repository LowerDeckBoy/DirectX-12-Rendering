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
		XMMATRIX Matrix{ DirectX::XMMatrixIdentity() };
		XMFLOAT3 Translation{ XMFLOAT3(0.0f, 0.0f, 0.0f) };
		XMFLOAT4 Rotation{ XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) };
		XMFLOAT3 Scale{ XMFLOAT3(1.0f, 1.0f, 1.0f) };
		XMMATRIX LocalMatrix{ DirectX::XMMatrixIdentity() };

		XMMATRIX GetLocalMatrix()
		{
			XMMATRIX translation{ XMMatrixTranslationFromVector(XMLoadFloat3(&Translation)) };
			XMMATRIX rotation{  XMMatrixRotationQuaternion(XMLoadFloat4(&Rotation)) };
			XMMATRIX scale{ XMMatrixScalingFromVector(XMLoadFloat3(&Scale)) };
			//Matrix * 
			//return (XMMatrixScalingFromVector(scale) * XMMatrixRotationQuaternion(rotation) * XMMatrixTranslationFromVector(translation) * Matrix);
			//return scale * rotation * translation * Matrix;
			//return translation * rotation * scale * Matrix;
			//return translation * rotation * scale * LocalMatrix;
			return LocalMatrix * scale * rotation * translation;
		}



		XMMATRIX GetMatrix()
		{
			//DirectX::XMMATRIX matrix{ Matrix };
			XMMATRIX matrix{ GetLocalMatrix() };
			model::Node* parent{ Parent };
			while (parent != nullptr)
			{
				matrix = parent->Matrix * matrix;
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
