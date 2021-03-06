// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include <ppltasks.h>
#include <unordered_set>

namespace Kodiak
{

// Forward declarations
class ConstantBuffer;
class GraphicsCommandList;
class IndexBuffer;
class Material;
class Scene;
class VertexBuffer;

enum class PrimitiveTopology;


namespace RenderThread
{
struct MaterialData;

struct StaticMeshPartData
{
	std::shared_ptr<VertexBuffer>	vertexBuffer;
	std::shared_ptr<IndexBuffer>	indexBuffer;
	std::shared_ptr<MaterialData>	material;
	PrimitiveTopology				topology;
	uint32_t						indexCount;
	uint32_t						startIndex;
	int32_t							baseVertexOffset;
};


struct StaticMeshData
{
	std::vector<StaticMeshPartData>		meshParts;
	Math::Matrix4						matrix;

	std::shared_ptr<ConstantBuffer>		perObjectConstants;
	bool								isDirty{ true };
};


struct StaticMeshPerObjectData
{
	Math::Matrix4 matrix;
};


struct StaticModelData
{
	std::vector<std::shared_ptr<StaticMeshData>>	meshes;
	Math::Matrix4									matrix;

	bool											isDirty{ true };

	void UpdateConstants(GraphicsCommandList& commandList);
};


} // namespace RenderThread


struct StaticMeshPart
{
	friend class StaticMesh;

	std::shared_ptr<VertexBuffer>	vertexBuffer;
	std::shared_ptr<IndexBuffer>	indexBuffer;
	std::shared_ptr<Material>		material;

	PrimitiveTopology				topology;
	uint32_t						indexCount;
	uint32_t						startIndex;
	int32_t							baseVertexOffset;
};


class StaticMesh : public std::enable_shared_from_this<StaticMesh>
{
	friend class Scene;
	friend class StaticModel;
public:
	StaticMesh();

	void AddMeshPart(StaticMeshPart part);
	size_t GetNumMeshParts() const { return m_meshParts.size(); }

	void SetMatrix(const Math::Matrix4& matrix);
	void ConcatenateMatrix(const Math::Matrix4& matrix);
	const Math::Matrix4& GetMatrix() const { return m_matrix; }

	std::shared_ptr<Material> GetMaterial(uint32_t meshPartIndex)
	{
		return m_meshParts[meshPartIndex].material;
	}

	std::shared_ptr<StaticMesh> Clone();

private:
	void CreateRenderThreadData();

private:
	std::vector<StaticMeshPart>	m_meshParts;
	Math::Matrix4				m_matrix;

	std::shared_ptr<RenderThread::StaticMeshData>	m_renderThreadData;
};


class StaticModel : public std::enable_shared_from_this<StaticModel>
{
	friend class Scene;

public:
	StaticModel();

	void AddMesh(std::shared_ptr<StaticMesh> mesh);
	std::shared_ptr<StaticMesh> GetMesh(uint32_t index);
	size_t GetNumMeshes() const { return m_meshes.size(); }

	void SetMatrix(const Math::Matrix4& matrix);
	const Math::Matrix4& GetMatrix() const { return m_matrix; }

private:
	void CreateRenderThreadData();

private:
	std::mutex									m_meshMutex;
	std::vector<std::shared_ptr<StaticMesh>>	m_meshes;
	Math::Matrix4								m_matrix;

	std::shared_ptr<RenderThread::StaticModelData>	m_renderThreadData;
};


// Factory methods
struct BoxMeshDesc
{
	float sizeX{ 1.0f };
	float sizeY{ 1.0f };
	float sizeZ{ 1.0f };
	Math::Vector3 colors[8];
	bool genColors{ false };
	bool genNormals{ false };
	bool facesIn{ false };
};


std::shared_ptr<StaticMesh> MakeBoxMesh(const BoxMeshDesc& desc);

// Loaders
std::shared_ptr<StaticModel> LoadModel(const std::string& path);
std::shared_ptr<StaticModel> LoadModelH3D(const std::string& fullPath);

} // namespace Kodiak