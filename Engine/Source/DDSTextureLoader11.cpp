// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from DDSTextureLoader.cpp in Microsoft's Miniengine sample
// https://github.com/Microsoft/DirectX-Graphics-Samples
//

#include "Stdafx.h"

#include "DDSTextureLoader11.h"

#include "CommandList11.h"
#include "DDSCommon.h"
#include "Utility.h"

#include <locale>
#include <codecvt>


using namespace Kodiak;
using namespace std;
using namespace DirectX;


namespace
{

//--------------------------------------------------------------------------------------
static HRESULT CreateD3DResources(ID3D11Device* device,
	uint32_t resDim,
	size_t width,
	size_t height,
	size_t depth,
	size_t mipCount,
	size_t arraySize,
	DXGI_FORMAT format,
	D3D11_USAGE usage,
	unsigned int bindFlags,
	unsigned int cpuAccessFlags,
	unsigned int miscFlags,
	bool forceSRGB,
	bool isCubeMap,
	D3D11_SUBRESOURCE_DATA* initData,
	ID3D11Resource** texture,
	ID3D11ShaderResourceView** textureView)
{
	if (!device)
	{
		return E_POINTER;
	}

	HRESULT hr = E_FAIL;

	if (forceSRGB)
	{
		format = DDS::MakeSRGB(format);
	}

	switch (resDim)
	{
	case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
	{
		D3D11_TEXTURE1D_DESC desc;
		desc.Width = static_cast<UINT>(width);
		desc.MipLevels = static_cast<UINT>(mipCount);
		desc.ArraySize = static_cast<UINT>(arraySize);
		desc.Format = format;
		desc.Usage = usage;
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = cpuAccessFlags;
		desc.MiscFlags = miscFlags & ~D3D11_RESOURCE_MISC_TEXTURECUBE;

		ID3D11Texture1D* tex = nullptr;
		hr = device->CreateTexture1D(&desc, initData, &tex);

		if (SUCCEEDED(hr) && tex != 0)
		{
			if (textureView != 0)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
				memset(&SRVDesc, 0, sizeof(SRVDesc));
				SRVDesc.Format = format;

				if (arraySize > 1)
				{
					SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
					SRVDesc.Texture1DArray.MipLevels = (!mipCount) ? -1 : desc.MipLevels;
					SRVDesc.Texture1DArray.ArraySize = static_cast<UINT>(arraySize);
				}
				else
				{
					SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
					SRVDesc.Texture1D.MipLevels = (!mipCount) ? -1 : desc.MipLevels;
				}

				hr = device->CreateShaderResourceView(tex, &SRVDesc, textureView);
				if (FAILED(hr))
				{
					tex->Release();
					return hr;
				}
			}

			if (texture != 0)
			{
				*texture = tex;
			}
		}
	}
	break;

	case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
	{
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = static_cast<UINT>(width);
		desc.Height = static_cast<UINT>(height);
		desc.MipLevels = static_cast<UINT>(mipCount);
		desc.ArraySize = static_cast<UINT>(arraySize);
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = usage;
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = cpuAccessFlags;
		if (isCubeMap)
		{
			desc.MiscFlags = miscFlags | D3D11_RESOURCE_MISC_TEXTURECUBE;
		}
		else
		{
			desc.MiscFlags = miscFlags & ~D3D11_RESOURCE_MISC_TEXTURECUBE;
		}

		ID3D11Texture2D* tex = nullptr;
		hr = device->CreateTexture2D(&desc, initData, &tex);

		if (SUCCEEDED(hr) && tex != 0)
		{
			if (textureView != 0)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
				memset(&SRVDesc, 0, sizeof(SRVDesc));
				SRVDesc.Format = format;

				if (isCubeMap)
				{
					if (arraySize > 6)
					{
						SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
						SRVDesc.TextureCubeArray.MipLevels = (!mipCount) ? -1 : desc.MipLevels;

						// Earlier we set arraySize to (NumCubes * 6)
						SRVDesc.TextureCubeArray.NumCubes = static_cast<UINT>(arraySize / 6);
					}
					else
					{
						SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
						SRVDesc.TextureCube.MipLevels = (!mipCount) ? -1 : desc.MipLevels;
					}
				}
				else if (arraySize > 1)
				{
					SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
					SRVDesc.Texture2DArray.MipLevels = (!mipCount) ? -1 : desc.MipLevels;
					SRVDesc.Texture2DArray.ArraySize = static_cast<UINT>(arraySize);
				}
				else
				{
					SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
					SRVDesc.Texture2D.MipLevels = (!mipCount) ? -1 : desc.MipLevels;
				}

				hr = device->CreateShaderResourceView(tex, &SRVDesc, textureView);

				if (FAILED(hr))
				{
					tex->Release();
					return hr;
				}
			}

			if (texture != 0)
			{
				*texture = tex;
			}
		}
	}
	break;

	case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
	{
		D3D11_TEXTURE3D_DESC desc;
		desc.Width = static_cast<UINT>(width);
		desc.Height = static_cast<UINT>(height);
		desc.Depth = static_cast<UINT>(depth);
		desc.MipLevels = static_cast<UINT>(mipCount);
		desc.Format = format;
		desc.Usage = usage;
		desc.BindFlags = bindFlags;
		desc.CPUAccessFlags = cpuAccessFlags;
		desc.MiscFlags = miscFlags & ~D3D11_RESOURCE_MISC_TEXTURECUBE;

		ID3D11Texture3D* tex = nullptr;
		hr = device->CreateTexture3D(&desc, initData, &tex);

		if (SUCCEEDED(hr) && tex != 0)
		{
			if (textureView != 0)
			{
				D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc;
				memset(&SRVDesc, 0, sizeof(SRVDesc));
				SRVDesc.Format = format;

				SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
				SRVDesc.Texture3D.MipLevels = (!mipCount) ? -1 : desc.MipLevels;

				hr = device->CreateShaderResourceView(tex, &SRVDesc, textureView);
				if (FAILED(hr))
				{
					tex->Release();
					return hr;
				}
			}

			if (texture != 0)
			{
				*texture = tex;
			}
		}
	}
	break;
	}

	return hr;
}


//--------------------------------------------------------------------------------------
HRESULT FillInitData(size_t width,
	size_t height,
	size_t depth,
	size_t mipCount,
	size_t arraySize,
	DXGI_FORMAT format,
	size_t maxsize,
	size_t bitSize,
	const uint8_t* bitData,
	size_t& twidth,
	size_t& theight,
	size_t& tdepth,
	size_t& skipMip,
	D3D11_SUBRESOURCE_DATA* initData)
{
	if (!bitData || !initData)
	{
		return E_POINTER;
	}

	skipMip = 0;
	twidth = 0;
	theight = 0;
	tdepth = 0;

	size_t NumBytes = 0;
	size_t RowBytes = 0;
	const uint8_t* pSrcBits = bitData;
	const uint8_t* pEndBits = bitData + bitSize;

	size_t index = 0;
	for (size_t j = 0; j < arraySize; ++j)
	{
		size_t w = width;
		size_t h = height;
		size_t d = depth;
		for (size_t i = 0; i < mipCount; ++i)
		{
			DDS::GetSurfaceInfo(w, h, format, &NumBytes, &RowBytes, nullptr);

			if ((mipCount <= 1) || !maxsize || (w <= maxsize && h <= maxsize && d <= maxsize))
			{
				if (!twidth)
				{
					twidth = w;
					theight = h;
					tdepth = d;
				}

				assert(index < mipCount * arraySize);
				_Analysis_assume_(index < mipCount * arraySize);
				initData[index].pSysMem = (const void*)pSrcBits;
				initData[index].SysMemPitch = static_cast<UINT>(RowBytes);
				initData[index].SysMemSlicePitch = static_cast<UINT>(NumBytes);
				++index;
			}
			else if (!j)
			{
				// Count number of skipped mipmaps (first item only)
				++skipMip;
			}

			if (pSrcBits + (NumBytes*d) > pEndBits)
			{
				return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
			}

			pSrcBits += NumBytes * d;

			w = w >> 1;
			h = h >> 1;
			d = d >> 1;
			if (w == 0)
			{
				w = 1;
			}
			if (h == 0)
			{
				h = 1;
			}
			if (d == 0)
			{
				d = 1;
			}
		}
	}

	return (index > 0) ? S_OK : E_FAIL;
}


HRESULT CreateTextureFromDDS(ID3D11Device* device, 
	const DDS_HEADER* header, 
	const uint8_t* bitData,
	size_t bitSize, 
	size_t maxsize, 
	D3D11_USAGE usage, 
	uint32_t bindFlags,
	uint32_t cpuAccessFlags, 
	uint32_t miscFlags, 
	bool forceSRGB,
	ID3D11Resource** texture, 
	ID3D11ShaderResourceView** textureView)
{
	HRESULT hr = S_OK;

	size_t width = header->width;
	size_t height = header->height;
	size_t depth = header->depth;

	uint32_t resDim = D3D11_RESOURCE_DIMENSION_UNKNOWN;
	size_t arraySize = 1;
	DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
	bool isCubeMap = false;

	size_t mipCount = header->mipMapCount;
	if (0 == mipCount)
	{
		mipCount = 1;
	}

	if ((header->ddspf.flags & DDS_FOURCC) && (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC))
	{
		auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header + sizeof(DDS_HEADER));

		arraySize = d3d10ext->arraySize;
		if (arraySize == 0)
		{
			return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		}

		switch (d3d10ext->dxgiFormat)
		{
		case DXGI_FORMAT_AI44:
		case DXGI_FORMAT_IA44:
		case DXGI_FORMAT_P8:
		case DXGI_FORMAT_A8P8:
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

		default:
			if (DXGIUtility::BitsPerPixel(d3d10ext->dxgiFormat) == 0)
			{
				return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
			}
		}

		format = d3d10ext->dxgiFormat;

		switch (d3d10ext->resourceDimension)
		{
		case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
			// D3DX writes 1D textures with a fixed Height of 1
			if ((header->flags & DDS_HEIGHT) && height != 1)
			{
				return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
			}
			height = depth = 1;
			break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
			if (d3d10ext->miscFlag & D3D11_RESOURCE_MISC_TEXTURECUBE)
			{
				arraySize *= 6;
				isCubeMap = true;
			}
			depth = 1;
			break;

		case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
			if (!(header->flags & DDS_HEADER_FLAGS_VOLUME))
			{
				return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
			}

			if (arraySize > 1)
			{
				return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
			}
			break;

		default:
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}

		resDim = d3d10ext->resourceDimension;
	}
	else
	{
		format = DDS::GetDXGIFormat(header->ddspf);

		if (format == DXGI_FORMAT_UNKNOWN)
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}

		if (header->flags & DDS_HEADER_FLAGS_VOLUME)
		{
			resDim = D3D11_RESOURCE_DIMENSION_TEXTURE3D;
		}
		else
		{
			if (header->caps2 & DDS_CUBEMAP)
			{
				// We require all six faces to be defined
				if ((header->caps2 & DDS_CUBEMAP_ALLFACES) != DDS_CUBEMAP_ALLFACES)
				{
					return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
				}

				arraySize = 6;
				isCubeMap = true;
			}

			depth = 1;
			resDim = D3D11_RESOURCE_DIMENSION_TEXTURE2D;

			// Note there's no way for a legacy Direct3D 9 DDS to express a '1D' texture
		}

		assert(DXGIUtility::BitsPerPixel(format) != 0);
	}

	// Bound sizes (for security purposes we don't trust DDS file metadata larger than the D3D 11.x hardware requirements)
	if (mipCount > D3D11_REQ_MIP_LEVELS)
	{
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	switch (resDim)
	{
	case D3D11_RESOURCE_DIMENSION_TEXTURE1D:
		if ((arraySize > D3D11_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
			(width > D3D11_REQ_TEXTURE1D_U_DIMENSION))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	case D3D11_RESOURCE_DIMENSION_TEXTURE2D:
		if (isCubeMap)
		{
			// This is the right bound because we set arraySize to (NumCubes*6) above
			if ((arraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
				(width > D3D11_REQ_TEXTURECUBE_DIMENSION) ||
				(height > D3D11_REQ_TEXTURECUBE_DIMENSION))
			{
				return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
			}
		}
		else if ((arraySize > D3D11_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
			(width > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
			(height > D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	case D3D11_RESOURCE_DIMENSION_TEXTURE3D:
		if ((arraySize > 1) ||
			(width > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
			(height > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
			(depth > D3D11_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;
	}

	// Create the texture
	std::unique_ptr<D3D11_SUBRESOURCE_DATA[]>
		initData(new (std::nothrow) D3D11_SUBRESOURCE_DATA[mipCount * arraySize]);
	if (!initData)
	{
		return E_OUTOFMEMORY;
	}

	size_t skipMip = 0;
	size_t twidth = 0;
	size_t theight = 0;
	size_t tdepth = 0;
	hr = FillInitData(width, height, depth, mipCount, arraySize, format, maxsize, bitSize, bitData,
		twidth, theight, tdepth, skipMip, initData.get());

	if (SUCCEEDED(hr))
	{
		hr = CreateD3DResources(device, resDim, twidth, theight, tdepth, mipCount - skipMip, arraySize,
			format, usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB,
			isCubeMap, initData.get(), texture, textureView);

		if (FAILED(hr) && !maxsize && (mipCount > 1))
		{
			// Retry with a maxsize determined by feature level
			switch (device->GetFeatureLevel())
			{
			case D3D_FEATURE_LEVEL_9_1:
			case D3D_FEATURE_LEVEL_9_2:
				if (isCubeMap)
				{
					maxsize = 512 /*D3D_FL9_1_REQ_TEXTURECUBE_DIMENSION*/;
				}
				else
				{
					maxsize = (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
						? 256 /*D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION*/
						: 2048 /*D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
				}
				break;

			case D3D_FEATURE_LEVEL_9_3:
				maxsize = (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
					? 256 /*D3D_FL9_1_REQ_TEXTURE3D_U_V_OR_W_DIMENSION*/
					: 4096 /*D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
				break;

			default: // D3D_FEATURE_LEVEL_10_0 & D3D_FEATURE_LEVEL_10_1
				maxsize = (resDim == D3D11_RESOURCE_DIMENSION_TEXTURE3D)
					? 2048 /*D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION*/
					: 8192 /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
				break;
			}

			hr = FillInitData(width, height, depth, mipCount, arraySize, format, maxsize, bitSize, bitData,
				twidth, theight, tdepth, skipMip, initData.get());

			if (SUCCEEDED(hr))
			{
				hr = CreateD3DResources(device, resDim, twidth, theight, tdepth, mipCount - skipMip, arraySize,
					format, usage, bindFlags, cpuAccessFlags, miscFlags, forceSRGB,
					isCubeMap, initData.get(), texture, textureView);
			}
		}
	}

	return hr;
}


//--------------------------------------------------------------------------------------
DDS_ALPHA_MODE GetAlphaMode(const DDS_HEADER* header)
{
	if (header->ddspf.flags & DDS_FOURCC)
	{
		if (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)
		{
			auto d3d10ext = reinterpret_cast<const DDS_HEADER_DXT10*>((const char*)header + sizeof(DDS_HEADER));
			auto mode = static_cast<DDS_ALPHA_MODE>(d3d10ext->miscFlags2 & DDS_MISC_FLAGS2_ALPHA_MODE_MASK);
			switch (mode)
			{
			case DDS_ALPHA_MODE_STRAIGHT:
			case DDS_ALPHA_MODE_PREMULTIPLIED:
			case DDS_ALPHA_MODE_OPAQUE:
			case DDS_ALPHA_MODE_CUSTOM:
				return mode;
			}
		}
		else if ((MAKEFOURCC('D', 'X', 'T', '2') == header->ddspf.fourCC)
			|| (MAKEFOURCC('D', 'X', 'T', '4') == header->ddspf.fourCC))
		{
			return DDS_ALPHA_MODE_PREMULTIPLIED;
		}
	}

	return DDS_ALPHA_MODE_UNKNOWN;
}

} // anonymous namespace


namespace Kodiak
{

//--------------------------------------------------------------------------------------
HRESULT CreateDDSTextureFromMemory(
	ID3D11Device* d3dDevice,
	const uint8_t* ddsData,
	size_t ddsDataSize,
	size_t maxsize,
	bool forceSRGB,
	ID3D11Resource** texture,
	ID3D11ShaderResourceView** textureView,
	DDS_ALPHA_MODE* alphaMode)
{
	if (texture)
	{
		*texture = nullptr;
	}

	if (alphaMode)
	{
		*alphaMode = DDS_ALPHA_MODE_UNKNOWN;
	}

	if (!d3dDevice || !ddsData)
	{
		return E_INVALIDARG;
	}

	// Validate DDS file in memory
	if (ddsDataSize < (sizeof(uint32_t) + sizeof(DDS_HEADER)))
	{
		return E_FAIL;
	}

	uint32_t dwMagicNumber = *(const uint32_t*)(ddsData);
	if (dwMagicNumber != DDS_MAGIC)
	{
		return E_FAIL;
	}

	auto header = reinterpret_cast<const DDS_HEADER*>(ddsData + sizeof(uint32_t));

	// Verify header to validate DDS file
	if (header->size != sizeof(DDS_HEADER) ||
		header->ddspf.size != sizeof(DDS_PIXELFORMAT))
	{
		return E_FAIL;
	}

	size_t offset = sizeof(DDS_HEADER) + sizeof(uint32_t);

	// Check for extensions
	if (header->ddspf.flags & DDS_FOURCC)
	{
		if (MAKEFOURCC('D', 'X', '1', '0') == header->ddspf.fourCC)
		{
			offset += sizeof(DDS_HEADER_DXT10);
		}
	}

	// Must be long enough for all headers and magic value
	if (ddsDataSize < offset)
	{
		return E_FAIL;
	}

	HRESULT hr = CreateTextureFromDDS(
		d3dDevice,
		header, 
		ddsData + offset, 
		ddsDataSize - offset, 
		maxsize,
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_SHADER_RESOURCE,
		0, // CPU access flag
		0, // Misc flag
		forceSRGB, 
		texture, 
		textureView);

	if (SUCCEEDED(hr))
	{
		if (texture != nullptr && *texture != nullptr)
		{
			const string debugName("DDSTextureLoader");
			(*texture)->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(debugName.size()), debugName.c_str());
		}

		if (alphaMode)
		{
			*alphaMode = GetAlphaMode(header);
		}
	}

	return hr;
}


HRESULT CreateDDSTextureFromFile(
	ID3D11Device* d3dDevice,
	const string& fileName,
	size_t maxsize,
	bool forceSRGB,
	ID3D11Resource** texture,
	ID3D11ShaderResourceView** textureView,
	DDS_ALPHA_MODE* alphaMode)
{
	if (texture)
	{
		*texture = nullptr;
	}

	if (alphaMode)
	{
		*alphaMode = DDS_ALPHA_MODE_UNKNOWN;
	}

	if (!d3dDevice || fileName.empty())
	{
		return E_INVALIDARG;
	}

	DDS_HEADER* header = nullptr;
	uint8_t* bitData = nullptr;
	size_t bitSize = 0;

	std::unique_ptr<uint8_t[]> ddsData;

	wstring_convert<codecvt_utf8_utf16<wchar_t>> converter;
	wstring wide = converter.from_bytes(fileName);

	HRESULT hr = DDS::LoadTextureDataFromFile(wide, ddsData, &header, &bitData, &bitSize);
	if (FAILED(hr))
	{
		return hr;
	}

	hr = CreateTextureFromDDS(
		d3dDevice, 
		header, 
		bitData, 
		bitSize, 
		maxsize, 
		D3D11_USAGE_IMMUTABLE,
		D3D11_BIND_SHADER_RESOURCE,
		0, // CPU access flag
		0, // Misc flag
		forceSRGB, 
		texture, 
		textureView);

	if (alphaMode)
	{
		*alphaMode = GetAlphaMode(header);
	}

	return hr;
}

} // namespace Kodiak