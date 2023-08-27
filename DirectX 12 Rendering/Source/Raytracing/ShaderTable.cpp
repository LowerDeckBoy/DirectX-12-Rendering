#include "ShaderTable.hpp"
#include "../Utilities/Utilities.hpp"
#include "../Graphics/Buffer/BufferUtils.hpp"
#include <d3dx12.h>

// Note: Starting address of Shader Table must be aligned to 64 bytes
// D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT == 64 bytes
inline constexpr uint32_t ALIGN(uint32_t Size, uint32_t Alignment)
{
	return (Size + (Alignment + 1)) & ~(Alignment - 1);
}

TableRecord::TableRecord(void* pIdentifier, uint32_t Size) noexcept
{
	m_Identifier.pData = pIdentifier;
	m_Identifier.Size = Size;

	TotalSize += Size;
}

TableRecord::TableRecord(void* pIdentifier, uint32_t Size, void* pLocalRootArgs, uint32_t ArgsSize) noexcept
{
	m_Identifier.pData = pIdentifier;
	m_Identifier.Size = Size;
	m_LocalRootArgs.pData = pLocalRootArgs;
	m_LocalRootArgs.Size = ArgsSize;

	TotalSize += Size + ArgsSize;
}

void TableRecord::CopyTo(void* pDestination) noexcept
{
	uint8_t* pByteDestination{ static_cast<uint8_t*>(pDestination) };
	std::memcpy(pByteDestination, m_Identifier.pData, m_Identifier.Size);
	if (m_LocalRootArgs.pData != nullptr)
		std::memcpy(pByteDestination + m_Identifier.Size, m_LocalRootArgs.pData, m_LocalRootArgs.Size);
}

ShaderTable::~ShaderTable()
{
	Release();
}

void ShaderTable::Create(ID3D12Device5* pDevice, uint32_t NumShaderRecord, uint32_t ShaderRecordSize, const std::wstring& DebugName)
{
	m_ShaderRecordSize = ALIGN(ShaderRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
	m_Records.reserve(NumShaderRecord);

	const uint32_t bufferSize{ NumShaderRecord * m_ShaderRecordSize };
	const auto uploadHeap{ CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD) };
	BufferUtils::Create(pDevice, &m_Storage, bufferSize, D3D12_RESOURCE_FLAG_NONE, D3D12_RESOURCE_STATE_GENERIC_READ, uploadHeap);
	m_MappedData = BufferUtils::MapCPU(m_Storage.Get());

	if (!DebugName.empty())
		SetTableName(DebugName);
}

void ShaderTable::AddRecord(TableRecord& Record)
{
	m_Records.push_back(Record);
	Record.CopyTo(m_MappedData);
	//m_MappedData += ALIGN(m_ShaderRecordSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
	m_MappedData += m_ShaderRecordSize;

}

void ShaderTable::SetStride(uint32_t Stride)
{
	m_Stride = Stride;
}

void ShaderTable::CheckAlignment()
{
	uint32_t max = std::max({ m_Records.data()->TotalSize });

	for (auto& record : m_Records)
		record.TotalSize = ALIGN(max, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

	m_Stride = ALIGN(max, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

	m_Storage->Unmap(0, nullptr);
}

void ShaderTable::SetTableName(std::wstring Name)
{
	m_Storage.Get()->SetName(Name.c_str());
}

void ShaderTable::Release()
{
	SAFE_RELEASE(m_Storage);

	m_Records.clear();

	if (m_MappedData)
	{
		m_MappedData = nullptr;
		delete m_MappedData;
	}
}

/*
ShaderTableBuilder::ShaderTableBuilder()
{
}

void ShaderTableBuilder::Create(ID3D12Device5* pDevice, ID3D12StateObjectProperties* pRaytracingPipeline, ID3D12Resource* pTableStorageBuffer)
{
	uint8_t* pData{ nullptr };
	ThrowIfFailed(pTableStorageBuffer->Map(0, nullptr, reinterpret_cast<void**>(&pData)));

	uint32_t offset{ 0 };

	m_RayGenSize = GetShaderSize(*m_RayGen);
	offset = CopyShaderData(pRaytracingPipeline, pData, *m_RayGen, m_RayGenSize);
	pData += offset;

	m_MissSize = GetShaderSize(*m_Miss);
	offset = CopyShaderData(pRaytracingPipeline, pData, *m_Miss, m_MissSize);
	pData += offset;

	m_HitGroupSize = GetShaderSize(*m_HitGroup);
	offset = CopyShaderData(pRaytracingPipeline, pData, *m_HitGroup, m_HitGroupSize);

	pTableStorageBuffer->Unmap(0, nullptr);
}

void ShaderTableBuilder::Reset()
{
	m_RayGen	 = nullptr;
	m_Miss		 = nullptr;
	m_ClosestHit = nullptr;


}

void ShaderTableBuilder::AddRayGenShader(std::wstring Entrypoint, std::vector<void*> pInputData)
{
	m_RayGen = new RaytraceShader(Entrypoint, pInputData);
}

void ShaderTableBuilder::AddMissShader(std::wstring Entrypoint, std::vector<void*> pInputData)
{
	m_Miss = new RaytraceShader(Entrypoint, pInputData);
}

void ShaderTableBuilder::AddHitGroup(std::wstring GroupName, std::vector<void*> pInputData)
{
	m_HitGroup = new RaytraceShader(GroupName, pInputData);
}

uint32_t ShaderTableBuilder::GetTableSize()
{
	m_IdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;

	// Currently only one entry per shader so no need to multiply by vector sizes
	m_RayGenSize	= GetShaderSize(*m_RayGen);
	m_MissSize		= GetShaderSize(*m_Miss);
	m_HitGroupSize	= GetShaderSize(*m_HitGroup);

	// 256 bytes alignment
	return ALIGN(m_RayGenSize + m_MissSize + m_HitGroupSize, 256);
}

uint32_t ShaderTableBuilder::GetRayGenSize()
{
	return m_RayGenSize;
}

uint32_t ShaderTableBuilder::GetMissSize()
{
	return m_MissSize;
}

uint32_t ShaderTableBuilder::GetHitGroup()
{
	return m_HitGroupSize;
}

uint32_t ShaderTableBuilder::GetShaderSize(RaytraceShader& Shader)
{
	size_t sizeOfArgs{ 0 };
	sizeOfArgs = std::max(sizeOfArgs, Shader.InputData.size());

	uint32_t shaderSize{ m_IdentifierSize + 8 * static_cast<uint32_t>(sizeOfArgs) };

	return ALIGN(shaderSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);
}

uint32_t ShaderTableBuilder::CopyShaderData(ID3D12StateObjectProperties* pRaytracingPipeline, uint8_t* pOutputData, RaytraceShader& Shader, const uint32_t ShaderSize)
{
	uint8_t* pData = pOutputData;

	void* id = pRaytracingPipeline->GetShaderIdentifier(Shader.Name.c_str());
	if (!id)
	{
		::OutputDebugStringA("Failed to get ID\n");
		throw std::logic_error("");
	}
	// Copy the shader identifier
	std::memcpy(pData, id, m_IdentifierSize);
	// Copy all its resources pointers or values in bulk
	std::memcpy(pData + m_IdentifierSize, Shader.InputData.data(), Shader.InputData.size() * 8);

	pData += ShaderSize;

	return ShaderSize;
}

*/
