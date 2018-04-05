#define _SCL_SECURE_NO_WARNINGS // to enable unsafe iterators for std::regex_replace

#include <iterator>
#include <fstream>
#include <sstream>
#include <cassert>
#include <locale>
#include <iostream>

#include "Worker.h"

using namespace std;

/*
* -создать объект(CreateFileMapping);
* -получить доступ к данным(MapViewOfFile);
* -закрыть доступ к данным(UnMapViewOfFile);
* -попрощаться с объектом(CloseHandle).
*/

/*
* Если вспомнить написанное выше, то страничный файл операционной системы используется как расширение памяти.
* Под расширением понимается процесс подкачки (swap) ОЗУ, вызывающийся системой по мере необходимости.
* Таким образом, создав объект файлового отображения, связанный со страничным свап-файлом, мы получим в качестве результата
* выделение глобально доступной памяти. Так как эта память является общедоступной, то любой другой процесс,
* создавший экземпляр объекта файлового отображения, будет иметь доступ к этим данным.
*/



std::wstring readFile(wchar_t* filename)
{
	HANDLE sourceFile = CreateFile(filename, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(sourceFile != NULL);
	DWORD sourceFileSize = GetFileSize(sourceFile, NULL);
	HANDLE sourceFileMap = CreateFileMapping(sourceFile, NULL, PAGE_READONLY, 0, sourceFileSize, NULL);
	PVOID sourceText = MapViewOfFile(sourceFileMap, FILE_MAP_READ, 0, 0, 0);
	std::wstring result(&((wchar_t*)sourceText)[1]);
	UnmapViewOfFile(sourceFileMap);
	CloseHandle(sourceFile);
	CloseHandle(sourceFileMap);
	return result;
}

void writeToFile(wchar_t* filename, std::wstring strToWrite) {
	HANDLE sourceFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	assert (sourceFile == NULL);
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = WriteFile(
		sourceFile,
		strToWrite.c_str(),
		strToWrite.length() * sizeof(wchar_t),
		&dwBytesWritten,
		NULL);
	CloseHandle(sourceFile);
}

CWorker::CWorker(const string& targetWordsFilename, int id, HANDLE& mappedFile)
	: id(id)
{
	string line;
	ifstream targetWordsFile;
	targetWordsFile.open(targetWordsFilename);		

	vector<wstring> fourChar;
	//fourChar.resize(10);

	if (targetWordsFile.is_open())
	{
		while (getline(targetWordsFile, line))
		{
			std::wstring wsTmp(line.begin(), line.end());
			fourChar.push_back(wsTmp);
		}
		targetWordsFile.close();
	}
	else 
		wcout << "Unable to open file";

	//wcout << L"!!!!!!!!_"<< fourChar[0] << L"_!!!!!!!!!!";

	std::wstring newTask = L"Global\\NewTask" + std::to_wstring(id);
	std::wstring Finish = L"Global\\Finish" + std::to_wstring(id);
	std::wstring Terminate = L"Global\\Terminate" + std::to_wstring(id);

	newTaskEvent = CreateEvent(nullptr, FALSE, FALSE, newTask.c_str());
	assert(newTaskEvent != nullptr);

	finishedTaskEvent = CreateEvent(nullptr, FALSE, FALSE, Finish.c_str());
	assert(finishedTaskEvent != nullptr);

	terminateEvent = CreateEvent(nullptr, TRUE, FALSE, Terminate.c_str());
	assert(terminateEvent != nullptr);

	std::wstring FileMapName = L"Global\\FileMap" + std::to_wstring(id);
	
	mappedFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, FileMapName.c_str());
	assert(mappedFile != NULL);
	PVOID mappedFileText = MapViewOfFile(mappedFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
	assert(mappedFileText != NULL);



	/*creatinonFObj = CreateFileMapping(DEFAULT_BUFFER_SIZE, NULL, PAGE_READWRITE, 0, 0, FileMap.c_str());
	assert(creatinonFObj != NULL);	

	//подключение объекта файлового отображения к адресному пространству:
	//получим начальный адрес данных объекта файлового отображения:
	LPVOID LPfileMapV = MapViewOfFile(creatinonFObj, FILE_MAP_WRITE, 0, 0, 0);
	assert(LPfileMapV != NULL);

	fileMapV = static_cast<char*>(LPfileMapV);
	//fileMap = OpenFileMapping(PAGE_READWRITE, FALSE, FileMap.c_str());
	// all rights except EXECUTE, the handle cannot be inherited by createprocess(), имя объекта файлового отображения

	//assert(fileMap != nullptr);*/
}

CWorker::~CWorker()
{
	CloseHandle(newTaskEvent);
	CloseHandle(finishedTaskEvent);
	CloseHandle(terminateEvent);

	UnmapViewOfFile(creatinonFObj);
	CloseHandle(fileMapV);
}

void CWorker::Work()
{
	while (true) {
		/*std::vector<HANDLE> eventsToWait{ terminateEvent, newTaskEvent };
		auto waitStatus = WaitForMultipleObjects(eventsToWait.size(), eventsToWait.data(), FALSE, INFINITE);
		switch (waitStatus) {
		case WAIT_OBJECT_0 + 0: // terminate event
			return;
		case WAIT_OBJECT_0 + 1: // new task event
		{
			// replacing target words with empty string
			char* filteredViewEnd = std::regex_replace(fileView,
				fileView,
				fileView + std::strlen(fileView),
				targetWords,
				" ");

			*filteredViewEnd = '\0';

			SetEvent(finishedTaskEvent);*/
	cout << "~~~~~~~~~~~~~~~~~\n";
	break;
		//}
		//default:
		//	assert(false);
		//}
	}
}
