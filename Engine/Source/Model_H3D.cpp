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

#include "BinaryReader.h"
#include "Defaults.h"
#include "Filesystem.h"
#include "IndexBuffer.h"
#include "Material.h"
#include "MaterialParameter.h"
#include "MaterialResource.h"
#include "RenderEnums.h"
#include "Texture.h"
#include "VertexBuffer.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


namespace H3D
{

struct BoundingBox
{
	DirectX::XMVECTOR min;
	DirectX::XMVECTOR max;
};

void ReadBoundingBox(BoundingBox& boundingBox, BinaryReader& reader)
{
	boundingBox.min = reader.Read<DirectX::XMVECTOR>();
	boundingBox.max = reader.Read<DirectX::XMVECTOR>();
}


struct Header
{
	uint32_t meshCount;
	uint32_t materialCount;
	uint32_t vertexDataByteSize;
	uint32_t indexDataByteSize;
	uint32_t vertexDataByteSizeDepth;
	
	BoundingBox boundingBox;
};

void ReadHeader(Header& header, BinaryReader& reader)
{
	header.meshCount = reader.Read<uint32_t>();
	header.materialCount = reader.Read<uint32_t>();
	header.vertexDataByteSize = reader.Read<uint32_t>();
	header.indexDataByteSize = reader.Read<uint32_t>();
	header.vertexDataByteSizeDepth = reader.Read<uint32_t>();
	reader.ReadArray<uint32_t>(3); // Padding
	ReadBoundingBox(header.boundingBox, reader);
}


enum
{
	attrib_0 = 0,
	attrib_1 = 1,
	attrib_2 = 2,
	attrib_3 = 3,
	attrib_4 = 4,
	attrib_5 = 5,
	attrib_6 = 6,
	attrib_7 = 7,
	attrib_8 = 8,
	attrib_9 = 9,
	attrib_10 = 10,
	attrib_11 = 11,
	attrib_12 = 12,
	attrib_13 = 13,
	attrib_14 = 14,
	attrib_15 = 15,

	// friendly name aliases
	attrib_position = attrib_0,
	attrib_texcoord0 = attrib_1,
	attrib_normal = attrib_2,
	attrib_tangent = attrib_3,
	attrib_bitangent = attrib_4,

	maxAttribs = 16
};

enum
{
	attrib_format_none = 0,
	attrib_format_ubyte,
	attrib_format_byte,
	attrib_format_ushort,
	attrib_format_short,
	attrib_format_float,

	attrib_formats
};


struct Attrib
{
	uint16_t offset;
	uint16_t normalized;
	uint16_t components;
	uint16_t format;
};

void ReadAttrib(Attrib& attrib, BinaryReader& reader)
{
	attrib.offset = reader.Read<uint16_t>();
	attrib.normalized = reader.Read<uint16_t>();
	attrib.components = reader.Read<uint16_t>();
	attrib.format = reader.Read<uint16_t>();
}


struct Mesh
{
	BoundingBox boundingBox;

	uint32_t materialIndex;
	uint32_t attribsEnabled;
	uint32_t attribsEnabledDepth;
	uint32_t vertexStride;
	uint32_t vertexStrideDepth;

	Attrib attrib[maxAttribs];
	Attrib attribDepth[maxAttribs];

	uint32_t vertexDataByteOffset;
	uint32_t vertexCount;
	uint32_t indexDataByteOffset;
	uint32_t indexCount;

	uint32_t vertexDataByteOffsetDepth;
	uint32_t vertexCountDepth;
};

void ReadMesh(Mesh& mesh, BinaryReader& reader)
{
	ReadBoundingBox(mesh.boundingBox, reader);
	
	mesh.materialIndex = reader.Read<uint32_t>();
	mesh.attribsEnabled = reader.Read<uint32_t>();
	mesh.attribsEnabledDepth = reader.Read<uint32_t>();
	mesh.vertexStride = reader.Read<uint32_t>();
	mesh.vertexStrideDepth = reader.Read<uint32_t>();
	
	for (uint32_t i = 0; i < maxAttribs; ++i)
	{
		ReadAttrib(mesh.attrib[i], reader);
	}
	for (uint32_t i = 0; i < maxAttribs; ++i)
	{
		ReadAttrib(mesh.attribDepth[i], reader);
	}

	mesh.vertexDataByteOffset = reader.Read<uint32_t>();
	mesh.vertexCount = reader.Read<uint32_t>();
	mesh.indexDataByteOffset = reader.Read<uint32_t>();
	mesh.indexCount = reader.Read<uint32_t>();

	mesh.vertexDataByteOffsetDepth = reader.Read<uint32_t>();
	mesh.vertexCountDepth = reader.Read<uint32_t>();
}


struct Material
{
	DirectX::XMVECTOR diffuse;
	DirectX::XMVECTOR specular;
	DirectX::XMVECTOR ambient;
	DirectX::XMVECTOR emissive;
	DirectX::XMVECTOR transparent;
	float opacity;
	float shininess;
	float specularStrength;

	enum{maxTexPath = 128};
	enum{texCount = 6};
	string texDiffusePath;
	string texSpecularPath;
	string texEmissivePath;
	string texNormalPath;
	string texLightmapPath;
	string texReflectionPath;

	enum{maxMaterialName=128};
	string name;
};

void ReadMaterial(Material& material, BinaryReader& reader)
{
	material.diffuse = reader.Read<DirectX::XMVECTOR>();
	material.specular = reader.Read<DirectX::XMVECTOR>();
	material.ambient = reader.Read<DirectX::XMVECTOR>();
	material.emissive = reader.Read<DirectX::XMVECTOR>();
	material.transparent = reader.Read<DirectX::XMVECTOR>();
	material.opacity = reader.Read<float>();
	material.shininess = reader.Read<float>();
	material.specularStrength = reader.Read<float>();

	material.texDiffusePath = reader.ReadArray<char>(Material::maxTexPath);
	material.texSpecularPath = reader.ReadArray<char>(Material::maxTexPath);
	material.texEmissivePath = reader.ReadArray<char>(Material::maxTexPath);
	material.texNormalPath = reader.ReadArray<char>(Material::maxTexPath);
	material.texLightmapPath = reader.ReadArray<char>(Material::maxTexPath);
	material.texReflectionPath = reader.ReadArray<char>(Material::maxTexPath);
	material.name = reader.ReadArray<char>(Material::maxMaterialName);
}

} // H3D namespace


namespace
{

string BuildTexturePath(const string& basePath)
{
	string ret = basePath;

	bool appendExtension = false;
	string extension;

	auto sepIndex = basePath.rfind('.');
	if (sepIndex == string::npos)
	{
		// Assume DDS
		ret += ".dds";
	}
	return ret;
}

shared_ptr<Texture> LoadTexture(const string& path, bool isSrgb, shared_ptr<Texture> defaultTexture)
{
	auto& filesystem = Filesystem::GetInstance();
	if (filesystem.IsRegularFile(path))
	{
		return Texture::Load(path, isSrgb);
	}
	return defaultTexture;
}

shared_ptr<Texture> LoadTexture2(const string& path, const string& path2, bool isSrgb, shared_ptr<Texture> defaultTexture)
{
	auto& filesystem = Filesystem::GetInstance();
	if (filesystem.IsRegularFile(path))
	{
		return Texture::Load(path, isSrgb);
	}
	else if (filesystem.IsRegularFile(path2))
	{
		return Texture::Load(path2, isSrgb);
	}
	return defaultTexture;
}

} // anonymous namespace


namespace Kodiak
{

shared_ptr<StaticModel> LoadModelH3D(const string& fullPath)
{
	BinaryReader reader(fullPath);

	H3D::Header header;
	auto headerSize = sizeof(H3D::Header);

	ReadHeader(header, reader);
	
	std::vector<H3D::Mesh> meshes(header.meshCount);
	for (uint32_t i = 0; i < header.meshCount; ++i)
	{
		ReadMesh(meshes[i], reader);
		reader.Read<uint32_t>(); // Padding
	}

	std::vector<H3D::Material> materials(header.materialCount);
	for (uint32_t i = 0; i < header.materialCount; ++i)
	{
		ReadMaterial(materials[i], reader);
		reader.Read<uint32_t>(); // Padding
	}

	auto vertexStride = meshes[0].vertexStride;
	auto vertexStrideDepth = meshes[0].vertexStrideDepth;

	// Vertex buffer
	const byte* vb_data = reader.ReadArray<byte>(header.vertexDataByteSize);
	shared_ptr<BaseVertexBufferData> vbData;
	vbData.reset(new VertexBufferDataRaw(vb_data, vertexStride, header.vertexDataByteSize));
	auto vbuffer = VertexBuffer::Create(vbData, Usage::Immutable);

	// Index buffer
	const byte* ib_data = reader.ReadArray<byte>(header.indexDataByteSize);
	shared_ptr<BaseIndexBufferData> ibData;
	ibData.reset(new IndexBufferData16(ib_data, header.indexDataByteSize));
	auto ibuffer = IndexBuffer::Create(ibData, Usage::Immutable);

	// Vertex buffer (depth)
	const byte* vb_data_depth = reader.ReadArray<byte>(header.vertexDataByteSizeDepth);
	shared_ptr<BaseVertexBufferData> vbDataDepth;
	vbDataDepth.reset(new VertexBufferDataRaw(vb_data_depth, vertexStrideDepth, header.vertexDataByteSizeDepth));
	auto vbufferDepth = VertexBuffer::Create(vbDataDepth, Usage::Immutable);

	// Index buffer (depth)
	const byte* ib_data_depth = reader.ReadArray<byte>(header.indexDataByteSize);
	shared_ptr<BaseIndexBufferData> ibDataDepth;
	ibDataDepth.reset(new IndexBufferData16(ib_data_depth, header.indexDataByteSize));
	auto ibufferDepth = IndexBuffer::Create(ibDataDepth, Usage::Immutable);

	// Create model
	auto model = make_shared<StaticModel>();

	bool asyncLoad = false;

	// Textures and materials
	shared_ptr<Texture> defDiffuse = Texture::Load("default.DDS", true);
	shared_ptr<Texture> defSpecular = Texture::Load("default_specular.DDS", true);
	shared_ptr<Texture> defNormal = Texture::Load("default_normal.DDS", false);

	vector<shared_ptr<Material>> opaqueMaterials(header.materialCount);
	vector<shared_ptr<Material>> shadowMaterials(header.materialCount);
	vector<shared_ptr<Material>> depthMaterials(header.materialCount);

	for (uint32_t i = 0; i < header.materialCount; ++i)
	{
		// Setup opaque base-pass material
		auto opaqueMaterial = opaqueMaterials[i] = make_shared<Material>();
		opaqueMaterial->SetEffect(GetDefaultBaseEffect());
		opaqueMaterial->SetRenderPass(GetDefaultBasePass());

		// Diffuse texture
		string diffusePath = BuildTexturePath(materials[i].texDiffusePath);
		auto diffuseTexture = LoadTexture(diffusePath, true, defDiffuse);
		diffuseTexture->AddPostLoadCallback([opaqueMaterial, diffuseTexture]()
		{
			opaqueMaterial->SetResource("texDiffuse", diffuseTexture);
		});

		// Specular texture
		string specularPath1 = BuildTexturePath(materials[i].texSpecularPath);
		string specularPath2 = BuildTexturePath(materials[i].texDiffusePath + "_specular");
		auto specularTexture = LoadTexture2(specularPath1, specularPath2, true, defSpecular);
		specularTexture->AddPostLoadCallback([opaqueMaterial, specularTexture]()
		{
			opaqueMaterial->SetResource("texSpecular", specularTexture);
		});
		
		// Normal texture
		string normalPath1 = BuildTexturePath(materials[i].texNormalPath);
		string normalPath2 = BuildTexturePath(materials[i].texDiffusePath + "_normal");
		auto normalTexture = LoadTexture2(normalPath1, normalPath2, false, defNormal);
		normalTexture->AddPostLoadCallback([opaqueMaterial, normalTexture]()
		{
			opaqueMaterial->SetResource("texNormal", normalTexture);
		});


		// TODO move this stuff to per-view data
		using namespace DirectX;
		opaqueMaterial->GetParameter("sunDirection")->SetValue(Vector3(0.336f, 0.924f, -0.183f));
		opaqueMaterial->GetParameter("sunColor")->SetValue(Vector3(4.0f, 4.0f, 4.0f));
		opaqueMaterial->GetParameter("ambientColor")->SetValue(Vector3(0.1f, 0.1f, 0.1f));
		opaqueMaterial->GetParameter("shadowTexelSize")->SetValue(1.0f / 2048.0f);

		// Setup depth material
		auto depthMaterial = depthMaterials[i] = make_shared<Material>();
		depthMaterial->SetEffect(GetDefaultDepthEffect());
		depthMaterial->SetRenderPass(GetDefaultDepthPass());

		diffuseTexture->AddPostLoadCallback([depthMaterial, diffuseTexture]()
		{
			depthMaterial->SetResource("texDiffuse", diffuseTexture);
		});
		
		// Setup shadow material
		auto shadowMaterial = shadowMaterials[i] = make_shared<Material>();
		shadowMaterial->SetEffect(GetDefaultShadowEffect());
		shadowMaterial->SetRenderPass(GetDefaultShadowPass());

		diffuseTexture->AddPostLoadCallback([shadowMaterial, diffuseTexture]()
		{
			shadowMaterial->SetResource("texDiffuse", diffuseTexture);
		});
	}

	// Create meshes
	for (uint32_t i = 0; i < header.meshCount; ++i)
	{
		const auto& h3dMesh = meshes[i];

		auto mesh = make_shared<StaticMesh>();

		// Opaque mesh part
		StaticMeshPart opaquePart{ 
			vbuffer, 
			ibuffer, 
			opaqueMaterials[h3dMesh.materialIndex],
			PrimitiveTopology::TriangleList, 
			h3dMesh.indexCount, 
			h3dMesh.indexDataByteOffset / sizeof(uint16_t),
			static_cast<int32_t>(h3dMesh.vertexDataByteOffset / vertexStride)};
		mesh->AddMeshPart(opaquePart);

		// Depth-only mesh part
		StaticMeshPart depthPart{
			vbuffer,
			ibuffer,
			depthMaterials[h3dMesh.materialIndex],
			PrimitiveTopology::TriangleList,
			h3dMesh.indexCount,
			h3dMesh.indexDataByteOffset / sizeof(uint16_t),
			static_cast<int32_t>(h3dMesh.vertexDataByteOffset / vertexStride)};
		mesh->AddMeshPart(depthPart);

		// Shadow mesh part
		StaticMeshPart shadowPart{
			vbuffer,
			ibuffer,
			shadowMaterials[h3dMesh.materialIndex],
			PrimitiveTopology::TriangleList,
			h3dMesh.indexCount,
			h3dMesh.indexDataByteOffset / sizeof(uint16_t),
			static_cast<int32_t>(h3dMesh.vertexDataByteOffset / vertexStride) };
		mesh->AddMeshPart(shadowPart);

		model->AddMesh(mesh);
	}

	return model;
}

} // namespace Kodiak