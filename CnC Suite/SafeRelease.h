#pragma once
#ifndef _SAFE_RELEASE_H_


// Safe release for class-interfaces
template <class A> inline void SafeRelease(A **ppInterface)
{
	if (*ppInterface != nullptr)
	{
		(*ppInterface)->Release();
		*ppInterface = nullptr;
	}
}
// Safe deletition for pointer allocated with 'new'
template <class B> inline void SafeDelete(B **ppToDelete)
{
	//__try
	//{
		if (*ppToDelete != nullptr)
		{
			delete (*ppToDelete);
			(*ppToDelete) = nullptr;
		}
	//}
	//__except (
	//	GetExceptionCode() == EXCEPTION_BREAKPOINT
	//	? EXCEPTION_EXECUTE_HANDLER
	//	: EXCEPTION_CONTINUE_SEARCH)
	//{
	//	return;
	//}
}

// Safe deletition for pointer allocated with 'new[  ]'
template <class C> inline void SafeDeleteArray(C **ppToDelete)
{
	//__try
	//{
		if (*ppToDelete != nullptr)
		{
			delete[](*ppToDelete);
			(*ppToDelete) = nullptr;
		}
	//}
	//__except (
	//	GetExceptionCode() == EXCEPTION_BREAKPOINT
	//	? EXCEPTION_EXECUTE_HANDLER
	//	: EXCEPTION_CONTINUE_SEARCH)
	//{
	//	return;
	//}
}

#endif // !_SAFE_RELEASE_H_

