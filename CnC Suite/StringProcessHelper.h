#pragma once
#ifndef _STRING_PROCESS_HELPER_H_
#define _STRING_PROCESS_HELPER_H_

#include<Windows.h>

typedef WCHAR* STRVECT;	// All pointer from type STRVECT must be released with 'ReleaseStringVect(p)' but are also compatible with WCHAR*

// STRVECT memory blocks must be released with this function
extern "C" void ReleaseStringVect32(STRVECT buffer);
extern "C" void ReleaseStringVect64(STRVECT buffer);


// This function returns the amount of chars in the buffer - including the terminating null character
extern "C" int GetStringLengthW32(LPCWSTR buffer);
extern "C" int GetStringLengthW64(LPCWSTR buffer);



/* This function compares two strings and returns 1 if the strings are equal, 0 if not.
// A negative return value indicates an invalid parameter. Both strings must be zero-terminated!*/
extern "C" int CompareStringsAsmW32(LPCWSTR string1, LPCWSTR string2);
extern "C" int CompareStringsAsmW64(LPCWSTR string1, LPCWSTR string2);


/* This function can be used for a successive retrieval of the single lines of a textbuffer.

If there is more than one line-end character(cr/lf), the function returns a string-code for every redundant character. "__cr" or "__lf"
But, only the redundand line-end char will be returned. If the format is 'cr/lf', cr will not be returned for lines with content!
But, if this is an empty line, both characters (cr + lf) will be returned!
This is necessary to savely detect empty lines and to conditionally override the lineend-format (lf,cr,cr/lf)

NOTE:	
			The return value is the address to the memory-block containing the line data(if there is data) or nullptr if not.
			The allocated memory-block must be released with 'ReleaseStringVect(...)'.
			The 'StartIndex' Parameter contains the beginning of the next line after a succeeded call or an errorcode.
			Errorcodes: (-1 == no data) (-2 == end of buffer) (-3 == invalid parameter)
 
IMPORTANT:
			Make sure the StartIndex is not outside of the array-boundaries by calling 'GetStringLenghtW' first.
			Otherwise it will cause an access-violation. 'buffer' must be zero-terminated*/
extern "C" STRVECT GetNextLineFromStartIndexW32(int* StartIndex, LPCWSTR buffer);
extern "C" STRVECT GetNextLineFromStartIndexW64(int* StartIndex, LPCWSTR buffer);


/* This function replaces the fields in a buffer successive from the given index.

NOTE:
			'appendix' must be zero-terminated !
			The function returns the copied elements if successful, or a negative value to indicate an error.
			(-1 == error invalid parameter) 

IMPORTANT:
			The function does not check if the end of the block from the beginning of the index is inside the boundaries of the array or
			if the given index is inside the boundaries - so for security reasons a check must be performed before calling this function*/
extern "C" int SetBlockInBufferFromIndexW32(LPWSTR buffer, LPCWSTR appendix, int index);
extern "C" int SetBlockInBufferFromIndexW64(LPWSTR buffer, LPCWSTR appendix, int index);


/* This function retrieves the fields in a buffer from the start-index to the end-index

NOTE:
		The targetBuffer must be large enough to take the segment (including the terminating null character), otherwise the function will crash
		If the function fails, the return-value is negative
IMPORTANT:
		The function does not check if the indexes are in the scope of the sourceBuffer, so make sure to check it before calling this function*/
extern "C" int GetBlockSegmentOutOfBufferW32(LPCWSTR sourceBuffer, LPWSTR targetBuffer, int startIndex, int endIndex);
extern "C" int GetBlockSegmentOutOfBufferW64(LPCWSTR sourceBuffer, LPWSTR targetBuffer, int startIndex, int endIndex);

/* This function inserts a wide-character at the specified index and re-arranges the buffer

NOTE:
		The newBuffer must be one field larger than the oldBuffer, otherwise this function will crash
		The oldBuffer must be zero-terminated.
		If the function fails, the return-value is negative (-1 == error invalid parameter)*/
extern "C" int InsertCharacterInBufferW32(LPCWSTR oldBuffer, LPWSTR newBuffer, WCHAR c, int charIndex);
extern "C" int InsertCharacterInBufferW64(LPCWSTR oldBuffer, LPWSTR newBuffer, WCHAR c, int charIndex);


/* This function removes a wide-character at the specified index an re-arranges the buffer

NOTE:
		oldBuffer must be zero-terminated
		newBuffer must be [sizeof(oldBuffer) - 1] or larger (optimum is one field smaller)
		If the function fails the return-value is negative (-1 == error invalid parameter)*/
extern "C" int RemoveCharacterFromBufferW32(LPCWSTR oldBuffer, LPWSTR newBuffer, int charIndex);
extern "C" int RemoveCharacterFromBufferW64(LPCWSTR oldBuffer, LPWSTR newBuffer, int charIndex);

/* This function inserts a wide-char-block at the specified Index and stores the new string in a new buffer

NOTE:
		The targetBuffer must have the size of the oldBuffer + the bufferToInsert + 1 for the terminating null character.
		The oldBuffer and the bufferToInsert must be zero terminated.
		If the function fails the return value is negative. (-1 == invalid parameter)*/
extern "C" int InsertBlockInBufferW32(LPWSTR targetBuffer, LPCWSTR oldBuffer, LPCWSTR bufferToInsert, int index);
extern "C" int InsertBlockInBufferW64(LPWSTR targetBuffer, LPCWSTR oldBuffer, LPCWSTR bufferToInsert, int index);

/* This function removes a wide-char-block from the specified startindex to the endindex and stores the data in a new buffer

NOTE:
		The targetBuffer must have the size of the oldBuffer minus the defined segment + 1 for the terminating null character
		The oldBuffer must be zero-terminated.
		If the function fails the return value is negative. (-1 == invalid parameter)*/
extern "C" int RemoveBlockFromBufferW32(LPWSTR targetBuffer, LPCWSTR oldBuffer, int startIndex, int endIndex);
extern "C" int RemoveBlockFromBufferW64(LPWSTR targetBuffer, LPCWSTR oldBuffer, int startIndex, int endIndex);


#if defined _M_IX86
// STRVECT memory blocks must be released with this function
#define ReleaseStringVect			ReleaseStringVect32

// This function returns the amount of chars in the buffer - including the terminating null character
#define	GetStringLengthW			GetStringLengthW32

/* This function compares two strings and returns 1 if the strings are equal, 0 if not.
A negative return value indicates an invalid parameter. Both strings must be zero-terminated!*/
#define	CompareStringsAsmW			CompareStringsAsmW32

/* This function can be used for a successive retrieval of the single lines of a textbuffer.

If there is more than one line-end character(cr/lf), the function returns a string-code for every redundant character. "__cr" or "__lf"
But, only the redundand line-end char will be returned. If the format is 'cr/lf', cr will not be returned for lines with content!
But, if this is an empty line, both characters (cr + lf) will be returned!
This is necessary to savely detect empty lines and to conditionally override the lineend-format (lf,cr,cr/lf)
 
NOTE:		The return value is the address to the memory-block containing the line data(if there is data) or nullptr if not.
			The allocated memory-block must be released with 'ReleaseStringVect(...)'.
			The 'StartIndex' Parameter contains the beginning of the next line after a succeeded call or an errorcode.
			Errorcodes: (-1 == no data) (-2 == end of buffer) (-3 == invalid parameter)
 
IMPORTANT:
			Make sure the StartIndex is not outside of the array-boundaries by calling 'GetStringLenghtW' first.
			Otherwise it will cause an access-violation. 'buffer' must be zero-terminated*/
#define	GetNextLineFromStartIndexW	GetNextLineFromStartIndexW32

/* This function replaces the fields in a buffer successive from the given index.

NOTE:
			'appendix' must be zero-terminated !
			The function returns the copied elements if successful, or a negative value to indicate an error.
			(-1 == error invalid parameter) 

IMPORTANT:
			The function does not check if the end of the block from the beginning of the index is inside the boundaries of the array or
			if the given index is inside the boundaries - so for security reasons a check must be performed before calling this function*/
#define	SetBlockInBufferFromIndexW	SetBlockInBufferFromIndexW32

/* This function retrieves the fields in a buffer from the start-index to the end-index

NOTE:
The targetBuffer must be large enough to take the segment (including the terminating null character), otherwise the function will crash
If the function fails, the return-value is negative
IMPORTANT:
The function does not check if the indexes are in the scope of the sourceBuffer, so make sure to check it before calling this function*/
#define	GetBlockSegmentOutOfBufferW	GetBlockSegmentOutOfBufferW32

/* This function inserts a wide-character at the specified index and re-arranges the buffer

NOTE:
The newBuffer must be one field larger than the oldBuffer, otherwise this function will crash
The oldBuffer must be zero-terminated.
If the function fails, the return-value is negative (-1 == error invalid parameter)*/
#define	InsertCharacterInBufferW	InsertCharacterInBufferW32

/* This function removes a wide-character at the specified index an re-arranges the buffer

NOTE:
oldBuffer must be zero-terminated
newBuffer must be [sizeof(oldBuffer) - 1] or larger (optimum is one field smaller)
If the function fails the return-value is negative (-1 == error invalid parameter)*/
#define	RemoveCharacterFromBufferW	RemoveCharacterFromBufferW32

/* This function inserts a wide-char-block at the specified Index and stores the new string in a new buffer

NOTE:
The targetBuffer must have the size of the oldBuffer + the bufferToInsert + 1 for the terminating null character.
The oldBuffer and the bufferToInsert must be zero terminated.
If the function fails the return value is negative. (-1 == invalid parameter)*/
#define	InsertBlockInBufferW		InsertBlockInBufferW32

/* This function removes a wide-char-block from the specified startindex to the endindex and stores the data in a new buffer

NOTE:
The targetBuffer must have the size of the oldBuffer minus the defined segment + 1 for the terminating null character
The oldBuffer must be zero-terminated.
If the function fails the return value is negative. (-1 == invalid parameter)*/
#define	RemoveBlockFromBufferW		RemoveBlockFromBufferW32
#else

// STRVECT memory blocks must be released with this function
#define	ReleaseStringVect			ReleaseStringVect64

// This function returns the amount of chars in the buffer - including the terminating null character
#define	GetStringLengthW			GetStringLengthW64

/*This function compares two strings and returns 1 if the strings are equal, 0 if not.
A negative return value indicates an invalid parameter. Both strings must be zero-terminated!*/
#define	CompareStringsAsmW			CompareStringsAsmW64

/* This function can be used for a successive retrieval of the single lines of a textbuffer.

If there is more than one line-end character(cr/lf), the function returns a string-code for every redundant character. "__cr" or "__lf"
But, only the redundand line-end char will be returned. If the format is 'cr/lf', cr will not be returned for lines with content!
But, if this is an empty line, both characters (cr + lf) will be returned!
This is necessary to savely detect empty lines and to conditionally override the lineend-format (lf,cr,cr/lf)

NOTE:	
			The return value is the address to the memory-block containing the line data(if there is data) or nullptr if not.
			The allocated memory-block must be released with 'ReleaseStringVect(...)'.
			The 'StartIndex' Parameter contains the beginning of the next line after a succeeded call or an errorcode.
			Errorcodes: (-1 == no data) (-2 == end of buffer) (-3 == invalid parameter)
 
IMPORTANT:
			Make sure the StartIndex is not outside of the array-boundaries by calling 'GetStringLenghtW' first.
			Otherwise it will cause an access-violation. 'buffer' must be zero-terminated*/
#define	GetNextLineFromStartIndexW	GetNextLineFromStartIndexW64

/* This function replaces the fields in a buffer successive from the given index.
 NOTE:
		'appendix' must be zero-terminated !
		The function returns the copied elements if successful, or a negative value to indicate an error.
		(-1 == error invalid parameter) 

 IMPORTANT:	The function does not check if the end of the block from the beginning of the index is inside the boundaries of the array or
			if the given index is inside the boundaries - so for security reasons a check must be performed before calling this function */
#define	SetBlockInBufferFromIndexW	SetBlockInBufferFromIndexW64

/* This function retrieves the fields in a buffer from the start-index to the end-index

NOTE:
The targetBuffer must be large enough to take the segment (including the terminating null character), otherwise the function will crash
If the function fails, the return-value is negative
IMPORTANT:
The function does not check if the indexes are in the scope of the sourceBuffer, so make sure to check it before calling this function*/
#define	GetBlockSegmentOutOfBufferW	GetBlockSegmentOutOfBufferW64

/* This function inserts a wide-character at the specified index and re-arranges the buffer

NOTE:
The newBuffer must be one field larger than the oldBuffer, otherwise this function will crash
The oldBuffer must be zero-terminated.
If the function fails, the return-value is negative (-1 == error invalid parameter)*/
#define	InsertCharacterInBufferW	InsertCharacterInBufferW64

/* This function removes a wide-character at the specified index an re-arranges the buffer

NOTE:
oldBuffer must be zero-terminated
newBuffer must be [sizeof(oldBuffer) - 1] or larger (optimum is one field smaller)
If the function fails the return-value is negative (-1 == error invalid parameter)*/
#define	RemoveCharacterFromBufferW	RemoveCharacterFromBufferW64

/* This function inserts a wide-char-block at the specified Index and stores the new string in a new buffer

NOTE:
The targetBuffer must have the size of the oldBuffer + the bufferToInsert + 1 for the terminating null character.
The oldBuffer and the bufferToInsert must be zero terminated.
If the function fails the return value is negative. (-1 == invalid parameter)*/
#define	InsertBlockInBufferW		InsertBlockInBufferW64

/* This function removes a wide-char-block from the specified startindex to the endindex and stores the data in a new buffer

NOTE:
The targetBuffer must have the size of the oldBuffer minus the defined segment + 1 for the terminating null character
The oldBuffer must be zero-terminated.
If the function fails the return value is negative. (-1 == invalid parameter)*/
#define	RemoveBlockFromBufferW		RemoveBlockFromBufferW64
#endif


#endif // !_STRING_PROCESS_HELPER_H_