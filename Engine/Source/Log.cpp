#include "Stdafx.h"

#include "Log.h"

#include "Paths.h"

#include <concurrent_queue.h>


using namespace Kodiak;
using namespace std;


namespace
{

class LogHandler
{
public:
	static LogHandler& GetInstance()
	{
		static LogHandler instance;
		return instance;
	}

	bool IsInitialized() const { return m_initialized; }

	void Initialize()
	{
		lock_guard<mutex> lock(m_initializationMutex);

		// Only initialize if we are in uninitialized state
		if (m_initialized)
		{
			return;
		}

		CreateLogFile();

		m_haltLogging = false;

		m_workerLoop = async(launch::async,
			[&] 
			{
				while (!m_haltLogging)
				{
					LogMessage message;
					if (m_messageQueue.try_pop(message))
					{
						m_file << message.message;
					}
				}
			}
			);

		m_initialized = true;
	}


	void Shutdown()
	{
		lock_guard<mutex> lock(m_initializationMutex);

		// Only shutdown if we are in initialized state
		if (!m_initialized)
		{
			return;
		}

		m_haltLogging = true;
		m_workerLoop.get();

		m_file.close();

		m_initialized = false;
	}


	void PostLogMessage(const LogMessage& message)
	{
		if (m_initialized)
		{
			m_messageQueue.push(message);
		}
	}


private:
	LogHandler()
		: m_initialized(false)
	{}

	void CreateLogFile()
	{
		// Get the log directory path
		string fullPath = Paths::GetInstance().LogDir();

		// Build the filename
		SYSTEMTIME dateTime;
		GetLocalTime(&dateTime);

		// Append current date+time and extension to filename
		string filenameBase = "Log";
		string timestampStr;
		char timestamp[256];
		sprintf_s(timestamp, "-%04d%02d%02d%02d%02d%02d.txt\0", dateTime.wYear, dateTime.wMonth, dateTime.wDay, dateTime.wHour, dateTime.wMinute, dateTime.wSecond);
		timestampStr = timestamp;
		string sTSFilename = filenameBase + timestampStr;

		// Build the full path
		string sTSFullPath = fullPath + sTSFilename;

		// Open the file stream
		m_file.open(sTSFullPath.c_str(), ios::out | ios::trunc);
		m_file << fixed;

		// Create a hard link to the non-timestamped file
		string filename = fullPath + filenameBase + ".txt";
		DeleteFileA(filename.c_str());
		CreateHardLinkA(filename.c_str(), sTSFullPath.c_str(), 0);
	}

private:
	mutex m_initializationMutex;
	ofstream m_file;
	Concurrency::concurrent_queue<LogMessage> m_messageQueue;
	atomic<bool> m_haltLogging;
	future<void> m_workerLoop;
	atomic<bool> m_initialized;
};

} // anonymous namespace


namespace Kodiak
{

string ELogLevelToString(ELogLevel level)
{
	switch (level)
	{
	case ELogLevel::Error:
		return string("Error");
		break;

	case ELogLevel::Warning:
		return string("Warning");
		break;

	case ELogLevel::Debug:
		return string("Debug");
		break;

	default:
		return string("Info");
		break;
	}
}


void PostLogMessage(const LogMessage& message)
{
	LogHandler::GetInstance().PostLogMessage(message);
}


void InitializeLogging()
{
	LogHandler::GetInstance().Initialize();
}


void ShutdownLogging()
{
	LogHandler::GetInstance().Shutdown();
}

} // namespace Kodiak