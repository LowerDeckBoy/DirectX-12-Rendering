#pragma once
#include <d3d12.h>
#include <wrl.h>
#include <DirectXMath.h>
#include "../Graphics/Buffer/ConstantBuffer.hpp"
#include <vector>
#include <memory>

class DeviceContext;
class VertexBuffer;
class IndexBuffer;
class Model;

class BottomLevel
{
public:
	~BottomLevel();

	void Create(ID3D12GraphicsCommandList4* pCommandList, ID3D12Resource* pScratch, ID3D12Resource* pResult, bool bAllowUpdate = false);
	// TODO: Change buffer into Mesh reference for models
	//void AddBuffers(VertexBuffer Vertex, IndexBuffer Index, bool bOpaque);
	void AddBuffers(D3D12_VERTEX_BUFFER_VIEW& VertexView, uint32_t VertexCount, D3D12_INDEX_BUFFER_VIEW& IndexView, uint32_t IndexCount, bool bOpaque);
	// TEST
	//void AddBuffers(std::vector<VertexBuffer> Vertices, std::vector<IndexBuffer> Indices, bool bOpaque);

	void GetBufferSizes(ID3D12Device5* pDevice, uint64_t* pScratchSize, uint64_t* pResultSize, bool bAllowUpdate);

	void Reset();

	inline ID3D12Resource* GetBuffer() { return m_ResultBuffer.Get(); }

	// Temporal storage
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ScratchBuffer;
	// Estimated memory requirement aka output
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ResultBuffer;
private:

	std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> m_Buffers;

	uint64_t m_ScratchSize{ 0 };
	uint64_t m_ResultSize { 0 };

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_Flags{ D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_MINIMIZE_MEMORY };
};

struct Instance
{
	Instance(ID3D12Resource* pBottomLevelBuffer, DirectX::XMMATRIX& Matrix, uint32_t InstanceID, uint32_t HitGroupID) noexcept
		: BottomLevel(pBottomLevelBuffer), Matrix(Matrix), InstanceID(InstanceID), HitGroupID(HitGroupID)
	{ }
	~Instance() { BottomLevel = nullptr; }

	ID3D12Resource*		BottomLevel{ nullptr };
	DirectX::XMMATRIX	Matrix{ DirectX::XMMatrixIdentity() };
	uint32_t			InstanceID;
	uint32_t			HitGroupID;
};

class TopLevel
{
private:

public:
	~TopLevel();

	void Create(ID3D12GraphicsCommandList4* pCommandList,
		ID3D12Resource* pScratch,
		ID3D12Resource* pResult,
		ID3D12Resource* pDescriptors,
		bool bUpdateOnly = false,
		ID3D12Resource* pPreviousResult = nullptr);

	void AddInstance(ID3D12Resource* pBottomLevelResult, DirectX::XMMATRIX Matrix, uint32_t InstanceID, uint32_t HitGroupID);
	void AddInstance(Instance* pInstance);

	void GetBufferSizes(ID3D12Device5* pDevice, uint64_t* pScratchSize, uint64_t* pResultSize, uint64_t* pDescSize, bool bAllowUpdate);
	//void GetBufferSizes(ID3D12Device5* pDevice, std::vector<Instance*> Instances, uint64_t* pScratchSize, uint64_t* pResultSize, uint64_t* pDescSize, bool bAllowUpdate);

	void Reset();

	Microsoft::WRL::ComPtr<ID3D12Resource> m_ScratchBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_ResultBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_InstanceDescsBuffer;

private:

	std::vector<Instance*> m_Instances;
	//std::vector<std::unique_ptr<Instance>> m_Instances;

	uint64_t m_ScratchSize		{ 0 };
	uint64_t m_ResultSize		{ 0 };
	uint64_t m_InstanceDescsSize{ 0 };

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS m_Flags{ D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE };

};

class AccelerationStructures
{
public:
	void Init(DeviceContext* pDeviceCtx) noexcept { m_Device = pDeviceCtx; }

	void CreateBottomLevels(VertexBuffer& Vertex, IndexBuffer& Index, bool bOpaque = true);
	void CreateBottomLevels(std::vector<VertexBuffer>& VertexBuffers, std::vector<IndexBuffer>& IndexBuffers, bool bOpaque = true);
	void CreateBottomLevels(std::vector<std::unique_ptr<Model>>& Models, bool bOpaque = true);

	void CreateTopLevel();
	void CreateTopLevel(ID3D12Resource* pBuffer, DirectX::XMMATRIX& Matrix);
	//void CreateTopLevel(std::vector<BottomLevel>& pBuffers, DirectX::XMMATRIX& Matrix);
	//void CreateTopLevel(std::vector<ID3D12Resource*> pBuffers, DirectX::XMMATRIX& Matrix);

	BottomLevel m_BottomLevel;
	TopLevel	m_TopLevel;
	//std::unique_ptr<TopLevel> m_TopLevel;


private:
	DeviceContext* m_Device{ nullptr };
	std::vector<BottomLevel*> m_BottomLevels;
	//std::vector<std::unique_ptr<BottomLevel>> m_BLASes;

};
