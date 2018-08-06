#pragma once
#include"external.h"
#include<StringClass.h>

//This is TTF file header
typedef struct _tagTT_OFFSET_TABLE {
	USHORT uMajorVersion;
	USHORT uMinorVersion;
	USHORT uNumOfTables;
	USHORT uSearchRange;
	USHORT uEntrySelector;
	USHORT uRangeShift;
}TT_OFFSET_TABLE;

//Tables in TTF file and there placement and name (tag)
typedef struct _tagTT_TABLE_DIRECTORY {
	char szTag[4]; //table name
	ULONG uCheckSum; //Check sum
	ULONG uOffset; //Offset from beginning of file
	ULONG uLength; //length of the table in bytes
}TT_TABLE_DIRECTORY;

//Header of names table
typedef struct _tagTT_NAME_TABLE_HEADER {
	USHORT uFSelector; //format selector. Always 0
	USHORT uNRCount; //Name Records count
	USHORT uStorageOffset; //Offset for strings storage, 
						   //from start of the table
}TT_NAME_TABLE_HEADER;

//Record in names table
typedef struct _tagTT_NAME_RECORD {
	USHORT uPlatformID;
	USHORT uEncodingID;
	USHORT uLanguageID;
	USHORT uNameID;
	USHORT uStringLength;
	USHORT uStringOffset; //from start of storage area
}TT_NAME_RECORD;

#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

iString GetFontNameFromFile(LPCTSTR lpszFilePath)
{
	FILE *stream;
	iString path(lpszFilePath);
	iString csRetVal;

	stream =
		_tfsopen(
			path.GetData(),
			L"rb",
			_SH_DENYWR
		);

	if (stream != NULL)
	{
		TT_OFFSET_TABLE ttOffsetTable;

		fread(&ttOffsetTable, sizeof(TT_OFFSET_TABLE), 1, stream);

		ttOffsetTable.uNumOfTables = SWAPWORD(ttOffsetTable.uNumOfTables);
		ttOffsetTable.uMajorVersion = SWAPWORD(ttOffsetTable.uMajorVersion);
		ttOffsetTable.uMinorVersion = SWAPWORD(ttOffsetTable.uMinorVersion);

		//check is this is a true type font and the version is 1.0
		if (ttOffsetTable.uMajorVersion != 1 || ttOffsetTable.uMinorVersion != 0)
			return csRetVal;

		TT_TABLE_DIRECTORY tblDir;
		SecureZeroMemory(&tblDir, sizeof(TT_TABLE_DIRECTORY));

		BOOL bFound = FALSE;
		iString csTemp;

		for (int i = 0; i< ttOffsetTable.uNumOfTables; i++)
		{
			csTemp.Clear();
			SecureZeroMemory(&tblDir, sizeof(TT_TABLE_DIRECTORY));

			fread(&tblDir, sizeof(TT_TABLE_DIRECTORY), 1, stream);

			TCHAR buf[5];
			buf[0] = tblDir.szTag[0];
			buf[1] = tblDir.szTag[1];
			buf[2] = tblDir.szTag[2];
			buf[3] = tblDir.szTag[3];
			buf[4] = L'\0';

			csTemp.Replace(buf);

			if(csTemp.EqualsNoCase(L"name"))
			{
				bFound = TRUE;
				tblDir.uLength = SWAPLONG(tblDir.uLength);
				tblDir.uOffset = SWAPLONG(tblDir.uOffset);
				break;
			}
		}
		if (bFound)
		{
			fseek(stream, tblDir.uOffset, SEEK_SET);

			TT_NAME_TABLE_HEADER ttNTHeader;

			fread(&ttNTHeader, sizeof(TT_NAME_TABLE_HEADER), 1, stream);

			ttNTHeader.uNRCount = SWAPWORD(ttNTHeader.uNRCount);
			ttNTHeader.uStorageOffset = SWAPWORD(ttNTHeader.uStorageOffset);

			TT_NAME_RECORD ttRecord;
			bFound = FALSE;

			for (int i = 0; i<ttNTHeader.uNRCount; i++)
			{
				fread(&ttRecord, sizeof(TT_NAME_RECORD), 1, stream);

				ttRecord.uNameID = SWAPWORD(ttRecord.uNameID);

				if (ttRecord.uNameID == 1)
				{
					ttRecord.uStringLength = SWAPWORD(ttRecord.uStringLength);
					ttRecord.uStringOffset = SWAPWORD(ttRecord.uStringOffset);

					fpos_t nPos;
					fgetpos(stream, &nPos);

					fseek(stream, tblDir.uOffset + ttRecord.uStringOffset + ttNTHeader.uStorageOffset, SEEK_SET);

					char* lowBuffer = new char[ttRecord.uStringLength + 1];
					if (lowBuffer != nullptr)
					{
						fread(lowBuffer, ttRecord.uStringLength, 1, stream);
						lowBuffer[ttRecord.uStringLength] = '\0';

						iString _sstring(lowBuffer);

						if (_sstring.GetLength() > 0)
						{
							csRetVal = _sstring;
							SafeDeleteArray(&lowBuffer);
							break;
						}
						SafeDeleteArray(&lowBuffer);
					}
					fseek(stream, (long)nPos, SEEK_SET);
				}
			}
		}
		fclose(stream);
	}
	return csRetVal;
}

