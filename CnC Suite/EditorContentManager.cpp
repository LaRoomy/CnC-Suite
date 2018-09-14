#include"EditorContentManager.h"
#include"HelperF.h"

EditorContentManager::EditorContentManager()
	: contentTextBuffer(nullptr),
	events(nullptr),
	executeAsync(true),
	eolFormat(EditorContentManager::ENDOFLINE_FORMAT_AUTO),
	asyncComplete(TRUE),
	hasChanged(false),
	writeBufferToFileAfterConversion(FALSE),
	pathToFile(nullptr),
	deleteObjectAsPostOperation(FALSE)
{
}

EditorContentManager::EditorContentManager(LPCWSTR content)
	: events(nullptr),
	executeAsync(true),
	eolFormat(EditorContentManager::ENDOFLINE_FORMAT_AUTO),
	asyncComplete(TRUE),
	hasChanged(false),
	writeBufferToFileAfterConversion(FALSE),
	pathToFile(nullptr),
deleteObjectAsPostOperation(FALSE)
{
	this->SetContent(content);
}

EditorContentManager::EditorContentManager(IConversionProtocol * handler)
	: contentTextBuffer(nullptr),
	events(handler),
	executeAsync(true),
	eolFormat(EditorContentManager::ENDOFLINE_FORMAT_AUTO),
	asyncComplete(TRUE),
	hasChanged(false),
	writeBufferToFileAfterConversion(FALSE),
	pathToFile(nullptr),
	deleteObjectAsPostOperation(FALSE)
{
}

EditorContentManager::EditorContentManager(LPCWSTR content, IConversionProtocol * handler)
	: events(handler),
	executeAsync(true),
	eolFormat(EditorContentManager::ENDOFLINE_FORMAT_AUTO),
	asyncComplete(TRUE),
	hasChanged(false),
	writeBufferToFileAfterConversion(FALSE),
	pathToFile(nullptr),
	deleteObjectAsPostOperation(FALSE)
{
	this->SetContent(content);
}

EditorContentManager::EditorContentManager(const EditorContentManager & ecm)
	: events(nullptr),
	executeAsync(true),
	eolFormat(EditorContentManager::ENDOFLINE_FORMAT_AUTO),
	asyncComplete(TRUE),
	hasChanged(false),
	writeBufferToFileAfterConversion(FALSE),
	pathToFile(nullptr),
	deleteObjectAsPostOperation(FALSE)
{
	this->SetContent(
		ecm.GetContent()
	);
}

EditorContentManager::~EditorContentManager()
{
	this->Clear();
}

const WCHAR* EditorContentManager::EMPTY_LINE = L"__blank";

void EditorContentManager::SetContent(LPCWSTR content)
{
	this->Clear();

	if (content != nullptr)
	{
		if (CopyStringToPtr(content, &this->contentTextBuffer) == TRUE)
		{
			if (this->executeAsync)
				this->convertAsync(EditorContentManager::TEXTBUFFER_TOLINECOLLECTION);
			else
				this->convert(EditorContentManager::TEXTBUFFER_TOLINECOLLECTION);
		}
	}
}

void EditorContentManager::SetContentWithoutCopy(WCHAR * pToBuffer)
{
	this->Clear();

	if (pToBuffer != nullptr)
	{
		this->contentTextBuffer = pToBuffer;

		if (this->executeAsync)
			this->convertAsync(EditorContentManager::TEXTBUFFER_TOLINECOLLECTION);
		else
			this->convert(EditorContentManager::TEXTBUFFER_TOLINECOLLECTION);
	}
}

bool EditorContentManager::PrepareContentForSubsequentUsage()
{
	if (this->GetLineCount() > 0)
	{
		if (this->executeAsync)
			this->convertAsync(EditorContentManager::COLLECTION_TOTEXTBUFFER);
		else
			this->convert(EditorContentManager::COLLECTION_TOTEXTBUFFER);

		return true;
	}
	else
		return false;
}

void EditorContentManager::ReplaceLine(LPCWSTR line, int index)
{
	if (line != nullptr)
	{
		auto _line_ = this->lines.GetAt(index);
		if (_line_ != nullptr)
		{
			delete[] _line_;
			_line_ = nullptr;

			if (CopyStringToPtr(line, &_line_) == TRUE)
			{
				this->lines.ReplaceAt(index, _line_);
				this->hasChanged = true;
			}
		}
	}
}

void EditorContentManager::AddLine(LPCWSTR line)
{
	if (line != nullptr)
	{
		WCHAR* _line_ = nullptr;

		if (CopyStringToPtr(line, &_line_) == TRUE)
		{
			this->lines.AddItem(_line_);
			this->hasChanged = true;
		}
	}
}

void EditorContentManager::InsertLineAt(LPCWSTR line, int index)
{
	if (line != nullptr)
	{
		WCHAR* _line_ = nullptr;

		if (CopyStringToPtr(line, &_line_) == TRUE)
		{
			this->lines.InsertAt(index, _line_);
			this->hasChanged = true;
		}
	}
}

void EditorContentManager::AddCharToLine(WCHAR c, int index)
{
	// at first validate the index
	auto nItems = this->lines.GetCount();

	if ((index >= 0) && (index < nItems))
	{
		// get the line
		auto str = this->lines.GetAt(index);
		if (str != nullptr)
		{
			auto len = GetStringLengthW(str);
			if (len > 0)
			{
				// check if this is an empty line
				if (CompareStringsAsmW(EditorContentManager::EMPTY_LINE, str) == 1)
				{
					// this is an empty line, so now this line contains only one character
					len = 1;
				}

				WCHAR* newLine = new WCHAR[len + 1];
				if (newLine != nullptr)
				{
					newLine[0] = L'\0';// secure the string

					if (len > 1)// the new string must be build
					{
						if (SetBlockInBufferFromIndexW(newLine, str, 0) > 0)
						{
							newLine[len - 1] = c;

							// set zero terminator
							newLine[len] = L'\0';
						}
					}
					else if(len == 1)// the character is the new string
					{
						newLine[0] = c;
						newLine[1] = L'\0';
					}
					else
					{
						// what ?? -> error...
					}
					this->lines.ReplaceAt(index, newLine);
					SafeDeleteArray(&str);
					this->hasChanged = true;
				}
			}
		}
	}
}

void EditorContentManager::InsertCharInLine(WCHAR c, int index, int row)
{
	// at first validate the index
	auto nItems = this->lines.GetCount();

	if ((index >= 0) && (index < nItems))
	{
		// get the line
		auto str = this->lines.GetAt(index);
		if (str != nullptr)
		{
			// get the length of the line
			auto len = GetStringLengthW(str);
			if (len > 0)
			{
				if (CompareStringsAsmW(str, EditorContentManager::EMPTY_LINE) == 1)
				{
					// if this is an empty line -> call the add function
					this->AddCharToLine(c, index);
				}				
				else
				{
					// then validate the row
					if ((row >= 0) && (row < len))
					{
						WCHAR* newLine = new WCHAR[len + 1];
						if (newLine != nullptr)
						{
							if (InsertCharacterInBufferW(str, newLine, c, row) == 1)
							{								
								this->lines.ReplaceAt(index, newLine);
								SafeDeleteArray(&str);
								this->hasChanged = true;
							}
						}
					}
				}
			}
		}
	}
}

void EditorContentManager::RemoveCharFromLine(int index, int row)
{
	// at first validate the index
	auto nItems = this->lines.GetCount();

	if ((index >= 0) && (index < nItems))
	{
		// get the line
		auto str = this->lines.GetAt(index);
		if (str != nullptr)
		{
			// get the length of the line
			auto len = GetStringLengthW(str);
			if (len > 2)
			{
				if (CompareStringsAsmW(str, EditorContentManager::EMPTY_LINE) == 1)
				{
					// if this is an empty line -> abort the action
					return;
				}
				else
				{
					auto newLine = new WCHAR[len - 1];
					if (newLine != nullptr)
					{
						// erase the character from the line
						if (RemoveCharacterFromBufferW(str, newLine, row) == 1)
						{
							this->lines.ReplaceAt(index, newLine);
							SafeDeleteArray(&str);
							this->hasChanged = true;
						}
					}
				}
			}
			else
			{
				if (len > 0)
				{
					// if there is only one character in the line set the empty-line placeholder
					WCHAR* emptyLine = nullptr;

					if (CopyStringToPtr(EditorContentManager::EMPTY_LINE, &emptyLine) == TRUE)
					{
						this->lines.ReplaceAt(index, emptyLine);
						SafeDeleteArray(&str);
						this->hasChanged = true;
					}
				}
			}
		}
	}
}

void EditorContentManager::AddSegmentToLine(LPCWSTR segment, int index)
{
	// at first validate the index
	auto nItems = this->lines.GetCount();

	if ((index >= 0) && (index < nItems))
	{
		// get the line
		auto str = this->lines.GetAt(index);
		if (str != nullptr)
		{
			// get the length of the line
			auto len = GetStringLengthW(str);
			if (len > 0)
			{
				if (CompareStringsAsmW(str, EditorContentManager::EMPTY_LINE) == 1)
				{
					// the line was empty, so the segment is the new line
					WCHAR* nLine = nullptr;

					if (CopyStringToPtr(segment, &nLine) == TRUE)
					{
						this->lines.ReplaceAt(index, nLine);
						SafeDeleteArray(&str);
						this->hasChanged = true;
					}
				}
				else
				{
					// build the new line
					auto segLen = GetStringLengthW(segment);
					if (segLen > 0)
					{
						WCHAR *newLine = new WCHAR[ ((len + segLen) - 1) ];
						if (newLine != nullptr)
						{
							if (SetBlockInBufferFromIndexW(newLine, str, 0) > 0)
							{
								if (SetBlockInBufferFromIndexW(newLine, segment, (len - 1)) > 0)
								{
									newLine[((len + segLen) - 2)] = L'\0';

									this->lines.ReplaceAt(index, newLine);
									SafeDeleteArray(&str);
									this->hasChanged = true;
								}
							}
						}
					}
				}
			}
		}
	}
}

void EditorContentManager::InsertSegmentInLine(LPCWSTR segment, int index, int row)
{
	// at first validate the index
	auto nItems = this->lines.GetCount();

	if ((index >= 0) && (index < nItems))
	{
		// get the line
		auto str = this->lines.GetAt(index);
		if (str != nullptr)
		{
			// get the length of the line
			auto len = GetStringLengthW(str);
			if (len > 0)
			{
				if (CompareStringsAsmW(str, EditorContentManager::EMPTY_LINE) == 1)
				{
					// if this is an empty line -> add the segment
					this->AddSegmentToLine(segment, index);
				}
				else
				{
					if (row >= len)
					{
						// if the row parameter is too high -> add the segment on the end
						this->AddSegmentToLine(segment, index);
					}
					else
					{
						// build the new line
						auto segLen = GetStringLengthW(segment);
						if (segLen > 0)
						{
							auto *newLine = new WCHAR[((len + segLen) - 1)];
							if (newLine != nullptr)
							{
								if (row <= 0)
								{
									// if the row-parameter is zero or below -> append the line to the segment
									if (SetBlockInBufferFromIndexW(newLine, segment, 0) > 0)
									{
										if (SetBlockInBufferFromIndexW(newLine, str, segLen - 1) > 0)
										{
											newLine[((len + segLen) - 2)] = L'\0';

											this->lines.ReplaceAt(index, newLine);
											SafeDeleteArray(&str);
											this->hasChanged = true;
										}
									}
								}
								else
								{
									// REMARK: The commented section is the less effective C++ version of the assembly-function 'InsertBlockInBufferW(...)'

									// insert the segment
									//auto startSegment = new WCHAR[row + 1];
									//if (startSegment != nullptr)
									//{
									//	if (GetBlockSegmentOutOfBufferW(str, startSegment, 0, row - 1) == 1)
									//	{
									//		if (SetBlockInBufferFromIndexW(newLine, startSegment, 0) > 0)
									//		{
									//			if (SetBlockInBufferFromIndexW(newLine, segment, row) > 0)
									//			{
									//				if (
									//					SetBlockInBufferFromIndexW(
									//						newLine,
									//						&str[row],
									//					((row + segLen) - 1)
									//					) > 0)
									//				{
									//					newLine[((len + segLen) - 2)] = L'\0';

									//					this->lines.ReplaceAt(index, newLine);
									//					SafeDeleteArray(&str);
									//					this->hasChanged = true;
									//				}
									//			}
									//		}
									//	}
									//	SafeDeleteArray(&startSegment);
									//}

									if (InsertBlockInBufferW(newLine, str, segment, row) > 0)
									{
										this->lines.ReplaceAt(index, newLine);
										SafeDeleteArray(&str);
										this->hasChanged = true;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void EditorContentManager::RemoveSegmentFromLine(int index, int row_start, int row_end)
{
	// at first validate the index
	auto nItems = this->lines.GetCount();

	if ((index >= 0) && (index < nItems))
	{
		// validate the rows
		if (row_start <= row_end)
		{
			// get the line
			auto str = this->lines.GetAt(index);
			if (str != nullptr)
			{
				// get the length of the line
				auto len = GetStringLengthW(str);
				if (len > 0)
				{
					if (CompareStringsAsmW(str, EditorContentManager::EMPTY_LINE) == 1)
					{
						// this is an empty line -> must be an error -> exit function
						return;
					}
					else
					{
						auto newLine = new WCHAR[((len + 1) - (row_end - row_start))];
						if (newLine != nullptr)
						{
							if (RemoveBlockFromBufferW(newLine, str, row_start, row_end) > 0)
							{
								this->lines.ReplaceAt(index, newLine);
								SafeDeleteArray(&str);
								this->hasChanged = true;
							}
						}
					}
				}
			}
		}
	}
}

void EditorContentManager::RemoveLineAt(int index)
{
	if (index >= 0)
	{
		if (index < this->lines.GetCount())
		{
			auto line = this->lines.GetAt(index);
			if (line != nullptr)
			{
				delete[] line;
			}
			this->lines.RemoveAt(index);
			this->hasChanged = true;
		}
	}
}

void EditorContentManager::RemoveMultipleLines(int startIndex, int endIndex)
{
	if (
		(startIndex < endIndex)
		&& ((startIndex >= 0)
			&& (endIndex >= 0)
			&& (endIndex < this->lines.GetCount()
				)
			)
		) {

		for (int i = startIndex; i <= endIndex; i++)
		{
			auto line = this->lines.GetAt(i);
			if (line != nullptr)
			{
				delete[] line;
			}
			this->lines.RemoveAt(i);			
		}
		this->hasChanged = true;
	}
}

bool EditorContentManager::Clear()
{
	if (this->IsAsyncComplete())
	{
		int lineCount = this->lines.GetCount();

		for (int i = 0; i < lineCount; i++)
		{
			auto pLineBuffer = this->lines.GetAt(i);
			if (pLineBuffer != nullptr)
			{
				delete[] pLineBuffer;
			}
		}
		this->lines.Clear();
		SafeDeleteArray(&this->contentTextBuffer);
		this->hasChanged = false;
		SafeDeleteArray(&this->pathToFile);
		this->writeBufferToFileAfterConversion = FALSE;

		return true;
	}
	else
		return false;
}

bool EditorContentManager::IsAsyncComplete()
{
	return InterlockedCompareExchange((LONG*)&this->asyncComplete, this->asyncComplete, TRUE)
		? true : false;
}

int EditorContentManager::GetLineLength(int lineIndex)
{
	auto line = this->lines.GetAt(lineIndex);

	if (CompareStringsAsmW(line, EditorContentManager::EMPTY_LINE) == 1)
	{
		return 0;
	}
	else
	{
		return _lengthOfString(line);
	}
}

void EditorContentManager::SaveToFileAfterConversion(LPCTSTR path)
{
	SafeDeleteArray(&this->pathToFile);

	if (path == nullptr)
	{
		InterlockedExchange((LONG*)&this->writeBufferToFileAfterConversion, FALSE);
	}
	else
	{
		if (CopyStringToPtr(path, &this->pathToFile) == TRUE)
		{
			InterlockedExchange((LONG*)&this->writeBufferToFileAfterConversion, TRUE);
		}
	}
}

DWORD EditorContentManager::conversionProc(LPVOID lParam)
{
	auto tData = reinterpret_cast<LPECMCTDATA>(lParam);
	if (tData != nullptr)
	{
		auto _this_ = reinterpret_cast<EditorContentManager*>(tData->toClass);
		if (_this_ != nullptr)
		{
			_this_->convert(tData->direction);

			InterlockedExchange((LONG*)&_this_->asyncComplete, (LONG)TRUE);

			if (InterlockedCompareExchange((LONG*)&_this_->deleteObjectAsPostOperation, _this_->deleteObjectAsPostOperation, TRUE))
			{
				delete _this_;
			}
		}
		delete tData;
	}
	return 0;
}

bool EditorContentManager::textBufferToCollection()
{
	__try
	{
		if (this->eolFormat == EditorContentManager::ENDOFLINE_FORMAT_AUTO)
		{
			this->autoDetectLineEndFormat();
		}

		int sIndex = 0;
		STRVECT line = nullptr;

		int maxBuffer =
			GetStringLengthW(this->contentTextBuffer);

		do
		{
			if (sIndex < maxBuffer)
			{
				line = GetNextLineFromStartIndexW(&sIndex, (STRVECT)this->contentTextBuffer);
				if (line != nullptr)
				{
					if (CompareStringsAsmW(line, L"__cr") == 1)
					{
						if ((this->eolFormat == EditorContentManager::ENDOFLINE_FORMAT_CR)
							|| (this->eolFormat == EditorContentManager::ENDOFLINE_FORMAT_CRLF))
						{
							this->AddLine(L"__blank");
						}
					}					
					else if(CompareStringsAsmW(line, L"__lf") == 1)
					{
						if (this->eolFormat == EditorContentManager::ENDOFLINE_FORMAT_LF)
						{
							this->AddLine(L"__blank");
						}
					}
					else
					{
						this->AddLine((LPCWSTR)line);
					}
					ReleaseStringVect(line);
					line = nullptr;
				}
			}

		} while (sIndex >= 0);// the loop should exit with (sIndex == -2) which indicates the end of the buffer - all other negative values are an error!

		return true; // check cIndex for error condition ??
	}
	__except (
		GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
		) {
		return false;
	}
}

bool EditorContentManager::collectionToTextBuffer()
{
	__try
	{
		SafeDeleteArray(&this->contentTextBuffer);

		const auto line_count = this->lines.GetCount();
		if (line_count > 0)
		{
			// at first - calculate the required buffer-size
			int buffer_size = 0;

			for (int i = 0; i < line_count; i++)
			{
				auto str = this->lines.GetAt(i);
				if (str != nullptr)
				{
					// if this is not a blank line, add the size of the line, otherwise skip this and add only the eol-character
					if (CompareStringsAsmW(str, L"__blank") == 0)
					{
						buffer_size += (GetStringLengthW(str) - 1);
					}

					// add the end-of-line character(s)
					if ((this->eolFormat & EditorContentManager::ENDOFLINE_FORMAT_CR)
						|| (this->eolFormat & EditorContentManager::ENDOFLINE_FORMAT_LF)
						|| (this->eolFormat & EditorContentManager::ENDOFLINE_FORMAT_AUTO))
					{
						buffer_size++;
					}
					else
					{
						// must be cr/lf
						buffer_size += 2;
					}
				}
			}
			buffer_size++;	// zero terminator

			// allocate content buffer
			this->contentTextBuffer = new WCHAR[buffer_size];
			if (this->contentTextBuffer != nullptr)
			{
				WCHAR linefeed[] = L"\n";
				WCHAR carriage[] = L"\r";
				WCHAR cr_lf_[] = L"\r\n";

				int buffer_index = 0;

				// build content buffer
				for (int i = 0; i < line_count; i++)
				{
					auto str = this->lines.GetAt(i);
					if (str != nullptr)
					{
						// this is not an empty line so copy the data
						if (CompareStringsAsmW(str, EditorContentManager::EMPTY_LINE) == 0)
						{
							if (SetBlockInBufferFromIndexW(this->contentTextBuffer, str, buffer_index) == -1)
							{
								break;
							}
							buffer_index += (GetStringLengthW(str) - 1);
						}

						if ((this->eolFormat & EditorContentManager::ENDOFLINE_FORMAT_CR)
							|| (this->eolFormat & EditorContentManager::ENDOFLINE_FORMAT_AUTO))
						{
							if (SetBlockInBufferFromIndexW(this->contentTextBuffer, carriage, buffer_index) == -1)
							{
								break;
							}
							buffer_index++;
						}
						else if (this->eolFormat & EditorContentManager::ENDOFLINE_FORMAT_LF)
						{
							if (SetBlockInBufferFromIndexW(this->contentTextBuffer, linefeed, buffer_index) == -1)
							{
								break;
							}
							buffer_index++;
						}
						else
						{
							// must be cr/lf
							if (SetBlockInBufferFromIndexW(this->contentTextBuffer, cr_lf_, buffer_index) == -1)
							{
								break;
							}
							buffer_index += 2;
						}
					}
				}
				this->contentTextBuffer[buffer_index] = L'\0';// set zero terminator!
			}
		}
		return true;
	}
	__except (
		GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION
		? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH
		) {
		return false;
	}
}

void EditorContentManager::convert(CONVERSIONDIRECTION direction)
{
	switch (direction)
	{
	case EditorContentManager::COLLECTION_TOTEXTBUFFER:

		if (this->collectionToTextBuffer())
		{
			if (this->events != nullptr)
			{
				this->events->TextBufferReady(
					reinterpret_cast<cObject>(this)
				);
			}

			// if requested save the buffer to file
			if (InterlockedCompareExchange((LONG*)&this->writeBufferToFileAfterConversion, this->writeBufferToFileAfterConversion, TRUE))
			{
				this->saveBufferAsTemporaryFile();
			}
		}
		else
		{
			if (this->events != nullptr)
			{
				this->events->ConversionError(
					reinterpret_cast<cObject>(this)
				);
			}
		}
		break;
	case EditorContentManager::TEXTBUFFER_TOLINECOLLECTION:

		if (this->textBufferToCollection())
		{
			if (this->events != nullptr)
			{
				this->events->CollectionComplete(
					reinterpret_cast<cObject>(this)
				);
			}
		}
		else
		{
			if (this->events != nullptr)
			{
				this->events->ConversionError(
					reinterpret_cast<cObject>(this)
				);
			}
		}
		break;
	default:
		break;
	}
}

void EditorContentManager::convertAsync(CONVERSIONDIRECTION direction)
{
	auto tData = new ECMCTDATA;
	if (tData != nullptr)
	{
		HANDLE hThread;
		DWORD threadID;

		this->asyncComplete = FALSE;

		tData->toClass = reinterpret_cast<LONG_PTR>(this);
		tData->direction = direction;

		hThread = CreateThread(
			nullptr,
			0,
			EditorContentManager::conversionProc,
			reinterpret_cast<LPVOID>(tData),
			0,
			&threadID
		);
		_NOT_USED(hThread);
	}
}

void EditorContentManager::saveBufferAsTemporaryFile()
{
	if (this->contentTextBuffer != nullptr)
	{
		WCHAR *path = nullptr;

		if (AppendStringToString(this->pathToFile, L".temp", &path) == TRUE)
		{
			char* utf8Buffer = nullptr;

			if (ConvertWCHARtoCHAR(this->contentTextBuffer, &utf8Buffer) == TRUE)
			{
				HANDLE hFile =
					CreateFile(
						path,
						GENERIC_WRITE,
						FILE_SHARE_WRITE,
						nullptr,
						CREATE_ALWAYS,
						FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_HIDDEN,
						nullptr
					);

				if (hFile != INVALID_HANDLE_VALUE)
				{
					DWORD bytesWritten = 0;
					DWORD numberOfBytesToWrite = _lengthOfString(this->contentTextBuffer);

					auto result = WriteFile(hFile, utf8Buffer, numberOfBytesToWrite, &bytesWritten, nullptr);
					if (result)
					{
						// ...
					}
					CloseHandle(hFile);
				}
			}
			SafeDeleteArray(&path);
		}
	}
}

bool EditorContentManager::isEmptyLine(LPCWSTR line, int len)
{
	if (line != nullptr)
	{
		if (len < 0)
		{
			len = GetStringLengthW(line);
		}
		if (len == 8)
		{
			if (CompareStringsAsmW(
				line, EditorContentManager::EMPTY_LINE)
				== 1)
			{
				return true;
			}
		}
	}
	return false;
}

void EditorContentManager::autoDetectLineEndFormat()
{
	if (this->contentTextBuffer != nullptr)
	{
		int i = 0;

		while ((this->contentTextBuffer[i] != L'\r') && (this->contentTextBuffer[i] != L'\n'))
		{
			if (this->contentTextBuffer[i] == L'\0')
				return;

			i++;
		}
		if ((this->contentTextBuffer[i] == L'\r') && (this->contentTextBuffer[i + 1] == L'\n'))
		{
			this->SetEndOfLineFormat(
				EditorContentManager::ENDOFLINE_FORMAT_CRLF
			);
		}
		else if ((this->contentTextBuffer[i] == L'\r') && (this->contentTextBuffer[i + 1] == L'\r'))
		{
			this->SetEndOfLineFormat(
				EditorContentManager::ENDOFLINE_FORMAT_CR
			);
		}
		else if ((this->contentTextBuffer[i] == L'\r') && ((this->contentTextBuffer[i + 1] != L'\r') && (this->contentTextBuffer[i] != L'\n')))
		{
			this->SetEndOfLineFormat(
				EditorContentManager::ENDOFLINE_FORMAT_CR
			);
		}
		else if (this->contentTextBuffer[i] == L'\n')
		{
			this->SetEndOfLineFormat(
				EditorContentManager::ENDOFLINE_FORMAT_LF
			);
		}
		else
		{
			this->SetEndOfLineFormat(
				EditorContentManager::ENDOFLINE_FORMAT_LF
			);
		}
	}
}
