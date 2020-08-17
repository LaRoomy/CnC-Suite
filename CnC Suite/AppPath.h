#pragma once
#include"external.h"
#include"CommonControls\StringClass.h"
#include"BasicFPO.h"

#define			PATHID_FILE_COMSETUP		1
#define			PATHID_FILE_EDITCOLOR		2
#define			PATHID_FILE_APPSTYLE		3
#define			PATHID_FILE_NAVROOT			4
#define			PATHID_FILE_WINDOW_DATA		5
#define			PATHID_FILE_AUTOCOMPLETE	6
#define			PATHID_FILE_AUTOSYNTAX		7
#define			PATHID_FILE_SETTINGS		8
#define			PATHID_FILE_EXSETTINGS		9
#define			PATHID_FILE_INTERNALSETUP	10
#define			PATHID_FILE_USERTEXT		11
#define			PATHID_FILE_TABIMAGE		12
#define			PATHID_FILE_TREEVIEWIMAGE	13
#define			PATHID_FILE_SEARCHSETUP		14
#define			PATHID_FILE_EXCEPTIONINFO	15

#define			PATHID_FILE_DESCPROP_ONE	30
#define			PATHID_FILE_DESCPROP_TWO	31
#define			PATHID_FILE_DESCPROP_THREE	32

#define			PATHID_FILE_FIRST_USE			33
#define			PATHID_FILE_ALREADYINSTALLED	34
#define			PATHID_FILE_FIRSTUSEEXAMPLE		35

#define			PATHID_FILE_HISTORY				36
#define			PATHID_FILE_HELPHTML_GERMAN		37
#define			PATHID_FILE_HELPHTML_ENGLISH	38

// ##########################################################################
// important: this ID's must have successive numbers for iteration
#define			PATHID_FOLDER_CNCSUITE_USERFOLDER				100
#define			PATHID_FOLDER_CNCSUITE_DOCUMENTSFOLDER			101
#define			PATHID_FOLDER_CNCSUITE_APPDATA					102
#define			PATHID_FOLDER_CNCSUITE_APPDATA_SESSION			103
#define			PATHID_FOLDER_CNCSUITE_APPDATA_SETTINGS			104
#define			PATHID_FOLDER_CNCSUITE_APPDATA_STRINGS			105
#define			PATHID_FOLDER_CNCSUITE_PROJECTS					106
#define			PATHID_FOLDER_CNCSUITE_SAMPLES					107
#define			PATHID_FOLDER_CNCSUITE_USERSTYLES				108
#define			PATHID_FOLDER_CNCSUITE_USERSTYLES_EDITOR		109
#define			PATHID_FOLDER_CNCSUITE_LOCALAPPDATA				110
#define			PATHID_FOLDER_CNCSUITE_LOCALAPPDATA_CONTROL		111
#define			PATHID_ITERATION_FINAL							112
// ##########################################################################

// Application Folder must be C:\Program Files (x86)\CnC Suite
// -> generated by installer
#define			PATHID_FOLDER_CNCSUITE_APPLICATIONFOLDER		150
#define			PATHID_FOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS	151

#define			PATHID_FOLDER_CNCSUITE_EXECUTABLE_FOLDER		155


// FILES in Documents Folder: #################################################################################################
#define			PATHTOFILE_COMSETUP_SYS						L"\\CnC Suite\\AppData\\comsetup.sys"
#define			PATHTOFILE_EDITCOLORSAMPLE					L"\\CnC Suite\\AppData\\Strings\\ectemp.dat"
#define			PATHTOFILE_APPSTYLE_ID						L"\\CnC Suite\\AppData\\appstyle.sys"
#define			PATHTOFILE_NAVIGATION_FOLDER				L"\\CnC Suite\\AppData\\navroot.sys"
#define			PATHTOFILE_WINDOWPLACEMENT_DATA				L"\\CnC Suite\\AppData\\wnduser.sys"
#define			PATHTOFILE_FILEHISTORY_DATA					L"\\CnC Suite\\AppData\\history.lib"
#define			PATHTOFILE_AUTOCOMPLETE_STRINGS				L"\\CnC Suite\\AppData\\Strings\\autocomplete.xml"
#define			PATHTOFILE_AUTOSYNTAX_SETTINGS				L"\\CnC Suite\\AppData\\Settings\\autosyntax.xml"
#define			PATHTOFILE_CNCSUITE_SETTINGS				L"\\CnC Suite\\AppData\\Settings\\cncsuitesettings.xml"
#define			PATHTOFILE_EXTENDED_SETTINGS				L"\\CnC Suite\\AppData\\Settings\\extendedsettings.xml"
#define			PATHTOFILE_INTERNAL_SETTINGS				L"\\CnC Suite\\AppData\\Settings\\internal.xml"
#define			PATHTOFILE_USERTEXT							L"\\CnC Suite\\AppData\\Settings\\userData.xml"
#define			PATHTOFILE_TABIMAGE							L"\\CnC Suite\\AppData\\Session\\TABimage.xml"
#define			PATHTOFILE_TREEVIEWIMAGE					L"\\CnC Suite\\AppData\\Session\\TVimage.dat"
#define			PATHTOFILE_LASTEXCEPTIONINFO				L"\\CnC Suite\\AppData\\Session\\lastExeptionInfo.dat"
#define			PATHTOFILE_SEARCHSETTINGS					L"\\CnC Suite\\AppData\\srchset.sys"
#define			PATHTOFILE_HELP_GERMAN						L"\\CnC Suite\\Manual\\Handbuch.html"
#define			PATHTOFILE_HELP_ENGLISH						L"\\CnC Sutie\\Manual\\manual.html"

#define			PATHTOFILE_DESCPROPERTY_ONE					L"\\CnC Suite\\AppData\\Strings\\Dproperty1.dat"
#define			PATHTOFILE_DESCPROPERTY_TWO					L"\\CnC Suite\\AppData\\Strings\\Dproperty2.dat"
#define			PATHTOFILE_DESCPROPERTY_THREE				L"\\CnC Suite\\AppData\\Strings\\Dproperty3.dat"

#define			PATHTOFILE_FIRSTUSE_EXAMPLEFILE				L"\\CnC-Suite\\Projects\\ExampleProject1\\example one.cnc3"
//##############################################################################################################################
//##############################################################################################################################
// FILES in Local-App-Data Folder:  ############################################################################################
#define			PATHTOFILE_FILEFORFIRSTUSE					L"\\CnC Suite\\CtrlData\\fstuse.dat"
#define			PATHTOFILE_ALREADYINSTALLED					L"\\CnC Suite\\CtrlData\\lrdinst.dat"
//##############################################################################################################################
//##############################################################################################################################
// FOLDERS in Documents Folder: ################################################################################################
#define			PATHTOFOLDER_CNCSUITE_USERFOLDER				L"\\CnC Suite"
#define			PATHTOFOLDER_CNCSUITE_DOCUMENTSFOLDER			L"\\CnC-Suite"
#define			PATHTOFOLDER_CNCSUITE_APPDATA					L"\\CnC Suite\\AppData"
#define			PATHTOFOLDER_CNCSUITE_APPDATA_SESSION			L"\\CnC Suite\\AppData\\Session"
#define			PATHTOFOLDER_CNCSUITE_APPDATA_SETTINGS			L"\\CnC Suite\\AppData\\Settings"
#define			PATHTOFOLDER_CNCSUITE_APPDATA_STRINGS			L"\\CnC Suite\\AppData\\Strings"
#define			PATHTOFOLDER_CNCSUITE_PROJECTS					L"\\CnC-Suite\\Projects"
#define			PATHTOFOLDER_CNCSUITE_SAMPLES					L"\\CnC-Suite\\Templates"
#define			PATHTOFOLDER_CNCSUITE_USERSTYLES				L"\\CnC Suite\\UserStyles"
#define			PATHTOFOLDER_CNCSUITE_USERSTYLES_EDITOR			L"\\CnC Suite\\UserStyles\\Editor"

#ifndef _WIN64
#define			PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER			L"C:\\Program Files (x86)\\CnC Suite"
#define			PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS	L"C:\\Program Files (x86)\\CnC Suite\\fonts"
#else
#define			PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER			L"C:\\Program Files\\CnC Suite"
#define			PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS	L"C:\\Program Files\\CnC Suite\\fonts"
#endif
//##############################################################################################################################
//##############################################################################################################################
// FOLDERS in Local-App-Data Folder:  ##########################################################################################
#define			PATHTOFOLDER_CNCSUITE_LOCALAPPDATA				L"\\CnC Suite"
#define			PATHTOFOLDER_CNCSUITE_LOCALAPPDATA_CONTROL		L"\\CnC Suite\\CtrlData"



class AppPath
{
public:
	AppPath(){

		this->setUserDirPath();
		
		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr)
		{
			TCHAR* path = nullptr;

			if (bfpo->GetKnownFolderPath(&path, FOLDERID_Documents))
			{
				this->documentsPath = path;
			}
			SafeDeleteArray(&path);
			SafeRelease(&bfpo);
		}
	}
	
	iString Get(int pathID)
	{
		switch (pathID)
		{
//////////////////////////////////////////////////////////////////////////////////////////////
// user documents: ///////////////////////////////////////////////////////////////////////////

		case PATHID_FILE_COMSETUP:
			return this->localAppDataPath + PATHTOFILE_COMSETUP_SYS;			// local app data
		case PATHID_FILE_EDITCOLOR:
			return this->localAppDataPath + PATHTOFILE_EDITCOLORSAMPLE;			// local app data
		case PATHID_FILE_APPSTYLE:
			return this->localAppDataPath + PATHTOFILE_APPSTYLE_ID;				// local app data
		case PATHID_FILE_NAVROOT:
			return this->localAppDataPath + PATHTOFILE_NAVIGATION_FOLDER;		// local app data
		case PATHID_FILE_WINDOW_DATA:
			return this->localAppDataPath + PATHTOFILE_WINDOWPLACEMENT_DATA;	// local app data
		case PATHID_FILE_AUTOCOMPLETE:
			return this->localAppDataPath + PATHTOFILE_AUTOCOMPLETE_STRINGS;	// local app data
		case PATHID_FILE_AUTOSYNTAX:
			return this->localAppDataPath + PATHTOFILE_AUTOSYNTAX_SETTINGS;		// local app data
		case PATHID_FILE_SETTINGS:
			return this->localAppDataPath + PATHTOFILE_CNCSUITE_SETTINGS;		// local app data
		case PATHID_FILE_EXSETTINGS:
			return this->localAppDataPath + PATHTOFILE_EXTENDED_SETTINGS;		// local app data
		case PATHID_FILE_INTERNALSETUP:
			return this->localAppDataPath + PATHTOFILE_INTERNAL_SETTINGS;		// local app data
		case PATHID_FILE_USERTEXT:
			return this->localAppDataPath + PATHTOFILE_USERTEXT;				// local app data
		case PATHID_FILE_TABIMAGE:
			return this->localAppDataPath + PATHTOFILE_TABIMAGE;				// local app data
		case PATHID_FILE_TREEVIEWIMAGE:
			return this->localAppDataPath + PATHTOFILE_TREEVIEWIMAGE;			// local app data
		case PATHID_FILE_SEARCHSETUP:
			return this->localAppDataPath + PATHTOFILE_SEARCHSETTINGS;			// local app data
		case PATHID_FILE_DESCPROP_ONE:
			return this->localAppDataPath + PATHTOFILE_DESCPROPERTY_ONE;		// local app data
		case PATHID_FILE_DESCPROP_TWO:
			return this->localAppDataPath + PATHTOFILE_DESCPROPERTY_TWO;		// local app data
		case PATHID_FILE_DESCPROP_THREE:
			return this->localAppDataPath + PATHTOFILE_DESCPROPERTY_THREE;		// local app data
		case PATHID_FILE_FIRSTUSEEXAMPLE:
			return this->documentsPath + PATHTOFILE_FIRSTUSE_EXAMPLEFILE;		// documents
		case PATHID_FILE_HISTORY:
			return this->localAppDataPath + PATHTOFILE_FILEHISTORY_DATA;		// local app data
		case PATHID_FILE_EXCEPTIONINFO:
			return this->localAppDataPath + PATHTOFILE_LASTEXCEPTIONINFO;		// local app data
		case PATHID_FILE_HELPHTML_GERMAN:
			return this->localAppDataPath + PATHTOFILE_HELP_GERMAN;				// local app data
		case PATHID_FILE_HELPHTML_ENGLISH:
			return this->localAppDataPath + PATHTOFILE_HELP_ENGLISH;			// local app data


		case PATHID_FOLDER_CNCSUITE_USERFOLDER:
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_USERFOLDER;		// local app data
		case PATHID_FOLDER_CNCSUITE_DOCUMENTSFOLDER:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_DOCUMENTSFOLDER;		// documents
		case PATHID_FOLDER_CNCSUITE_APPDATA:
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_APPDATA;			// local app data
		case PATHID_FOLDER_CNCSUITE_APPDATA_SESSION:
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_APPDATA_SESSION;	// local app data
		case PATHID_FOLDER_CNCSUITE_APPDATA_SETTINGS:
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_APPDATA_SETTINGS;	// local app data
		case PATHID_FOLDER_CNCSUITE_APPDATA_STRINGS:
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_APPDATA_STRINGS;	// local app data
		case PATHID_FOLDER_CNCSUITE_PROJECTS:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_PROJECTS;			// documents
		case PATHID_FOLDER_CNCSUITE_SAMPLES:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_SAMPLES;				// documents
		case PATHID_FOLDER_CNCSUITE_USERSTYLES:
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_USERSTYLES;		// local app data
		case PATHID_FOLDER_CNCSUITE_USERSTYLES_EDITOR:
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_USERSTYLES_EDITOR;// local app data
		case PATHID_FOLDER_CNCSUITE_APPLICATIONFOLDER:
			return iString(PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER);				// deprecated
		case PATHID_FOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS:
			return iString(PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS);			// deprecated

//////////////////////////////////////////////////////////////////////////////////////////////
// local app data: ///////////////////////////////////////////////////////////////////////////
		case PATHID_FILE_FIRST_USE:
			return this->localAppDataPath + PATHTOFILE_FILEFORFIRSTUSE;
		case PATHID_FILE_ALREADYINSTALLED:
			return this->localAppDataPath + PATHTOFILE_ALREADYINSTALLED;

		case PATHID_FOLDER_CNCSUITE_LOCALAPPDATA:
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_LOCALAPPDATA;
		case PATHID_FOLDER_CNCSUITE_LOCALAPPDATA_CONTROL:
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_LOCALAPPDATA_CONTROL;
		case PATHID_FOLDER_CNCSUITE_EXECUTABLE_FOLDER:
#ifndef CNCSUITE_USERINSTALLATION
			return iString(PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER);
#else	
			return this->localAppDataPath + PATHTOFOLDER_CNCSUITE_USERFOLDER;
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
		default:
			iString uknwn;
			uknwn.setToFailedState();
			return uknwn;
		}
	}

	iString GetRoamingDataPath()
	{
		iString result;
		result.setToFailedState();

		auto bfpo = CreateBasicFPO();
		if (bfpo != nullptr)
		{
			TCHAR* path = nullptr;

			if (bfpo->GetKnownFolderPath(&path, FOLDERID_RoamingAppData))
			{
				result = path;
			}
			SafeDeleteArray(&path);
			SafeRelease(&bfpo);
		}
		return result;
	}

	iString GetLocalAppDataPath() {
		return this->localAppDataPath;
	}

	iString GetDocumentsPath() {
		return this->documentsPath;
	}


private:
	iString documentsPath;
	iString localAppDataPath;

	void setUserDirPath()
	{
		TCHAR* path = nullptr;

		auto bufSize = GetEnvironmentVariable(L"LOCALAPPDATA", nullptr, 0);
		if (bufSize > 0)
		{
#if defined(_WIN64)
			__int64 tmp_val = bufSize;
			tmp_val++;
			path = new TCHAR[tmp_val];
#else
			path = new TCHAR[bufSize + 1];
#endif

			if (path != nullptr)
			{
				GetEnvironmentVariable(L"LOCALAPPDATA", path, bufSize + 1);

				this->localAppDataPath = path;

				SafeDeleteArray(&path);
			}
		}
	}
};
