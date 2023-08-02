#pragma once
#include <fstream>
#include <string>

class Log
{
public:
	static bool Initialize()
	{
		mLogFile.open("InGameTimecycLog.txt");
		
		if(!mLogFile)
		{
			return false;
		}

		return true;
	}

	static void Info(const std::string message)
	{
		mLogFile << "Info: " + message + "\n";
	}

	static void Warning(const std::string message)
	{
		mLogFile << "Warning: " + message + "\n";
	}

	static void Error(const std::string message)
	{
		mLogFile << "ERROR: " + message + "\n";
	}

private:
	static std::ofstream mLogFile;
};