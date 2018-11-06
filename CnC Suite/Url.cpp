#include"Url.h"
#include"HelperF.h"

void Url::SetUrlFromLocalPath(LPCTSTR path)
{
	if (path != nullptr)
	{
		iString newUrl(L"file:///");
		auto len = _lengthOfString(path);

		if (len > 0)
		{
			for (int i = 0; i < len; i++)
			{
				if (path[i] == L'\\')
				{
					newUrl += L"/";
				}
				else if (path[i] == L' ')
				{
					newUrl += L"%20";
				}
				else
				{
					TCHAR buf[2];
					buf[0] = path[i];
					buf[1] = L'\0';

					newUrl += buf;
				}
			}
			this->_url_.Replace(newUrl);
		}
	}
}
