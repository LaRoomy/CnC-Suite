#pragma once

#ifndef _SAFE_RELEASE_H_
#define _SAFE_RELEASE_H_

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
	if (*ppToDelete != nullptr)
	{
		delete (*ppToDelete);
		(*ppToDelete) = nullptr;
	}
}

// Safe deletition for pointer allocated with 'new[  ]'
template <class C> inline void SafeDeleteArray(C **ppToDelete)
{
	if (*ppToDelete != nullptr)
	{
		delete[](*ppToDelete);
		(*ppToDelete) = nullptr;
	}
}

#endif // !_SAFE_RELEASE_H_

