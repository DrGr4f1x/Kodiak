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
class BaseIndexBufferData;
class BaseVertexBufferData;
class IndexBuffer;
class Scene;
class VertexBuffer;

enum class PrimitiveTopology;


class MeshPart
{
	friend class Mesh;
	friend class Scene;

public:
	MeshPart(
		std::shared_ptr<VertexBuffer> vbuffer,
		std::shared_ptr<IndexBuffer> ibuffer,
		PrimitiveTopology topology,
		uint32_t indexCount,
		uint32_t startIndex,
		int32_t baseVertex);

private:
	std::shared_ptr<VertexBuffer>	m_vertexBuffer;
	std::shared_ptr<IndexBuffer>	m_indexBuffer;
	PrimitiveTopology				m_topology;
	uint32_t						m_indexCount;
	uint32_t						m_startIndex;
	int32_t							m_baseVertexOffset;
	class Mesh*						m_parent{ nullptr };
	concurrency::task<void>			m_loadTask;
};


class Mesh
{
	friend class Model;
	friend class Scene;

public:
	void SetMeshParts(std::vector<MeshPart>& meshParts);
	
private:
	std::vector<MeshPart>		m_meshParts;
	class Model*				m_parent{ nullptr };
	concurrency::task<void>		m_loadTask;
};


class Model
{
	friend class Scene;
public:
	Model();

	void SetSingleMesh(Mesh& mesh);

	concurrency::task<void>	loadTask;

	void SetTransform(const DirectX::XMFLOAT4X4& matrix);
	const DirectX::XMFLOAT4X4& GetTransform() const;

	bool IsDirty() const { return m_isDirty; }
	void ResetDirty() { m_isDirty = false; }

private:
	std::vector<Mesh>	m_meshes;

	DirectX::XMFLOAT4X4	m_matrix;
	bool				m_isReady{ false };
	bool				m_isDirty{ true };
};


struct BoxModelDesc
{
	float sizeX{ 1.0f };
	float sizeY{ 1.0f };
	float sizeZ{ 1.0f };
	DirectX::XMFLOAT3 colors[8];
	bool genColors{ false };
	bool genNormals{ false };
	bool facesIn{ false };
};

std::shared_ptr<Model> MakeBoxModel(const BoxModelDesc& desc);



namespace RenderThread
{

struct StaticMeshPartData
{
	std::shared_ptr<VertexBuffer>	vertexBuffer;
	std::shared_ptr<IndexBuffer>	indexBuffer;
	PrimitiveTopology				topology;
	uint32_t						indexCount;
	uint32_t						startIndex;
	int32_t							baseVertexOffset;
};


struct StaticMeshData
{
	std::vector<StaticMeshPartData>		meshParts;
	DirectX::XMFLOAT4X4					matrix;
};


struct StaticModelData
{
	std::vector<StaticMeshData>		meshes;
	std::vector<Scene*>				scenes;
	DirectX::XMFLOAT4X4				matrix;

	concurrency::task<void>			prepareTask;
};


} // namespace RenderThread


struct StaticMeshPart
{
	// TODO: Support sharing vertex/index data between parts/meshes/models 
	// Render thread needs to know if a particular VB/IB has already been created, and just use the cached one.
	// Global VB and IB hash tables for caching should suffice.
	std::shared_ptr<BaseVertexBufferData>	vertexData;
	std::shared_ptr<BaseIndexBufferData>	indexData;

	PrimitiveTopology						topology;
	uint32_t								indexCount;
	uint32_t								startIndex;
	int32_t									baseVertexOffset;
};


class StaticMesh
{
	friend class Scene;
	friend class StaticModel;
public:
	StaticMesh();

	void AddMeshPart(StaticMeshPart part);
	size_t GetNumMeshParts() const { return m_meshParts.size(); }

private:
	std::vector<StaticMeshPart>	m_meshParts;
	DirectX::XMFLOAT4X4			m_matrix;
};


class StaticModel
{
	// TODO: A lot of friends here, refactor?
	friend class AddStaticModelAction;
	friend class RemoveStaticModelAction;
	friend class Renderer;
	friend class Scene;
public:
	StaticModel();

	void AddMesh(StaticMesh mesh);
	//void RemoveMesh(uint32_t index);
	//StaticMesh& GetMesh(uint32_t index);
	size_t GetNumMeshes() const { return m_meshes.size(); }

private:
	void CreateRenderThreadData();

private:
	std::mutex				m_meshMutex;
	std::vector<StaticMesh>	m_meshes;
	DirectX::XMFLOAT4X4		m_matrix;

	std::shared_ptr<RenderThread::StaticModelData>	m_renderThreadData;
};


struct BoxMeshDesc
{
	float sizeX{ 1.0f };
	float sizeY{ 1.0f };
	float sizeZ{ 1.0f };
	DirectX::XMFLOAT3 colors[8];
	bool genColors{ false };
	bool genNormals{ false };
	bool facesIn{ false };
};

StaticMesh MakeBoxMesh(const BoxMeshDesc& desc);


} // namespace Kodiak