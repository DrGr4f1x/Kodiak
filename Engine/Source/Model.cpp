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

#include "CommandList.h"
#include "ConstantBuffer.h"
#include "Defaults.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "Paths.h"
#include "Profile.h"
#include "Renderer.h"
#include "RenderEnums.h"
#include "VertexBuffer.h"
#include "VertexFormats.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void RenderThread::StaticModelData::UpdateConstants(GraphicsCommandList& commandList)
{
	for (auto& mesh : meshes)
	{
#if defined(DX11)
		if (isDirty || mesh->isDirty)
#endif
		{
			StaticMeshPerObjectData perObjectData;
			
			perObjectData.matrix = matrix * mesh->matrix;

			auto dest = commandList.MapConstants(*mesh->perObjectConstants);
			memcpy(dest, &perObjectData, sizeof(perObjectData));
			commandList.UnmapConstants(*mesh->perObjectConstants);

			mesh->isDirty = false;
		}
	}

	isDirty = false;
}


StaticMesh::StaticMesh()
	: m_matrix(kIdentity)
{
	CreateRenderThreadData();
}


void StaticMesh::AddMeshPart(StaticMeshPart part)
{
	m_meshParts.emplace_back(part);

	auto thisMesh = shared_from_this();
	
	EnqueueRenderCommand([thisMesh, part]()
	{
		RenderThread::StaticMeshPartData data = { part.vertexBuffer, part.indexBuffer, part.material->GetRenderThreadData(), part.topology, part.indexCount, part.startIndex, part.baseVertexOffset };
		thisMesh->m_renderThreadData->meshParts.emplace_back(data);
	});
}


void StaticMesh::SetMatrix(const Matrix4& matrix)
{
	m_matrix = matrix;

	DirectX::XMFLOAT4X4 matrixNonAligned;
	DirectX::XMStoreFloat4x4(&matrixNonAligned, m_matrix);

	auto staticMeshData = m_renderThreadData;
	EnqueueRenderCommand([staticMeshData, matrixNonAligned]()
	{
		staticMeshData->matrix = Matrix4(DirectX::XMLoadFloat4x4(&matrixNonAligned));
		staticMeshData->isDirty = true;
	});
}


void StaticMesh::ConcatenateMatrix(const Matrix4& matrix)
{
	m_matrix = m_matrix * matrix;

	DirectX::XMFLOAT4X4 matrixNonAligned;
	DirectX::XMStoreFloat4x4(&matrixNonAligned, m_matrix);

	auto staticMeshData = m_renderThreadData;
	EnqueueRenderCommand([staticMeshData, matrixNonAligned]()
	{
		staticMeshData->matrix = Matrix4(DirectX::XMLoadFloat4x4(&matrixNonAligned));
		staticMeshData->isDirty = true;
	});
}


shared_ptr<StaticMesh> StaticMesh::Clone()
{
	auto clone = make_shared<StaticMesh>();
	clone->SetMatrix(m_matrix);

	for (const auto& part : m_meshParts)
	{
		StaticMeshPart partCopy = part;
		partCopy.material = part.material->Clone();
		clone->AddMeshPart(partCopy);
	}

	return clone;
}


void StaticMesh::CreateRenderThreadData()
{
	m_renderThreadData = make_shared<RenderThread::StaticMeshData>();
	m_renderThreadData->matrix = m_matrix;
	
	m_renderThreadData->perObjectConstants = make_shared<ConstantBuffer>();
	m_renderThreadData->perObjectConstants->Create(sizeof(RenderThread::StaticMeshPerObjectData), Usage::Dynamic);
}


StaticModel::StaticModel()
	: m_matrix(kIdentity)
{
	CreateRenderThreadData();
}


void StaticModel::AddMesh(shared_ptr<StaticMesh> mesh)
{
	m_meshes.emplace_back(mesh);

	auto staticModelData = m_renderThreadData;
	auto staticMeshData = mesh->m_renderThreadData;

	EnqueueRenderCommand([staticModelData, staticMeshData]()
	{
		staticModelData->meshes.emplace_back(staticMeshData);
	});
}


shared_ptr<StaticMesh> StaticModel::GetMesh(uint32_t index)
{
	assert(index < m_meshes.size());

	return m_meshes[index];
}


void StaticModel::SetMatrix(const Matrix4& matrix)
{
	m_matrix = matrix;
	auto thisModel = shared_from_this();
	EnqueueRenderCommand([thisModel]()
	{
		thisModel->m_renderThreadData->matrix = thisModel->GetMatrix();
		thisModel->m_renderThreadData->isDirty = true;
	});
}


void StaticModel::CreateRenderThreadData()
{
	const auto numMeshes = m_meshes.size();

	m_renderThreadData = make_shared<RenderThread::StaticModelData>();
	m_renderThreadData->matrix = m_matrix;
	m_renderThreadData->meshes.reserve(numMeshes);
}


namespace Kodiak
{

shared_ptr<StaticMesh> MakeBoxMesh(const BoxMeshDesc& desc)
{
	shared_ptr<VertexBuffer> vbuffer;

	auto material = make_shared<Material>();
	material->SetEffect(GetDefaultBaseEffect());
	material->SetRenderPass(GetDefaultBasePass());

	if (desc.genNormals && desc.genColors)
	{
		VertexBufferData<VertexPositionNormalColor> vdata
		{
			// -X face
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(-1.0f, 0.0f, 0.0f), Vector3(desc.colors[1]) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(-1.0f, 0.0f, 0.0f), Vector3(desc.colors[0]) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(-1.0f, 0.0f, 0.0f), Vector3(desc.colors[7]) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(-1.0f, 0.0f, 0.0f), Vector3(desc.colors[6]) },
			// +X face
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(1.0f, 0.0f, 0.0f), Vector3(desc.colors[5]) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(1.0f, 0.0f, 0.0f), Vector3(desc.colors[4]) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(1.0f, 0.0f, 0.0f), Vector3(desc.colors[3]) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(1.0f, 0.0f, 0.0f), Vector3(desc.colors[2]) },
			// -Y face
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, -1.0f, 0.0f), Vector3(desc.colors[1]) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, -1.0f, 0.0f), Vector3(desc.colors[7]) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, -1.0f, 0.0f), Vector3(desc.colors[3]) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, -1.0f, 0.0f), Vector3(desc.colors[5]) },
			// +Y face
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 1.0f, 0.0f), Vector3(desc.colors[2]) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 1.0f, 0.0f), Vector3(desc.colors[4]) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 1.0f, 0.0f), Vector3(desc.colors[0]) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 1.0f, 0.0f), Vector3(desc.colors[6]) },
			// -Z face
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, -1.0f), Vector3(desc.colors[3]) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, -1.0f), Vector3(desc.colors[2]) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, -1.0f), Vector3(desc.colors[1]) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, -1.0f), Vector3(desc.colors[0]) },
			// +Z face
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, 1.0f), Vector3(desc.colors[7]) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, 1.0f), Vector3(desc.colors[6]) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, 1.0f), Vector3(desc.colors[5]) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, 1.0f), Vector3(desc.colors[4]) }
		};

		vbuffer = VertexBuffer::Create(vdata, Usage::Immutable);
	}
	else if (desc.genNormals)
	{
		VertexBufferData<VertexPositionNormal> vdata
		{
			// -X face
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(-1.0f, 0.0f, 0.0f) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(-1.0f, 0.0f, 0.0f) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(-1.0f, 0.0f, 0.0f) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(-1.0f, 0.0f, 0.0f) },
			// +X face
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(1.0f, 0.0f, 0.0f) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(1.0f, 0.0f, 0.0f) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(1.0f, 0.0f, 0.0f) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(1.0f, 0.0f, 0.0f) },
			// -Y face
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, -1.0f, 0.0f) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, -1.0f, 0.0f) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, -1.0f, 0.0f) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, -1.0f, 0.0f) },
			// +Y face
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 1.0f, 0.0f) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 1.0f, 0.0f) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 1.0f, 0.0f) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 1.0f, 0.0f) },
			// -Z face
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, -1.0f) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, -1.0f) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, -1.0f) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, -1.0f) },
			// +Z face
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, 1.0f) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, 1.0f) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, 1.0f) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(0.0f, 0.0f, 1.0f) }
		};

		vbuffer = VertexBuffer::Create(vdata, Usage::Immutable);
	}
	else if (desc.genColors)
	{
		VertexBufferData<VertexPositionColor> vdata
		{
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(desc.colors[0]) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(desc.colors[1]) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(desc.colors[2]) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ), Vector3(desc.colors[3]) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(desc.colors[4]) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(desc.colors[5]) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(desc.colors[6]) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ), Vector3(desc.colors[7]) }
		};

		vbuffer = VertexBuffer::Create(vdata, Usage::Immutable);
	}
	else
	{
		VertexBufferData<VertexPosition> vdata
		{
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY, -0.5f * desc.sizeZ) },
			{ Vector3(0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ Vector3(0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ Vector3(-0.5f * desc.sizeX,  0.5f * desc.sizeY,  0.5f * desc.sizeZ) },
			{ Vector3(-0.5f * desc.sizeX, -0.5f * desc.sizeY,  0.5f * desc.sizeZ) }
		};

		vbuffer = VertexBuffer::Create(vdata, Usage::Immutable);
	}

	// Create the mesh and mesh parts
	auto mesh = make_shared<StaticMesh>();

	if (desc.genNormals)
	{
		// -X face
		{
			IndexBufferData16 idata{ { 0, 1, 2, 3 } };

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			StaticMeshPart meshPart{ vbuffer, ibuffer, material, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// +X face
		{
			IndexBufferData16 idata{ { 4, 5, 6, 7 } };

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			StaticMeshPart meshPart{ vbuffer, ibuffer, material, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// -Y face
		{
			IndexBufferData16 idata{ { 8, 9, 10, 11 } };

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			StaticMeshPart meshPart{ vbuffer, ibuffer, material, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// +Y face
		{
			IndexBufferData16 idata{ { 12, 13, 14, 15 } };

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			StaticMeshPart meshPart{ vbuffer, ibuffer, material, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// -Z face
		{
			IndexBufferData16 idata{ { 16, 17, 18, 19 } };

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			StaticMeshPart meshPart{ vbuffer, ibuffer, material, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// +Z face
		{
			IndexBufferData16 idata{ { 20, 21, 22, 23 } };

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			StaticMeshPart meshPart{ vbuffer, ibuffer, material, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}
	}
	else
	{
		// Body
		{
			IndexBufferData16 idata{ { 0, 1, 2, 3, 4, 5, 6, 7, 0, 1 } };

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			StaticMeshPart meshPart{ vbuffer, ibuffer, material, PrimitiveTopology::TriangleStrip, 10, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// Top
		{
			IndexBufferData16 idata{ { 6, 0, 4, 2 } };

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			StaticMeshPart meshPart{ vbuffer, ibuffer, material, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}

		// Bottom
		{
			IndexBufferData16 idata{ { 5, 3, 7, 1 } };

			auto ibuffer = IndexBuffer::Create(idata, Usage::Immutable);

			StaticMeshPart meshPart{ vbuffer, ibuffer, material, PrimitiveTopology::TriangleStrip, 4, 0, 0 };
			mesh->AddMeshPart(meshPart);
		}
	}

	return mesh;
}


namespace
{

enum class ModelFormat : uint32_t
{
	None,
	H3D,

	NumFormats
};

const string s_formatString[] =
{
	"none",
	"h3d",
};

} // Anonymous namespace


std::shared_ptr<StaticModel> LoadModel(const string& path)
{
	ModelFormat format = ModelFormat::None;

	auto sepIndex = path.rfind('.');
	if (sepIndex != string::npos)
	{
		string extension = path.substr(sepIndex + 1);
		transform(begin(extension), end(extension), begin(extension), ::tolower);

		for (uint32_t i = 0; i < static_cast<uint32_t>(ModelFormat::NumFormats); ++i)
		{
			if (extension == s_formatString[i])
			{
				format = static_cast<ModelFormat>(i);
				break;
			}
		}

		if (format != ModelFormat::None)
		{
			string fullPath = Paths::GetInstance().ModelDir() + path;

			switch (format)
			{
			case ModelFormat::H3D:
				return LoadModelH3D(fullPath);
				break;
			}
		}
	}

	return nullptr;
}


} // namespace Kodiak