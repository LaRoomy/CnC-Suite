#include "LogControl.h"
#include "BasicFPO.h"
#include "AppPath.h"

LogControl::LogControl()
	: currentAddIndex(0)
{
	for (int i = 0; i < 100; i++) {
		this->logData.AddItem(iString());
	}
}

void LogControl::AddLogString(LPCTSTR data)
{
	if (data != nullptr)
	{
		if (this->currentAddIndex >= 0 && (this->currentAddIndex < this->logData.GetCount())) {
			this->logData.getObjectCoreReferenceAt(this->currentAddIndex)->Replace(data);

			this->currentAddIndex++;

			if (this->currentAddIndex > 99)
			{
				this->currentAddIndex = 0;
			}
		}
	}
}

void LogControl::SaveToFile()
{
	auto bfpo = CreateBasicFPO();
	if (bfpo != nullptr)
	{
		AppPath appPath;

		auto path = appPath.Get(PATHID_FILE_TEMPLOGDATA);
		if (path.succeeded())
		{
			iString buffer;
			int maxCounter = 0;
			int cPosition = this->currentAddIndex;
			if (cPosition < 0)
			{
				cPosition = 0;
			}

			while (maxCounter < 100)
			{
				if (cPosition < this->logData.GetCount())
				{
					if (this->logData.getObjectCoreReferenceAt(cPosition)->GetLength() > 0)
					{
						//iString strIndex(cPosition);
						//buffer += strIndex;
						buffer += this->logData.getObjectCoreReferenceAt(cPosition)->GetData();
						buffer += L"\r\n";
					}
				}

				cPosition++;
				if (cPosition > 99)
				{
					cPosition = 0;
				}

				maxCounter++;
			}

			bfpo->SaveBufferToFile(buffer.GetData(), path.GetData());
		}
	}
}

void LogControl::Clear()
{
	this->logData.Clear();

	for (int i = 0; i < 100; i++) {
		this->logData.AddItem(iString());
	}
}

bool LogControl::hasContent()
{
	for (int i = 0; i < this->logData.GetCount(); i++)
	{
		if (this->logData.getObjectCoreReferenceAt(i)->GetLength() > 0)
		{
			return true;
		}
	}
	return false;
}