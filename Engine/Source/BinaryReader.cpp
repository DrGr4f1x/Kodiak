// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Adapted from BinaryReader.h in Microsoft's DirectXTK project
// https://github.com/Microsoft/DirectXTK
//

#include "Stdafx.h"

#include "BinaryReader.h"

#include "RenderUtils.h"
#include "Utility.h"


using namespace Kodiak;
using namespace std;


// Constructor reads from the filesystem
BinaryReader::BinaryReader(const string& fileName)
{
	size_t dataSize{ 0 };

	ThrowIfFailed(ReadEntireFile(fileName, m_ownedData, &dataSize));
	
	m_pos = m_ownedData.get();
	m_end = m_ownedData.get() + dataSize;
}


// Constructor reads from existing memory buffer
BinaryReader::BinaryReader(const uint8_t* dataBlob, size_t dataSize)
{
	m_pos = dataBlob;
	m_end = dataBlob + dataSize;
}


// Reads from the filesystem into memory
HRESULT BinaryReader::ReadEntireFile(const string& fileName, unique_ptr<uint8_t[]>& data, size_t* dataSize)
{
	ScopedHandle hFile(SafeHandle(CreateFileA(fileName.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr)));

	if (!hFile)
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	// Get the file size
	LARGE_INTEGER fileSize{ 0 };

	FILE_STANDARD_INFO fileInfo;

	if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	fileSize = fileInfo.EndOfFile;

	// File is too big for 32-bit allocation, so reject read.
	if (fileSize.HighPart > 0)
	{
		return E_FAIL;
	}

	// Create enough space for the file data.
	data.reset(new uint8_t[fileSize.LowPart]);

	if (!data)
	{
		return E_OUTOFMEMORY;
	}

	// Read the data in.
	DWORD bytesRead{ 0 };

	if (!ReadFile(hFile.get(), data.get(), fileSize.LowPart, &bytesRead, nullptr))
	{
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (bytesRead < fileSize.LowPart)
	{
		return E_FAIL;
	}

	*dataSize = bytesRead;

	return S_OK;
}