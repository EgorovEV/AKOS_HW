#include <string>
#include <string>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <windows.h>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

std::wstring cleanTextInMappedFile(PVOID Map_file_text, int dictSize, vector<std::wstring> dict) {
	//wcout << L"________beginClean________" << text;
	std::wstring text(&((wchar_t*)Map_file_text)[1]);
	text.push_back(L' ');
	text.insert(0, L" ");
	for (int i = 0; i < dictSize; i++) {
		std::wstring pattern = L" " + std::wstring(dict[i]) + L" ";
		size_t pos = 0;
		while ((pos = text.find(pattern, pos)) != std::wstring::npos) {
			text.replace(pos, pattern.length(), L" ");
			pos += 1;
		}
	}
	string output;
	unique_copy(text.begin(), text.end(), back_insert_iterator<string>(output),
		[](char a, char b) { return isspace(a) && isspace(b);});
	wcout << L"________endClean________" << text;
	return text;
}

PVOID MapFile(wchar_t* filename, HANDLE& sourceFile, DWORD& sourceFileSize, HANDLE& sourceFileMap)//, HANDLE& onReadyForProcessingEvent)
{
	wcout << filename << L"<-filename\n";
	sourceFile = CreateFile(filename, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(sourceFile != NULL);
	sourceFileSize = GetFileSize(sourceFile, NULL);
	sourceFileMap = CreateFileMapping(sourceFile, NULL, PAGE_READWRITE, 0, sourceFileSize, NULL);
	PVOID sourceText = MapViewOfFile(sourceFileMap, FILE_MAP_READ, 0, 0, 0);
	//return value is the starting address of the mapped view.

	if (sourceText == NULL)
	{
		fprintf(stdout, "MapViewOfFile: Error %ld\n",
			GetLastError());
		return NULL;
	}
	/*std::wstring result(&((wchar_t*)sourceText)[1]);

	UnmapViewOfFile(sourceFileMap);
	CloseHandle(sourceFile);
	CloseHandle(sourceFileMap);*/

	//wcout << sourceText <<" "<< result << L'\n';
	//cout << "reeeding!!\n";
	/*if (!SetEvent(onReadyForProcessingEvent)) {
	reportError(GetLastError());
	return NULL;
	}*/

	return sourceText;
}

void writeToFile(wchar_t* filename, std::wstring strToWrite, HANDLE sourceFile, int shift) {
	//CloseHandle(sourceFile);
	//CloseHandle(sourceFileMap);
	//HANDLE sourceFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	//assert(sourceFile == NULL);
	DWORD dwBytesWritten = 0;
	BOOL bErrorFlag = WriteFile(
		sourceFile,
		strToWrite.c_str(),
		strToWrite.length() * sizeof(wchar_t),
		&dwBytesWritten,
		NULL);
	CloseHandle(sourceFile);
}

bool IsFileExist(LPCTSTR strFileName)
{
	HANDLE hFile = ::CreateFile(strFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);
	BOOL ans = (hFile != INVALID_HANDLE_VALUE);
	CloseHandle(hFile);
	return ans;
}


vector<HANDLE> newTaskEvents(4);
std::vector<HANDLE> fileMaps(4);
std::vector<char*> fileViews(4);
std::vector<PROCESS_INFORMATION> processInfos(4);
const std::wstring workerExeFilename = L"Worker.exe";
std::wstring workerCommandLine = workerExeFilename + L"bad_w";
int numWorkers = 2;
std::vector<HANDLE> finishedTaskEvents(4);

HANDLE sourceFile;
DWORD sourceFileSize;
HANDLE sourceFileMap;
HANDLE terminateEvent;

//run_proc -- создает события и процессы
void run_proc() {
	DWORD processID = GetCurrentProcessId();
	
	std::vector<std::string> myargv;
	myargv.resize(3);
	myargv[1] = "in.txt";
	myargv[2] = "16";

	std::vector<PVOID> mappedFilesText(1);

	//имена должны совпадать с именами в Worker.exe
	//std::wstring Ready_to_proccesing_ev = (std::wstring(L"Global\\Ready") + std::to_wstring(processID));
	//LPCTSTR Close_evName = L"Global\\Close";
	//std::wstring mappedFileName = (std::wstring(L"text.txt") + std::to_wstring(processID));
	//std::wstring onTaskIsDoneEventName = (std::wstring(L"Global\\Done") + std::to_wstring(processID));
	//std::cout << Ready_to_proccesing_ev << ""

	terminateEvent = CreateEvent(nullptr, TRUE, FALSE, L"Global\\TerminateEvent");
	assert(terminateEvent != nullptr);

	for (int i = 0; i < numWorkers; ++i) {
		newTaskEvents[i] = CreateEvent(NULL, FALSE, FALSE,
			(std::wstring(L"Global\\Ready") + std::to_wstring(i)).c_str());
		assert(newTaskEvents[i] != NULL);


		finishedTaskEvents[i] = CreateEvent(NULL, TRUE, FALSE,
			(std::wstring(L"Global\\Done") + std::to_wstring(i)).c_str());
		assert(finishedTaskEvents[i] != NULL);

		//fileMaps[i] = MapFile(L"text.txt", sourceFile, sourceFileSize, sourceFileMap);

		/*fileMaps[i] = CreateFileMapping(INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, 40960,
			(std::wstring(L"Global\\TFTempFileMapping") + std::to_wstring(i)).c_str());
		cout << GetLastError() << '\n';
		assert(fileMaps[i] != nullptr);

		fileViews[i] = static_cast<char*>(MapViewOfFile(fileMaps[i], FILE_MAP_WRITE, 0, 0, 0));
		assert(fileViews[i] != nullptr);*/

		STARTUPINFO startupInfo;
		ZeroMemory(&startupInfo, sizeof(startupInfo));
		startupInfo.cb = sizeof(startupInfo);
		ZeroMemory(&processInfos[i], sizeof(processInfos[i]));

		bool createProcStatus = CreateProcess(workerExeFilename.c_str(),
			const_cast<LPWSTR>((workerCommandLine + std::to_wstring(i)).c_str()),
			nullptr,
			nullptr,
			FALSE,
			CREATE_DEFAULT_ERROR_MODE,
			nullptr,
			nullptr,
			&startupInfo,
			&processInfos[i]);
		if (!createProcStatus) {
			printf("CreateProcess failed (%d).\n", GetLastError());
			return;
		}
	}
}

//filter_text -- распределяет обязанности для процессов worker.exe - кому какую часть парсить.
//файл считается и отфильтруется Worker'ами.
//ждет, пока всё распарситься
//собираюся в 1 файлы тоже в воркере.

int filter_text() {
	//vector<std::wstring> bad_words(2);
	//bad_words[0] = L"uno";
	//bad_words[1] = L"dos";

	/*if (IsFileExist(L"text.txt")) {
	//сигнал о готовности
	if (!SetEvent(onReadyForProcessingEvent)) {
	return NULL;
	}
	}
	else
	return 3;*/

	const std::wstring myfilename = L"mytext.txt";
	HANDLE MyFile = CreateFile(
		myfilename.c_str(),
		GENERIC_ALL,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	size_t lenF = GetFileSize(MyFile, NULL);
	size_t part_len = lenF / 4;

	vector<std::wstring> cleanText(numWorkers);

	for (int i = 0; i < numWorkers; ++i) {
		//cleanText[i] = cleanTextInMappedFile(fileMaps[i], 2, bad_words);	//todo +добавить смещение
		//оно там само будет работать...
		SetEvent(newTaskEvents[i]);
	}
	Sleep(1000);
	//DWORD catchedEvent = WaitForMultipleObjects(numWorkers, newTaskEvents.data(), TRUE, INFINITE);
	/*switch (catchedEvent) {
	case WAIT_FAILED:
		// неправильный вызов функции (неверный описатель?)
		break;

	case WAIT_TIMEOUT:
		// ни один из объектов не освободился в течение 5000 мс -> infinite
		break;
	}*/
	cout << "already...\n";
	DWORD catchedEventFin = WaitForMultipleObjects(numWorkers, finishedTaskEvents.data(), TRUE, INFINITE);
	cout << "~~~ALL!!!~~~\n";
	return 0;
}

void end_proc() {
	for (int i = 0; i < 2; ++i) {
		CloseHandle(processInfos[i].hProcess);
		CloseHandle(processInfos[i].hThread);

		CloseHandle(newTaskEvents[i]);
		CloseHandle(finishedTaskEvents[i]);
		CloseHandle(fileMaps[i]);
	}

	SetEvent(terminateEvent);
	CloseHandle(terminateEvent);
}


int main(int argc, char** argv)
{
	//Запускать от имени администратора
	run_proc();
	filter_text();
	//end_proc();
	Sleep(1000);
	return 0;
}