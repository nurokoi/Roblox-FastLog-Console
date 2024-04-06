// SPDX-License-Identifier: MIT

/*
MIT License

Copyright (c) 2024 nurokoi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>

struct dbwin_buffer {
    DWORD dwProcessId;
    char data[4096 - sizeof(DWORD)];
};

std::mutex processMutex;
std::vector<DWORD> globalPIDs;

std::string FormatTimestamp(const std::string& isoTimestamp) {
    std::tm tm = {};
    std::istringstream ss(isoTimestamp.substr(0, 19));
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

    if (ss.fail()) {
        return "Malformed timestamp";
    }

    std::time_t time = _mkgmtime(&tm);
    std::tm* localTime = std::localtime(&time);

    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    return std::string(buffer) + isoTimestamp.substr(19, 4);
}

void FLog(const char* logMessage) {
    std::string logMsg(logMessage);
    size_t pos1 = logMsg.find(',');
    size_t pos2 = logMsg.find(',', pos1 + 1);
    size_t pos3 = logMsg.find(',', pos2 + 1);

    if (pos1 != std::string::npos && pos2 != std::string::npos && pos3 != std::string::npos) {
        std::string timestamp = FormatTimestamp(logMsg.substr(0, pos1));
        if (!timestamp.empty()) {
            std::string threadId = logMsg.substr(pos2 + 1, pos3 - pos2 - 1);
            size_t levelStart = pos3 + 1;
            size_t levelEnd = logMsg.find(' ', levelStart);
            std::string level = logMsg.substr(levelStart, levelEnd - levelStart);
            std::string message = logMsg.substr(levelEnd + 1);
            printf("%s - Thread ID %s - Level %s - %s\n", timestamp.c_str(), threadId.c_str(), level.c_str(), message.c_str());
        }
    }
}

void UpdateProcesses(const std::vector<std::string>& processNames) {
    while (true) {
        std::vector<DWORD> newPIDs;
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 processEntry = {};
            processEntry.dwSize = sizeof(PROCESSENTRY32);

            if (Process32First(snapshot, &processEntry)) {
                do {
                    for (const auto& name : processNames) {
                        if (std::string(processEntry.szExeFile) == name) {
                            newPIDs.push_back(processEntry.th32ProcessID);
                        }
                    }
                } while (Process32Next(snapshot, &processEntry));
            }
            CloseHandle(snapshot);
        }
        {
            std::lock_guard<std::mutex> lock(processMutex);
            globalPIDs = newPIDs;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
}

void Configure() {
    HANDLE hDataReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, "DBWIN_DATA_READY");
    hDataReady = CreateEvent(NULL, FALSE, FALSE, "DBWIN_DATA_READY");
    
    HANDLE hBufferReady = OpenEvent(EVENT_ALL_ACCESS, FALSE, "DBWIN_BUFFER_READY");
    hBufferReady = CreateEvent(NULL, FALSE, FALSE, "DBWIN_BUFFER_READY");
    
    HANDLE hFileMapping = OpenFileMapping(FILE_MAP_READ, FALSE, "DBWIN_BUFFER");
    hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(dbwin_buffer), "DBWIN_BUFFER");
    
    auto buffer = static_cast<dbwin_buffer*>(MapViewOfFile(hFileMapping, SECTION_MAP_READ, 0, 0, 0));

    printf("Debug output listener running...\n");

    while (true) {
        SetEvent(hBufferReady);
        WaitForSingleObject(hDataReady, INFINITE);
        
        std::lock_guard<std::mutex> lock(processMutex);
        if (std::find(globalPIDs.begin(), globalPIDs.end(), buffer->dwProcessId) != globalPIDs.end()) {
            FLog(buffer->data);
        }
    }

    UnmapViewOfFile(buffer);
    CloseHandle(hFileMapping);
    CloseHandle(hBufferReady);
    CloseHandle(hDataReady);
}

int main(int argc, char* argv[]) {
    std::vector<std::string> processNames;
    for (int i = 1; i < argc; ++i) {
        processNames.push_back(argv[i]);
    }

    std::thread updateListThread(UpdateProcesses, processNames);
    updateListThread.detach();

    Configure();
    return 0;
}