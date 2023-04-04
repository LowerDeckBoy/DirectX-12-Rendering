#pragma once
#include "Material.hpp"

//struct Material;

struct Primitive
{
	Primitive() { }
	
	Primitive(uint32_t FirstIndexLocation, uint32_t BaseVertexLocation,
			  uint32_t IndexCount, Material* Material) :
		FirstIndexLocation(FirstIndexLocation), BaseVertexLocation(BaseVertexLocation),
		IndexCount(IndexCount),
		Material(Material),
		bHasIndices(IndexCount > 0)
	{}
	
	Primitive(const Primitive& rhs)
	{
		FirstIndexLocation = rhs.FirstIndexLocation;
		BaseVertexLocation = rhs.BaseVertexLocation;
		IndexCount = rhs.IndexCount;
		VertexCount = rhs.VertexCount;
		Material = rhs.Material;
		bHasIndices = rhs.bHasIndices;
		::OutputDebugStringA("Primitive copied.\n");
	}

	Primitive& operator=(const Primitive& rhs)
	{
		//Primitive* newPrimitive{ new Primitive() };
		//newPrimitive->FirstIndexLocation = rhs.FirstIndexLocation;
		//newPrimitive->BaseVertexLocation = rhs.BaseVertexLocation;
		//newPrimitive->IndexCount = rhs.IndexCount;
		//newPrimitive->VertexCount = rhs.VertexCount;
		//newPrimitive->Material = rhs.Material;
		//newPrimitive->bHasIndices = rhs.bHasIndices;
		//return *newPrimitive;
		::OutputDebugStringA("Primitive copied.\n");
		return *this;
	}
	~Primitive()
	{
		//delete Material;
	}

	uint32_t FirstIndexLocation{};
	uint32_t BaseVertexLocation{};

	uint32_t IndexCount{};
	uint32_t VertexCount{};
	uint32_t StartVertexLocation{};

	Material* Material;
	bool bHasIndices{ false };
};