#ifndef _TASK_H_20230228_
#define _TASK_H_20230228_
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
class CTask {
public:
	CTask();
	~CTask();
	int exec(int argc, char* argv[]);
	int stop();
private:
	int execWithUI(int argc, char* argv[]);
	int execWithoutUI(int argc, char* argv[]);
	int runTask();
	int loadProject();
	int loadHMIProject();
	int loadBOXProject();
private:
	std::vector<std::string> m_vecParam;
	bool	m_bHMIMode;
	unsigned short m_nLanguage;
	std::mutex	m_mutex;
	std::condition_variable	m_cv;
};
#endif