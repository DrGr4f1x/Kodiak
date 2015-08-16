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
	, m_assetDir()
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


const string& Paths::AssetDir() const
{
	return m_assetDir;
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

	m_assetDir = m_baseDir + string("Assets\\");
	m_logDir = m_baseDir + string("Log\\");
}