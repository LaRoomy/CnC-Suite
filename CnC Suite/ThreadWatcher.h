#pragma once
#include<Windows.h>
#include<TlHelp32.h>

#define			THREADWATCHER_ALL_THREADS_FINISHED	1
#define			THREADWATCHER_WAIT_TIMEOUT			-1
#define			THREADWATCHER_ERROR_NO_THREADS		-2
#define			THREADWATCHER_ERROR_INITIAL_FAIL	-3
#define			THREADWATCHER_ERROR_UNKNOWN			-4

class ThreadWatcher
{
public:
	ThreadWatcher(DWORD processID)
		:listLenght(0),
		threadList(nullptr),
		_processID(processID)
	{
		this->success =
			this->GetIdentifierListOfActiveThreadsForProcess(processID, &this->threadList);
		if (success)
			this->listLenght = (int)this->success;
	}

	~ThreadWatcher()
	{
		if (this->threadList != nullptr)
		{
			delete[] this->threadList;
		}
	}

	void Release() { delete this; }

	bool isValid() {
		return this->success ? true : false;
	}

	void Reset()
	{
		this->eraseList(nullptr);

		this->success =
			this->GetIdentifierListOfActiveThreadsForProcess(this->_processID, &this->threadList);
		if (success)
			this->listLenght = (int)this->success;
	}

	BOOL wait(DWORD timeout_millisec)
	{
		if (!this->success)
			return THREADWATCHER_ERROR_INITIAL_FAIL;

		DWORD time = 0;
		DWORD* compareList = nullptr;

		while (1)
		{
			if(compareList != nullptr)
				this->eraseList(&compareList);

			BOOL res = this->GetIdentifierListOfActiveThreadsForProcess(this->_processID, &compareList);
			if (res)
			{
				auto compRes = this->threadIdListComparison(compareList, (int)res);
				if (compRes)
				{
					this->eraseList(&compareList);
					return THREADWATCHER_ALL_THREADS_FINISHED;
				}
			}
			else
				return THREADWATCHER_ERROR_NO_THREADS;

			Sleep(10);
			time += 10;
			if (time > timeout_millisec)
			{
				this->eraseList(&compareList);

				return THREADWATCHER_WAIT_TIMEOUT;
			}
		}
		return THREADWATCHER_ERROR_UNKNOWN;
	}

private:
	int listLenght;
	DWORD* threadList;
	BOOL success;
	DWORD _processID;

	BOOL GetIdentifierListOfActiveThreadsForProcess(_In_ DWORD processID, _Inout_ DWORD** list_out)
	{
		if (*list_out != nullptr)
			return FALSE;
		else
		{
			BOOL result = TRUE;
			HANDLE threadSnapshot = INVALID_HANDLE_VALUE;

			THREADENTRY32 te32;
			te32.dwSize = sizeof(THREADENTRY32);

			threadSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			if (threadSnapshot != INVALID_HANDLE_VALUE)
			{
				int cnt = 0;

				for (int i = 0; i < 2; i++)
				{
					if (i == 1)
					{
						if (cnt > 0)
						{
							*list_out = new DWORD[cnt];
							if (*list_out == nullptr)
							{
								result = FALSE;
								break;
							}
							result = cnt;
							cnt = 0;
						}
						else
						{
							result = FALSE;
							break;
						}
					}

					if (!Thread32First(threadSnapshot, &te32))
					{
						result = FALSE;
						break;
					}
					else
					{
						do
						{
							if (te32.th32OwnerProcessID == processID)
							{
								if (i == 0)
								{
									cnt++;
								}
								else
								{
									(*list_out)[cnt] = te32.th32ThreadID;
									cnt++;
								}
							}

						} while (Thread32Next(threadSnapshot, &te32));
					}
				}
				CloseHandle(threadSnapshot);
			}
			return result;
		}
	}

	bool threadIdListComparison(DWORD* idList_new, int newListLength) {

		if (newListLength > this->listLenght)
			return false;
		else if (this->listLenght > newListLength)
			return true;
		else
		{
			for (int i = 0; i < this->listLenght; i++)
			{
				if (this->threadList[i] != idList_new[i])
					return false;
			}
			return true;
		}
	}

	void eraseList(DWORD** list) {

		if (*list != nullptr)
		{
			delete[] *list;
			*list = nullptr;
		}
		else
		{
			delete[] this->threadList;
			this->threadList = nullptr;
		}
	}
};