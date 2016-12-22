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

#include <Shlwapi.h>

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


void Filesystem::SetRootDir(const string& rootDir)
{
	unique_lock<shared_mutex> CS(m_mutex);

	if (m_rootDir == rootDir)
	{
		return;
	}

	RemoveAllSearchPaths();

	m_rootDir = rootDir;
}


void Filesystem::AddSearchPath(const string& path, bool appendPath)
{
	PathDesc* pathPtr = nullptr;
	PathDesc* cur = m_searchPaths;
	PathDesc* prev = nullptr;

	unique_lock<shared_mutex> CS(m_mutex);

	while (cur)
	{
		if (cur->localPath == path)
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

	string fullpath = m_rootDir + path;
	FileStat stat;
	if (!InternalGetFileStat(fullpath, stat))
	{
		assert_msg(false, "Path %s is not a valid archive path beneath root.", fullpath.c_str());
		return;
	}

	PathDesc* newPathDesc = new PathDesc;
	newPathDesc->localPath = path;
	newPathDesc->fullPath = fullpath;

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


void Filesystem::RemoveSearchPath(const string& path)
{
	unique_lock<shared_mutex> CS(m_mutex);

	if (path == m_rootDir)
	{
		return;
	}

	PathDesc* cur = m_searchPaths;
	PathDesc* prev = nullptr;
	while (cur)
	{
		if (cur->localPath == path)
		{
			// Head
			if (prev == nullptr)
			{
				m_searchPaths = cur->next;
			}
			else
			{
				prev->next = cur->next;
			}
			delete cur;
			break;
		}
		prev = cur;
		cur = cur->next;
	}
}


void Filesystem::RemoveAllSearchPaths()
{
	PathDesc* cur = m_searchPaths;
	while (cur)
	{
		PathDesc* temp = cur->next;
		delete cur;
		cur = temp;
	}
	m_searchPaths = nullptr;
}


vector<string> Filesystem::GetSearchPaths()
{
	shared_lock<shared_mutex> CS(m_mutex);

	vector<string> paths;

	PathDesc* cur = m_searchPaths;
	while (cur)
	{
		paths.push_back(cur->fullPath);
		cur = cur->next;
	}

	return paths;
}


bool Filesystem::Exists(const string& fname)
{
	shared_lock<shared_mutex> CS(m_mutex);

	PathDesc* cur = m_searchPaths;
	while (cur)
	{
		string fullpath = cur->fullPath + "\\" + fname;
		if (TRUE == PathFileExistsA(fullpath.c_str()))
		{
			return true;
		}
		cur = cur->next;
	}
	return false;
}


bool Filesystem::IsRegularFile(const std::string& fname)
{
	shared_lock<shared_mutex> CS(m_mutex);

	FileStat stat;
	if (GetFileStat(fname, stat))
	{
		return stat.filetype == EFileType::Regular;
	}
	return false;
}


bool Filesystem::IsDirectory(const std::string& fname)
{
	shared_lock<shared_mutex> CS(m_mutex);

	FileStat stat;
	if (GetFileStat(fname, stat))
	{
		return stat.filetype == EFileType::Directory;
	}
	return false;
}


bool Filesystem::GetFileStat(const string& fname, FileStat& stat)
{
	shared_lock<shared_mutex> CS(m_mutex);

	PathDesc* cur = m_searchPaths;
	while (cur)
	{
		string fullpath = cur->fullPath + "\\" + fname;
		if (InternalGetFileStat(fullpath.c_str(), stat))
		{
			return true;
		}
		cur = cur->next;
	}
	return false;
}


string Filesystem::GetFullPath(const string& fname)
{
	shared_lock<shared_mutex> CS(m_mutex);

	FileStat stat;
	PathDesc* cur = m_searchPaths;
	while (cur)
	{
		string fullpath = cur->fullPath + "\\" + fname;
		if (InternalGetFileStat(fullpath.c_str(), stat))
		{
			return fullpath;
		}
		cur = cur->next;
	}
	return "";
}


void Filesystem::Initialize()
{
	unique_lock<shared_mutex> CS(m_mutex);

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
}


static __int64 PackFileTime(const FILETIME& ft)
{
	SYSTEMTIME st_utc;
	SYSTEMTIME st_localtz;
	TIME_ZONE_INFORMATION tzi;

	if (TRUE != FileTimeToSystemTime(&ft, &st_utc))
	{
		return -1;
	}

	if (TIME_ZONE_ID_INVALID == GetTimeZoneInformation(&tzi))
	{
		return -1;
	}

	if (TRUE != SystemTimeToTzSpecificLocalTime(&tzi, &st_utc, &st_localtz))
	{
		return -1;
	}

	struct tm tm;
	tm.tm_sec = st_localtz.wSecond;
	tm.tm_min = st_localtz.wMinute;
	tm.tm_hour = st_localtz.wHour;
	tm.tm_mday = st_localtz.wDay;
	tm.tm_mon = st_localtz.wMonth - 1;
	tm.tm_year = st_localtz.wYear - 1900;
	tm.tm_wday = -1 /*st_localtz.wDayOfWeek*/;
	tm.tm_yday = -1;
	tm.tm_isdst = -1;

	return (__int64)mktime(&tm);
}


bool Filesystem::InternalGetFileStat(const string& fullpath, FileStat& stat)
{
	// Assume the caller has a shared_lock on m_mutex!!
	WIN32_FILE_ATTRIBUTE_DATA winstat;

	if (TRUE != GetFileAttributesExA(fullpath.c_str(), GetFileExInfoStandard, &winstat))
	{
		return false;
	}

	stat.modtime = PackFileTime(winstat.ftLastWriteTime);
	stat.accesstime = PackFileTime(winstat.ftLastAccessTime);
	stat.createtime = PackFileTime(winstat.ftCreationTime);

	if (winstat.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{
		stat.filetype = EFileType::Directory;
		stat.filesize = 0;
	}
	else if (winstat.dwFileAttributes & (FILE_ATTRIBUTE_OFFLINE | FILE_ATTRIBUTE_DEVICE))
	{
		stat.filetype = EFileType::Other;
		stat.filesize = 0;
	}
	else
	{
		stat.filetype = EFileType::Regular;
		stat.filesize = (((__int64)winstat.nFileSizeHigh) << 32) | winstat.nFileSizeLow;
	}

	stat.readonly = ((winstat.dwFileAttributes & FILE_ATTRIBUTE_READONLY) != 0);

	return true;
}