#pragma once

#include <regex>
#include <Windows.h>

class CWorker {
public:
	CWorker(const std::string& targetWordsFilename, int id, HANDLE& mappedFile);
	~CWorker();
	void Work();

private:
	int id;
	std::regex targetWords;

	HANDLE newTaskEvent;		//����������, ��������������� ��� �������� ��������� ��������.  (void*), �� �� ���?
	HANDLE finishedTaskEvent;
	HANDLE terminateEvent;

	HANDLE creatinonFObj;
	char* fileMapV;
};
