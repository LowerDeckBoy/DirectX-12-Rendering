#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include "../Core/DeviceContext.hpp"
#include "../Graphics/Buffer/ConstantBuffer.hpp"
#include "../Graphics/Shader.hpp"
#include "AccelerationStructures.hpp"
#include "ShaderTable.hpp"
#include "../Graphics/ShaderManager.hpp"

class Camera;
class VertexBuffer;
class IndexBuffer;
class Model;


// Scene buffer for 3D
struct RaytraceBuffer
{
	DirectX::XMMATRIX ViewProjection;
	DirectX::XMVECTOR CameraPosition;
	DirectX::XMVECTOR LightPosition;
	DirectX::XMVECTOR LightAmbient;
	DirectX::XMVECTOR LightDiffuse;
};

struct CameraBuffer
{
	DirectX::XMMATRIX View;
	DirectX::XMMATRIX Projection;
	DirectX::XMMATRIX InversedView;
	DirectX::XMMATRIX InversedProjection;
};

struct CubeBuffer
{
	DirectX::XMVECTOR CubeColor;
};

// TODO:
enum LocalRootArguments
{
	eAlbedo = 0,
	eTopLevelReference
};

enum GlobalRootArguments
{
	eOutputUAV = 0,
	eTopLevel,
	eCameraBuffer,
	eSceneBuffer,
	eTexResolution
};

class RaytracingContext
{
public:
	//RaytracingContext(DeviceContext* pDeviceCtx, ShaderManager* pShaderManager, Camera* pCamera, std::vector<VertexBuffer>& Vertex, std::vector<IndexBuffer>& Index, std::vector<ConstantBuffer<cbPerObject>>& ConstBuffers);
	RaytracingContext(DeviceContext* pDeviceCtx, ShaderManager* pShaderManager, Camera* pCamera, std::vector<std::unique_ptr<Model>>& Models);
	~RaytracingContext() noexcept(false);

	void Create();
	void OnRaytrace();
	void DispatchRaytrace();

	void BuildAccelerationStructures(std::vector<std::unique_ptr<Model>>& ModelsRef);
	void CreateRootSignatures();
	void CreateStateObject();
	void CreateOutputResource();

	void BuildShaderTables();

	void OutputToBackbuffer();

	void OnResize();

	[[maybe_unused]]
	void SerializeAndCreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& Desc, ComPtr<ID3D12RootSignature>* ppRootSignature) const;

	//test
	void SetConstBufferData();

	void DrawGUI();

	// Getters
	inline Descriptor const& GetOutputDescriptor() const { return m_OutputUAV; }
	inline Descriptor const& GetTLASDescriptor() const { return m_TopLevelView; }

private:
	DeviceContext* m_DeviceCtx{ nullptr };
	ShaderManager* m_ShaderManager{ nullptr };
	Camera* m_Camera{ nullptr };

	std::unique_ptr<DescriptorHeap> m_RaytracingHeap;

	ComPtr<ID3D12StateObject> m_StateObject;
	ComPtr<ID3D12StateObjectProperties> m_StateObjectProperties;

	ComPtr<ID3D12Resource> m_RaytracingOutput;
	Descriptor m_OutputUAV;
	Descriptor m_TopLevelView;

	// Shaders data
	ComPtr<IDxcBlob> m_RayGenShader;
	ComPtr<IDxcBlob> m_MissShader;
	ComPtr<IDxcBlob> m_HitShader;
	ComPtr<IDxcBlob> m_ShadowShader;

	// Shader Root Signatures
	ComPtr<ID3D12RootSignature> m_GlobalRootSignature;
	ComPtr<ID3D12RootSignature> m_MissRootSignature;
	ComPtr<ID3D12RootSignature> m_ClosestHitRootSignature;
	ComPtr<ID3D12RootSignature> m_ShadowRootSignature;

	uint32_t m_PayloadSize		{ 0 };
	uint32_t m_AttributeSize	{ 0 };
	// 1 -> Primary rays
	// 2 -> Simple shadows
	uint32_t m_MaxRecursiveDepth{ 2 };

	AccelerationStructures m_AS;

	// Geometry for lookup
	//VertexBuffer m_VertexBuffer;
	//IndexBuffer  m_IndexBuffer;

	std::vector<VertexBuffer> m_VertexBuffers;
	std::vector<IndexBuffer> m_IndexBuffers;
	std::vector<ConstantBuffer<cbPerObject>> m_ConstBuffers;

	// Shader Table
	ShaderTable m_RayGenTable;
	ShaderTable m_MissTable;
	ShaderTable m_HitTable;
	//
	//ShaderTable m_ShadowTable;

	//std::vector<Model*> m_Models;

	// For single buffer shader table storage
	//std::unique_ptr<ShaderTableBuilder> m_ShaderTableBuilder;
	//ComPtr<ID3D12Resource> m_ShaderTableStorage;

	void UpdateCamera();
	// Const buffer for 3D
	ConstantBuffer<RaytraceBuffer> m_SceneBuffer;
	RaytraceBuffer m_SceneData{};
	ConstantBuffer<CameraBuffer> m_CameraBuffer;
	CameraBuffer m_CameraData{};
	ConstantBuffer<CubeBuffer> m_CubeBuffer;
	CubeBuffer m_CubeData{};

	// GlobalCB

	// Light data for shading
	std::array<float, 3> m_LightPosition{ 0.0f, 1.5f, -8.0f };
	std::array<float, 4> m_LightAmbient { 0.5f, 0.5f, 0.5f, 1.0f };
	std::array<float, 4> m_LightDiffuse { 0.960f, 0.416f, 0.416f, 1.0f };

	// Shader names
	// Those are associate with names given inside Raytracing shaders
	// Thus those MUST match
	static const wchar_t* m_HitGroupName;
	static const wchar_t* m_RayGenShaderName;
	static const wchar_t* m_MissShaderName;
	static const wchar_t* m_ClosestHitShaderName;
	static const wchar_t* m_ShadowHitGroupName;
	static const wchar_t* m_ShadowMissName;
	static const wchar_t* m_PlaneHitGroupName;

};
