// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Filesystem.h"

#include "DebugUtility.h"


using namespace Kodiak;
using namespace std;


Filesystem& Filesystem::GetInstance()
{
	static Filesystem instance;
	return instance;
}


Filesystem::Filesystem()
{
	Initialize();
}


void Filesystem::SetBinaryPath(const string& path, bool patchRootDir)
{
	assert_msg(m_binarySubpath.empty(), "You can only call SetBinaryPath once!");

	string prevRoot = m_rootDir;

	m_binarySubpath = path;
	auto pos = m_binaryDir.find(m_binarySubpath);
	if (pos != m_binaryDir.npos)
	{
		m_rootDir = m_binaryDir.substr(0, pos);
	}

	if (patchRootDir)
	{
		PathDesc* path = m_searchPaths;
		while (path)
		{
			if (path->mountRoot == prevRoot)
			{
				path->mountRoot = m_rootDir;
				path->fullPath = path->mountRoot + "\\" + path->mountPath;
			}
			path = path->next;
		}
	}
}


void Filesystem::Mount(const string& path, bool appendPath)
{
	PathDesc* pathPtr = nullptr;
	PathDesc* cur = m_searchPaths;
	PathDesc* prev = nullptr;

	while (cur)
	{
		if (cur->mountPath == path)
		{
			pathPtr = cur;
			break;
		}
		prev = cur;
		cur = cur->next;
	}

	if (pathPtr)
	{
		return;
	}

	PathDesc* newPathDesc = new PathDesc;
	newPathDesc->mountRoot = m_rootDir;
	newPathDesc->mountPath = path;
	newPathDesc->fullPath = newPathDesc->mountRoot + "\\" + newPathDesc->mountPath;

	if (appendPath)
	{
		if (prev == nullptr)
		{
			m_searchPaths = newPathDesc;
		}
		else
		{
			prev->next = newPathDesc;
		}
	}
	else
	{
		newPathDesc->next = m_searchPaths;
		m_searchPaths = newPathDesc;
	}
}


void Filesystem::Initialize()
{
	// Get Path
	string path;
	path.resize(4096, 0);
	GetModuleFileNameA(nullptr, &path[0], 4096);

	// Change current working directory to that of the executable
	char* lastSlash = strrchr(&path[0], '\\');
	*lastSlash = 0;

	bool result = TRUE == SetCurrentDirectoryA(path.c_str());

	m_binaryDir = path;
	m_rootDir = m_binaryDir;

	assert(m_searchPaths == nullptr);
	
	m_searchPaths = new PathDesc;
	m_searchPaths->next = nullptr;
	m_searchPaths->mountRoot = m_rootDir;
	m_searchPaths->mountPath = "";
	m_searchPaths->fullPath = m_rootDir;
}