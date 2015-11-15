// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Model.h"


#include "IndexBuffer.h"
#include "Renderer.h"
#include "RenderEnums.h"
#include "VertexBuffer.h"
#include "VertexFormats.h"


using namespace Kodiak;
using namespace DirectX;
using namespace std;


MeshPart::MeshPart(std::shared_ptr<VertexBuffer> vbuffer,
	std::shared_ptr<IndexBuffer> ibuffer,
	PrimitiveTopology topology,
	uint32_t indexCount,
	uint32_t startIndex,
	int32_t baseVertex)
	: m_vertexBuffer(vbuffer)
	, m_indexBuffer(ibuffer)
	, m_topology(topology)
	, m_indexCount(indexCount)
	, m_startIndex(startIndex)
	, m_baseVertexOffset(baseVertex)
{
	m_loadTask = (vbuffer->loadTask && ibuffer->loadTask);
}


void Mesh::SetMeshParts(vector<MeshPart>& meshParts)
{
	m_meshParts = meshParts;

	if (!meshParts.empty())
	{
		m_loadTask = meshParts[0].m_loadTask;
	}

	for (size_t i = 0; i < m_meshParts.size(); ++i)
	{
		m_meshParts[i].m_parent = this;
		if (i > 1)
		{
			m_loadTask = (m_loadTask && m_meshParts[i].m_loadTask);
		}
	}

	if (m_parent)
	{
		m_parent->loadTask = (m_parent->loadTask && m_loadTask);
	}
}


void Model::SetSingleMesh(Mesh& mesh)
{
	m_meshes.clear();
	m_meshes.push_back(mesh);

	mesh.m_parent = this;
	loadTask = mesh.m_loadTask;
}


namespace Kodiak
{

shared_ptr<Model> MakeBoxModel(Renderer* renderer, const BoxModelDesc& desc)
{
	auto model = make_shared<Model>();

	auto vbuffer = make_shared<VertexBuffer>();
	shared_ptr<BaseVertexBufferData> vdata;

	if (desc.genNormals && desc.genColors)
	{
		vdata.reset(new VertexBufferData<VertexPositionNormalColor>(
		{
			// -X face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[0]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[7]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[6]) },
			// +X face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[5]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[4]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f), XMFLOAT3(desc.colors[2]) },
			// -Y face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[7]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f), XMFLOAT3(desc.colors[5]) },
			// +Y face
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[2]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[4]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[0]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f), XMFLOAT3(desc.colors[6]) },
			// -Z face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[2]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(desc.colors[0]) },
			// +Z face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[7]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[6]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[5]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(desc.colors[4]) }

		}
		));

		vbuffer->Create(vdata, Usage::Immutable, "Box vertex buffer with normals and colors");
	}
	else if (desc.genNormals)
	{
		vdata.reset(new VertexBufferData<VertexPositionNormal>(
		{
			// -X face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(-1.0f, 0.0f, 0.0f) },
			// +X face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(1.0f, 0.0f, 0.0f) },
			// -Y face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, -1.0f, 0.0f) },
			// +Y face
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 1.0f, 0.0f) },
			// -Z face
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, -1.0f) },
			// +Z face
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(0.0f, 0.0f, 1.0f) }

		}
		));

		vbuffer->Create(vdata, Usage::Immutable, "Box vertex buffer with normals");
	}
	else if (desc.genColors)
	{
		vdata.reset(new VertexBufferData<VertexPositionColor>(
		{
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[0]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[1]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[2]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), XMFLOAT3(desc.colors[3]) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[4]) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[5]) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[6]) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), XMFLOAT3(desc.colors[7]) }
		}
		));

		vbuffer->Create(vdata, Usage::Immutable, "Box vertex buffer with colors");
	}
	else
	{
		vdata.reset(new VertexBufferData<VertexPosition>(
		{
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ XMFLOAT3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ XMFLOAT3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ XMFLOAT3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ) }
		}
		));

		vbuffer->Create(vdata, Usage::Immutable, "Box vertex buffer");
	}

	// Create the mesh and mesh parts
	Mesh mesh;
	vector<MeshPart> meshParts;
	

	if (desc.genNormals)
	{
		meshParts.reserve(6);

		// -X face
		{
			auto ibuffer = make_shared<IndexBuffer>();
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 0, 1, 2, 3 }));

			ibuffer->Create(idata, Usage::Immutable, "-X face");

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// +X face
		{
			auto ibuffer = make_shared<IndexBuffer>();
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 4, 5, 6, 7 }));

			ibuffer->Create(idata, Usage::Immutable, "+X face");

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// -Y face
		{
			auto ibuffer = make_shared<IndexBuffer>();
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 8, 9, 10, 11 }));

			ibuffer->Create(idata, Usage::Immutable, "-Y face");

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// +Y face
		{
			auto ibuffer = make_shared<IndexBuffer>();
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 12, 13, 14, 15 }));

			ibuffer->Create(idata, Usage::Immutable, "+Y face");

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// -Z face
		{
			auto ibuffer = make_shared<IndexBuffer>();
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 16, 17, 18, 19 }));

			ibuffer->Create(idata, Usage::Immutable, "-Z face");

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// +Z face
		{
			auto ibuffer = make_shared<IndexBuffer>();
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 20, 21, 22, 23 }));

			ibuffer->Create(idata, Usage::Immutable, "+Z face");

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}
	}
	else
	{
		meshParts.reserve(3);

		// Body
		{
			auto ibuffer = make_shared<IndexBuffer>();
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 0, 1, 2, 3, 4, 5, 6, 7, 0, 1 }));

			ibuffer->Create(idata, Usage::Immutable, "Body");

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 10, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// Top
		{
			auto ibuffer = make_shared<IndexBuffer>();
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 6, 0, 4, 2 }));

			ibuffer->Create(idata, Usage::Immutable, "Top");

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}

		// Bottom
		{
			auto ibuffer = make_shared<IndexBuffer>();
			shared_ptr<BaseIndexBufferData> idata;
			idata.reset(new IndexBufferData16({ 5, 3, 7, 1 }));

			ibuffer->Create(idata, Usage::Immutable, "Bottom");

			auto meshPart = MeshPart(vbuffer, ibuffer, PrimitiveTopology::TriangleStrip, 4, 0, 0);
			meshParts.emplace_back(meshPart);
		}
	}

	mesh.SetMeshParts(meshParts);
	model->SetSingleMesh(mesh);

	return model;
}

} // namespace Kodiak