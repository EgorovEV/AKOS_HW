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
	int shift = id * (sourceFileSize/2);
	//cout << "shift=" << shift << '\n';
	std::wstring text(&((wchar_t*)Map_file_text + shift)[1]);
	wcout << L"________beginClean________" << text;
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

PVOID MapFile(wchar_t* filename, HANDLE& sourceFile, DWORD sourceFileSize, HANDLE& sourceFileMap)//, HANDLE& onReadyForProcessingEvent)
{
	//wcout << filename << L"<-filename\n";
	sourceFile = CreateFile(filename, GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	assert(sourceFile != NULL);
	sourceFileSize = GetFileSize(sourceFile, NULL);
	cout << "len^" << sourceFileSize << '\n';
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




HANDLE sourceFile;
//DWORD sourceFileSize;
HANDLE sourceFileMap;
HANDLE terminateEvent;


int main(int argc, char** argv) {
	const wstring filenm = L"C:\\Users\\евгений\\Documents\\Visual Studio 2015\\Projects\\Text_processing\\Worker\\text.txt";
	//cout << "File Exist?" << IsFileExist(filenm.c_str()) << '\n';
	//cout << "File Exist?" << IsFileExist(L"mytext.txt") << '\n';

	cout << "!!!--" << argv[0][15] << '\n';
	//DWORD processID = GetCurrentProcessId();

	DWORD processID = argv[0][15];
	processID -= 48;

	std::vector<std::string> myargv;
	myargv.resize(3);
	myargv[1] = "in.txt";
	myargv[2] = "16";

	//std::vector<PVOID> mappedFilesText(1);

	std::wstring Ready_to_proccesing_ev = (std::wstring(L"Global\\Ready") + std::to_wstring(processID));
	LPCTSTR Close_evName = L"Global\\Close";
	std::wstring onTaskIsDoneEventName = (std::wstring(L"Global\\Done") + std::to_wstring(processID));


	HANDLE onReadyForProcessingEvent = CreateEvent(NULL, FALSE, FALSE, Ready_to_proccesing_ev.c_str());
	assert(onReadyForProcessingEvent != NULL);


	HANDLE Close_ev = CreateEvent(NULL, TRUE, FALSE, Close_evName);
	assert(Close_ev != NULL);

	HANDLE onTaskIsDoneEvent = CreateEvent(NULL, FALSE, FALSE, onTaskIsDoneEventName.c_str());
	assert(onTaskIsDoneEvent != NULL);

	// creating file mappings 
	//fileMaps[i] = MapFile(L"text.txt", sourceFile, sourceFileSize, sourceFileMap);
	//assert(fileMaps[i] != nullptr);

	vector<std::wstring> bad_words(2);
	bad_words[0] = L"ms";
	bad_words[1] = L"dos";

	HANDLE events[2];
	events[0] = onReadyForProcessingEvent;
	events[1] = Close_ev;


	/*if (IsFileExist(L"text.txt")) {
		//сигнал о готовности
		if (!SetEvent(onReadyForProcessingEvent)) {
			return NULL;
		}
	}
	else
		return 3;*/
	cout << "proc number == " << processID << '\n';
	DWORD catchedEvent = WaitForMultipleObjects(2, events, FALSE, INFINITE);
	cout << "__________PASS_WAIT__________\n";
	switch (catchedEvent) {
	case WAIT_FAILED:
		// неправильный вызов функции (неверный описатель?)
		break;

	case WAIT_TIMEOUT:
		// ни один из объектов не освободился в течение 5000 мс -> infinite
		break;
	case WAIT_OBJECT_0 + 0:
	{
		//// завершился процесс, идентифицируемый events[0]
		//PVOID fileText = MapViewOfFile(mappedFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
		//std::wstring input_text = readFile(L"text.txt", sourceFile, sourceFileSize, sourceFileMap);
		
		PVOID mappedFile = MapFile(L"C:\\Users\\евгений\\Documents\\Visual Studio 2015\\Projects\\Text_processing\\Worker\\text.txt", sourceFile, sourceFileSize, sourceFileMap);

		

		wstring cleanText = cleanTextInMappedFile(mappedFile, 2, bad_words, processID);
		wcout << cleanText << L"<-- cleanText\n";
		writeToFile(L"text.txt", cleanText, sourceFile, 0);

		UnmapViewOfFile(mappedFile);

		UnmapViewOfFile(sourceFileMap);
		CloseHandle(sourceFile);
		CloseHandle(sourceFileMap);
		cout << "qwjrwh\n";
		BOOL res = SetEvent(onTaskIsDoneEvent);
		if (!res) {
			return 1;
		}
		cout << "All right!!!\n";
		int wait_please;
		cin >> wait_please;
		return 0;
	}

	case WAIT_OBJECT_0 + 1:
		return 0;
	}


	return 0;
}
