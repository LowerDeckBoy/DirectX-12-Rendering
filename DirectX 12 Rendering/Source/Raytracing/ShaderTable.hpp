#pragma once
#include <d3d12.h>
#include <vector>
#include <string>
#include <wrl/client.h>

/// Notes:
/// Shader Record stride must be at least size of the largest record
/// inside given table

class TableRecord
{
public:
	TableRecord(void* pIdentifier, uint32_t Size) noexcept;
	TableRecord(void* pIdentifier, uint32_t Size, void* pLocalRootArgs, uint32_t ArgsSize) noexcept;

	void CopyTo(void* pDestination) noexcept;
	void SetAlignment(uint32_t Alignment);
	
	struct Identifier
	{
		void* pData{ nullptr };
		uint32_t Size{ 0 };
	};

	Identifier m_Identifier;
	Identifier m_LocalRootArgs;

	// For using more then one record in a ShaderTable
	// requires aligment for largest record size
	uint32_t TotalSize{ 0 };

};

class ShaderTable
{
public:
	ShaderTable() { }
	~ShaderTable();

	void Create(ID3D12Device5* pDevice, uint32_t NumShaderRecord, uint32_t ShaderRecordSize, const std::wstring& DebugName = L"");
	
	void AddRecord(TableRecord& Record);
	
	void SetStride(uint32_t Stride);
	void CheckAlignment();

	void SetTableName(std::wstring Name);

	void Release();

	inline uint32_t GetRecordsCount() const noexcept { return static_cast<uint32_t>(m_Records.size()); }
	inline uint32_t Stride() { return m_Stride; }

	inline uint32_t GetShaderRecordSize() const noexcept { return m_ShaderRecordSize; }
	inline ID3D12Resource* GetStorage() const noexcept { return m_Storage.Get(); }
	inline const D3D12_GPU_VIRTUAL_ADDRESS GetAddressOf() const { return m_Storage.Get()->GetGPUVirtualAddress(); }

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> m_Storage;
	uint8_t* m_MappedData{ nullptr };
	uint32_t m_ShaderRecordSize{ 0 };
	uint32_t m_Stride{ 0 };

	std::vector<TableRecord> m_Records;

};

// For single buffer usage
/*
class ShaderTableBuilder
{
public:
	ShaderTableBuilder();

	void Create(ID3D12Device5* pDevice, ID3D12StateObjectProperties* pRaytracingPipeline, ID3D12Resource* pTableStorageBuffer);
	void Reset();

	void AddRayGenShader(std::wstring Entrypoint, std::vector<void*> pInputData);
	void AddMissShader(std::wstring Entrypoint, std::vector<void*> pInputData);
	void AddHitGroup(std::wstring GroupName, std::vector<void*> pInputData);

	// Get total size based on shaders
	uint32_t GetTableSize();

	uint32_t GetRayGenSize();
	uint32_t GetMissSize();
	uint32_t GetHitGroup();

private:

	struct RaytraceShader
	{
		RaytraceShader(std::wstring Entrypoint, std::vector<void*> pInputData) noexcept
			: Name(std::move(Entrypoint)), InputData(std::move(pInputData))
		{ }

		const std::wstring Name;
		std::vector<void*> InputData;
	};

	RaytraceShader* m_RayGen{ nullptr };
	uint32_t m_RayGenSize{};
	RaytraceShader* m_Miss{ nullptr };
	uint32_t m_MissSize{};
	RaytraceShader* m_ClosestHit{ nullptr };
	uint32_t m_HitGroupSize{};
	RaytraceShader* m_HitGroup{ nullptr };

	uint32_t GetShaderSize(RaytraceShader& Shader);
	uint32_t CopyShaderData(ID3D12StateObjectProperties* pRaytracingPipeline, uint8_t* pOutputData,	RaytraceShader& Shader, const uint32_t ShaderSize);

	uint32_t m_IdentifierSize{ D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES };

};
*/
