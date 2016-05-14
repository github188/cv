#pragma once

#ifndef LOG_H
#define LOG_H

#include <fstream>
using namespace std;

class Log
{
public:
	Log(void);
	~Log(void);

	void startWrite();//ʱ�䣬�ļ�����������

	void write(char *param);
	void write(int param);
	void write(void *p);
	void endWrite();

private:
	ofstream out;
};



#endif