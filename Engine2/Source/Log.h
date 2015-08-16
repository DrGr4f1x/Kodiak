#pragma once

namespace Kodiak
{

enum class ELogLevel
{
	Error,
	Warning,
	Info,
	Debug
};


std::string ELogLevelToString(ELogLevel level);

void InitializeLogging();
void ShutdownLogging();


struct LogMessage
{
	std::string message;
};


void PostLogMessage(const LogMessage& message);


template <ELogLevel TLevel>
class Logger
{
public:
	Logger() {}

	~Logger()
	{
		std::string message(ELogLevelToString(TLevel));
		message += ": ";
		message += m_stream.str();
		message += "\n";

		PostLogMessage({ message });
	}

	std::ostringstream& MessageStream() { return m_stream; }

private:
	std::ostringstream m_stream;
};


#define LOG(level) Logger<level>().MessageStream()
#define LOG_ERROR Logger<ELogLevel::Error>().MessageStream()
#define LOG_WARNING Logger<ELogLevel::Warning>().MessageStream()
#define LOG_INFO Logger<ELogLevel::Info>().MessageStream()
#define LOG_DEBUG Logger<ELogLevel::Debug>().MessageStream()

} // namespace Kodiak