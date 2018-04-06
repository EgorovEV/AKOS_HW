#include <string>
#include <iterator>
#include <algorithm>
#include <cassert>
#include <windows.h>
#include <iostream>
#include <vector>

using namespace std;
DWORD sourceFileSize;

std::wstring cleanTextInMappedFile(PVOID Map_file_text, int dictSize, vector<std::wstring> dict, int id) {
	std::wstring tmptext(&((wchar_t*)Map_file_text)[1]);
	int text_len = tmptext.length();
	int shift = id * text_len / 2;
	
	std::wstring text = tmptext.substr(shift, text_len / 2);
	wcout << "text_to_parce:" << text << "\n";
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
	wcout << L"Parsed text" << text << L"\n\n";
	return text;
}

PVOID MapFile(wchar_t* filename, HANDLE& sourceFile, DWORD sourceFileSize, HANDLE& sourceFileMap)//, HANDLE& onReadyForProcessingEvent)
{
	sourceFile = CreateFile(filename, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(sourceFile != NULL);
	sourceFileSize = GetFileSize(sourceFile, NULL);
	cout << "len^" << sourceFileSize << '\n';
	sourceFileMap = CreateFileMapping(sourceFile, NULL, PAGE_READWRITE, 0, sourceFileSize, NULL);
	PVOID sourceText = MapViewOfFile(sourceFileMap, FILE_MAP_READ, 0, 0, 0);	

	if (sourceText == NULL)
	{
		fprintf(stdout, "MapViewOfFile: Error %ld\n",
			GetLastError());
		return NULL;
	}
	return sourceText;
}

void writeToFile(std::wstring strToWrite, wstring filename) {
	DWORD dwBytesWritten = 0;

	HANDLE outputFile = CreateFile(
		filename.c_str(),
		FILE_APPEND_DATA,
		0,
		nullptr,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	BOOL bErrorFlag = WriteFile(
		outputFile,
		strToWrite.c_str(),
		strToWrite.length() * sizeof(wchar_t),
		&dwBytesWritten,
		NULL);
	CloseHandle(outputFile);
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




HANDLE sourceFile;
HANDLE sourceFileMap;
HANDLE terminateEvent;

int wmain(int argc, wchar_t** argv) {
	const wstring filenm = L"C:\\Users\\åâãåíèé\\Documents\\Visual Studio 2015\\Projects\\Text_processing\\Worker\\text.txt";
	int processID = argv[0][15];
	processID -= 48;
	wcout << "In proc: " << processID << L"\n";
	std::vector<std::string> myargv;

	std::wstring Ready_to_proccesing_ev = (std::wstring(L"Global\\Ready") + std::to_wstring(processID));
	LPCTSTR Close_evName = L"Global\\Close";
	std::wstring onTaskIsDoneEventName = (std::wstring(L"Global\\Done") + std::to_wstring(processID));


	HANDLE onReadyForProcessingEvent = CreateEvent(NULL, FALSE, FALSE, Ready_to_proccesing_ev.c_str());
	assert(onReadyForProcessingEvent != NULL);


	HANDLE Close_ev = CreateEvent(NULL, TRUE, FALSE, Close_evName);
	assert(Close_ev != NULL);

	HANDLE onTaskIsDoneEvent = CreateEvent(NULL, FALSE, FALSE, onTaskIsDoneEventName.c_str());
	assert(onTaskIsDoneEvent != NULL);

	vector<std::wstring> bad_words(2);
	bad_words[0] = L"ms";
	bad_words[1] = L"dos";

	HANDLE events[2];
	events[0] = onReadyForProcessingEvent;
	events[1] = Close_ev;


	DWORD catchedEvent = WaitForMultipleObjects(2, events, FALSE, INFINITE);
	switch (catchedEvent) {
	case WAIT_FAILED:
		// íåïðàâèëüíûé âûçîâ ôóíêöèè (íåâåðíûé îïèñàòåëü?)
		break;

	case WAIT_TIMEOUT:
		// íè îäèí èç îáúåêòîâ íå îñâîáîäèëñÿ â òå÷åíèå 5000 ìñ -> infinite
		break;
	case WAIT_OBJECT_0 + 0:
	{
		PVOID mappedFile = MapFile(L"..\\Worker\\text.txt", sourceFile, sourceFileSize, sourceFileMap);
		wstring cleanText = cleanTextInMappedFile(mappedFile, 2, bad_words, processID);
		wstring output = to_wstring(processID) + L".txt";
		writeToFile(cleanText, output);

		UnmapViewOfFile(mappedFile);

		UnmapViewOfFile(sourceFileMap);
		CloseHandle(sourceFile);
		CloseHandle(sourceFileMap);
		BOOL res = SetEvent(onTaskIsDoneEvent);
		if (!res) {
			return 1;
		}
		int wait_please;
		return 0;
	}

	case WAIT_OBJECT_0 + 1:
		return 0;
	}
	return 0;
}
