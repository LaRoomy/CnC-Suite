#pragma once
#include"external.h"

class LanguageDispatcher
{
public:
	LanguageDispatcher(int language)
		:lang(language)
	{
		// ...
	}

	void Release() { delete this; }

	TCHAR* getLangString(int ID)
	{
		switch (this->lang)
		{
		case LANG_GERMAN:
			return this->getGermanString(ID);
		case LANG_ENGLISH:
			return this->getEnglishString(ID);
		default:
			return this->getEnglishString(ID);
		}
	}

private:
	int lang;

	TCHAR* getGermanString(int ID);
	TCHAR* getEnglishString(int ID);
};



