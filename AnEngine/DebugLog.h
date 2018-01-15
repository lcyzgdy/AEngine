#pragma once
#ifndef __DEBUGLOG_H__
#define __DEBUGLOG_H__

#include<fstream>
#include<string>
#include<mutex>
#include<ctime>

namespace AnEngine::Debug
{
	std::ofstream out(to_string(time(0)) + "_log.txt");

	void Log(std::string l)
	{
		out << l << std::endl;
	}
}

#endif // !__DEBUGLOG_H__