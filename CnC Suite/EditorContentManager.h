#pragma once
#ifndef _EDITORCONTENTMANAGER_H_
#define _EDITORCONTENTMANAGER_H_

#include<Windows.h>
#include"cObject.h"
#include"CommonControls\ItemCollection.h"

typedef int CONVERSIONDIRECTION;

typedef struct _ECMCTDATA {

	LONG_PTR toClass;
	CONVERSIONDIRECTION direction;

}ECMCTDATA, *LPECMCTDATA;

__interface IConversionProtocol
{
	void CollectionComplete(cObject sender);
	void TextBufferReady(cObject sender);
	void ConversionError(cObject sender);
};

class EditorContentManager
{
public:
	EditorContentManager();
	EditorContentManager(LPCWSTR content);
	EditorContentManager(IConversionProtocol* handler);
	EditorContentManager(LPCWSTR content, IConversionProtocol* handler);
	EditorContentManager(const EditorContentManager& ecm);

	~EditorContentManager();

	// Releases the object, if asyncronous operations are pending the release will be scheduled and executes when the thread is finished
	// NOTE: Do never delete the object direct when the asynchronous mode is used, this could corrupt the system
	void Release() {
		if (this->IsAsyncComplete())
		{
			delete this;
		}
		else
		{
			this->events = nullptr;
			InterlockedExchange((LONG*)&this->deleteObjectAsPostOperation, TRUE);
		}
	}

	/*	IMPORTANT NOTE:	
						THE END-OF-LINE CONVERSION FLAGS ARE ONLY NECESSARY FOR THE OUTPUT-CONVERSION!
						THE INPUT-FORMAT MUST BE UNICODE-UTF16 AND THE LINE-END SHOULD ONLY BE MARKED WITH '\r' (carriage return)
						A LINEFEED ('\n') WILL ALSO BE TREATED AS AN NEW LINE, SO A CR/LF FORMAT WILL INSERT AN ADDITIONAL BLANK LINE
	*/

	// Sets the content by copying the buffer
	// NOTE:
	//		The asynchronous execution of the conversion is enabled by default and must be explicitly disabled if not needed
	//		If executed asynchron, remember to hook up an "IConversionProtocol" event-handler to be informed when the operation is complete
	//      The method automatically sets the EOL-Format parameter to the format of the given buffer
	void SetContent(LPCWSTR content);

	// Sets the content without copying, the internal WCHAR* pointer will be set to the value of the pToBuffer parameter
	// this method is for efficiency purposes, but also dangerous! Discard the pointer outside of this component immediately after calling this method
	// otherwise an deletition or change of the buffer will corrupt the EditorContentManager object
	// This method also starts the conversion, so see the NOTE-section by 'SetContent(...)'
	void SetContentWithoutCopy(WCHAR* pToBuffer);

	// converts the internal line-structure to a complete buffer
	// NOTE:
	//		The asynchronous execution of the conversion is enabled by default and must be explicitly disabled if not needed
	//		If executed asynchron, remember to hook up an event handler to be informed when the conversion is complete
	//		If executed synchron, 'GetContent()' can be called direct after this method.
	//		The method returns false if there is no content
	bool PrepareContentForSubsequentUsage();
	
	// Retrieves a pointer to the internal contentBuffer
	// NOTE: Set the line-end format first (the default is Auto - '\r')
	// -> then call PrepareContentForSubsequentUsage() to start the conversion
	const WCHAR* GetContent() const {
		return this->contentTextBuffer;
	}

	// returns the amount of lines
	int GetLineCount() {
		return this->lines.GetCount();
	}

	// returns the line data
	const WCHAR* GetLine(int index) const {
		return (this->lines.GetCount() > index)
			? this->lines.GetAt(index) : nullptr;
	}
	
	// Replaces a line at the given index
	// to replace with an empty line call the method with the empty-line-placeholder: ReplaceLine(EditorContentManager::EMPTY_LINE, index);
	void ReplaceLine(LPCWSTR line, int index);

	// Adds a new line at the end of the text
	// to add an empty line call the method with the empty-line-placeholder:	AddLine(EditorContentManager::EMPTY_LINE);
	void AddLine(LPCWSTR line);

	// Inserts a line at the given index and moves all other lines down (including the current line at the index)
	// to insert an empty line call the method with the empty-line-placeholder: InsertLine(EditorContentManager::EMPTY_LINE, index);
	void InsertLineAt(LPCWSTR line, int index);

	// Adds a new character to the line (at the end of the line)
	void AddCharToLine(WCHAR c, int index);

	// Inserts a new character in the line (at the specified row)
	void InsertCharInLine(WCHAR c, int index, int row);

	// Removes a character at the specified row from the line at the specified index
	// if there is only one character in the line, the row-parameter will be ignored and the line will be set as empty line
	void RemoveCharFromLine(int index, int row);

	// Adds a buffer-segment at the end of the line on the specified index
	void AddSegmentToLine(LPCWSTR segment, int index);

	// Inserts a buffer-segment at the specified row
	void InsertSegmentInLine(LPCWSTR segment, int index, int row);

	// Removes a buffer-segment from the specified start-index to the end-index
	void RemoveSegmentFromLine(int index, int row_start, int row_end);

	// Removes a complete line at the specified index
	void RemoveLineAt(int index);

	// Removes all lines from the startindex to the endindex
	void RemoveMultipleLines(int startIndex, int endIndex);

	// Erase the object-content and reset the internal control parameter
	// NOTE: If an async operation is pending, the method returns false and the object will not be cleared!
	bool Clear();

	// Hook up an eventhandler to look for the end of an asynchron operation
	void SetEventHandler(IConversionProtocol* eventHandler) {
		this->events = eventHandler;
	}

	// if set to true, all conversions will be executed in a new thread
	// this is true by default
	// NOTE: hook up an event handler to get information when the conversion is complete
	void SetExecuteAsync(bool executeasynchron) {
		this->executeAsync = executeasynchron;
	}

	// thread-save method to get info wheather the conversion-thread is currently active
	bool IsAsyncComplete();

	// returns the amount of character in the line at the specified index
	// (excluding the terminating null character!)
	int GetLineLength(int lineIndex);

	// set the output format for the textconversion
	// the default is AUTO ('\r')
	void SetEndOfLineFormat(DWORD eolf) {
		this->eolFormat =
			(!((eolf & 0x01) || (eolf & 0x02) || (eolf & 0x04) || (eolf & 0x08)))
			? 0x08 : eolf;
	}

	// Sets the save-path and enables the automatic write-to-file process when the conversion is complete
	// NOTE: If the path == nullptr, this function will be disabled and the path is cleared
	void SaveToFileAfterConversion(LPCTSTR path);

	// indicates if the internal structure is different from the content-buffer
	// if this is true, the struture must be converted before the buffer can be retrieved with GetContent()
	bool HasContentChanged() {
		return this->hasChanged;
	}

	static const CONVERSIONDIRECTION TEXTBUFFER_TOLINECOLLECTION = 1;
	static const CONVERSIONDIRECTION COLLECTION_TOTEXTBUFFER = 2;

	static const DWORD ENDOFLINE_FORMAT_LF = 0x01;
	static const DWORD ENDOFLINE_FORMAT_CR = 0x02;
	static const DWORD ENDOFLINE_FORMAT_CRLF = 0x04;
	static const DWORD ENDOFLINE_FORMAT_AUTO = 0x08;

	static const WCHAR* EMPTY_LINE;

private:
	WCHAR * contentTextBuffer;
	itemCollection<WCHAR*> lines;

	WCHAR* pathToFile;

	BOOL writeBufferToFileAfterConversion;
	BOOL deleteObjectAsPostOperation;

	bool hasChanged;
	bool executeAsync;
	DWORD eolFormat;
	BOOL asyncComplete;

	IConversionProtocol* events;

	static DWORD __stdcall conversionProc(LPVOID lParam);

	bool textBufferToCollection();
	bool collectionToTextBuffer();

	void convert(CONVERSIONDIRECTION direction);
	void convertAsync(CONVERSIONDIRECTION direction);

	void saveBufferAsTemporaryFile();

	// watch out for race-conditions or access-violations!
	// what is when data is added or altered in the editor and the internal conversion is running?!

	bool isEmptyLine(LPCWSTR line, int len);
	void autoDetectLineEndFormat();
};

#endif // !_EDITORCONTENTMANAGER_H_
