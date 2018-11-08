#include "Async.h"


ULONG_PTR InterlockedRead(ULONG_PTR* value) {

	return InterlockedCompareExchange(value, *value, 0);
}


void exchangeFunction(LPVOID data) {

	auto fData = reinterpret_cast<LPDUALVALUE>(data);
	if (fData != nullptr)
	{
		InterlockedExchange((ULONG_PTR*)fData->valueOne, fData->valueTwo);

		delete fData;
	}
}

void InterlockedExchangeDelayed(ULONG_PTR* destination, ULONG_PTR value, DWORD milliseconds) {

	auto async = new Async();
	if (async != nullptr)
	{
		auto fData = new DUALVALUE;
		if (fData != nullptr)
		{
			fData->valueOne = (ULONG_PTR)destination;
			fData->valueTwo = value;

			async->setDelay(milliseconds);
			async->setFunctionData((LPVOID)fData);
			async->callFunction(&exchangeFunction);
		}
	}
}
