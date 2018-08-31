#pragma once
#include"external.h"

typedef void(*async_smpl)();
typedef void(*async_parameter)(LPVOID);

typedef LPVOID FUNCTION_PTR;

class Async
	: public ClsObject<Async>
{
public:
	Async()
		:delay(0), functionData(nullptr), funcPtr(nullptr)
	{}

	// do not use the object after the invokation of this method!
	void callFunction(FUNCTION_PTR _function_)
	{
		if (_function_ != nullptr)
		{
			DWORD threadId;
			HANDLE hThread;

			this->funcPtr = _function_;

			hThread = CreateThread(nullptr, 0, Async::asyncProc, reinterpret_cast<LPVOID>(this), 0, &threadId);
			if (hThread)
			{
				WaitForSingleObject(hThread, 20);
				CloseHandle(hThread);
			}
		}
	}
	// do not use the object after the invokation of this method!
	void callFunction(FUNCTION_PTR _function_, LPVOID data)
	{
		this->functionData = data;
		this->callFunction(_function_);
	}

	void setDelay(DWORD _delay)
	{
		this->delay = _delay;
	}

	void setFunctionData(LPVOID data) {
		this->functionData = data;
	}

	const wchar_t* ToString() {
		return L"async-class";
	}
	void FromString(const wchar_t* stringRepresentation) {
		UNREFERENCED_PARAMETER(stringRepresentation);
	}

private:
	DWORD delay;
	LPVOID functionData;
	FUNCTION_PTR funcPtr;

	static DWORD __stdcall asyncProc(LPVOID lParam)
	{
		__try
		{
			auto _this = reinterpret_cast<Async*>(lParam);
			if (_this != nullptr)
			{
				if (_this->functionData != nullptr)
				{
					auto _function = (async_parameter)_this->funcPtr;
					if (_function != nullptr)
					{
						if (_this->delay != 0)
						{
							Sleep(_this->delay);
						}
						(*_function)(_this->functionData);
					}
				}
				else
				{
					auto _function = (async_smpl)_this->funcPtr;
					if (_function != nullptr)
					{
						if (_this->delay != 0)
						{
							Sleep(_this->delay);
						}
						(*_function)();
					}
				}
				_this->Release();
				return 1001;
			}		
		}
		__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
		{
			return 1007;
		}
		return 0;
	}
};

template<class F> class AsyncOperation
{
public:
	AsyncOperation(){}
	~AsyncOperation(){}

	void Execute(F method) {
		((async_smpl)method)();
	}
};
