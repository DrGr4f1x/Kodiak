// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "Paths.h"

using namespace Kodiak;
using namespace std;


Paths& Paths::GetInstance()
{
	static Paths instance;
	return instance;
}


Paths::Paths()
	: m_baseDir()
	, m_binaryDir()
	, m_modelDir()
	, m_textureDir()
	, m_shaderDir()
	, m_logDir()
{
	Initialize();
}


const string& Paths::BaseDir() const
{
	return m_baseDir;
}


const string& Paths::BinaryDir() const
{
	return m_binaryDir;
}


const string& Paths::ModelDir() const
{
	return m_modelDir;
}


const string& Paths::TextureDir() const
{
	return m_textureDir;
}


const string& Paths::ShaderDir() const
{
	return m_shaderDir;
}


const string& Paths::LogDir() const
{
	return m_logDir;
}


void Paths::Initialize()
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

	auto pos = m_binaryDir.find("Bin");
	if (pos != m_binaryDir.npos)
	{
		m_baseDir = m_binaryDir.substr(0, pos);
	}
	else
	{
		// In case Visual Studio made "bin" all lowercase
		pos = m_binaryDir.find("bin");
		if (pos != m_binaryDir.npos)
		{
			m_baseDir = m_binaryDir.substr(0, pos);
		}
	}

	m_modelDir = m_baseDir + string("Models\\");
	m_textureDir = m_baseDir + string("Textures\\");
	m_shaderDir = m_baseDir + string("Shaders\\");
	m_logDir = m_baseDir + string("Log\\");
}