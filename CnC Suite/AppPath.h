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

#define			PATHID_FILE_DESCPROP_ONE	30
#define			PATHID_FILE_DESCPROP_TWO	31
#define			PATHID_FILE_DESCPROP_THREE	32

#define			PATHID_FILE_FIRST_USE			33
#define			PATHID_FILE_ALREADYINSTALLED	34
#define			PATHID_FILE_FIRSTUSEEXAMPLE		35

#define			PATHID_FILE_HISTORY			36


// important: this ID's must have successive numbers for iteration
#define			PATHID_FOLDER_CNCSUITE_USERFOLDER				100
#define			PATHID_FOLDER_CNCSUITE_APPDATA					101
#define			PATHID_FOLDER_CNCSUITE_APPDATA_SESSION			102
#define			PATHID_FOLDER_CNCSUITE_APPDATA_SETTINGS			103
#define			PATHID_FOLDER_CNCSUITE_APPDATA_STRINGS			104
#define			PATHID_FOLDER_CNCSUITE_PROJECTS					105
#define			PATHID_FOLDER_CNCSUITE_SAMPLES					106
#define			PATHID_FOLDER_CNCSUITE_USERSTYLES				107
#define			PATHID_FOLDER_CNCSUITE_USERSTYLES_EDITOR		108
#define			PATHID_FOLDER_CNCSUITE_LOCALAPPDATA				109
#define			PATHID_FOLDER_CNCSUITE_LOCALAPPDATA_CONTROL		110
#define			PATHID_ITERATION_FINAL							111

// Application Folder must be C:\Program Files (x86)\CnC Suite
// -> generated by installer
#define			PATHID_FOLDER_CNCSUITE_APPLICATIONFOLDER		150
#define			PATHID_FOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS	151


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
#define			PATHTOFILE_SEARCHSETTINGS					L"\\CnC Suite\\AppData\\srchset.sys"

#define			PATHTOFILE_DESCPROPERTY_ONE					L"\\CnC Suite\\AppData\\Strings\\Dproperty1.dat"
#define			PATHTOFILE_DESCPROPERTY_TWO					L"\\CnC Suite\\AppData\\Strings\\Dproperty2.dat"
#define			PATHTOFILE_DESCPROPERTY_THREE				L"\\CnC Suite\\AppData\\Strings\\Dproperty3.dat"

#define			PATHTOFILE_FIRSTUSE_EXAMPLEFILE				L"\\CnC Suite\\Projects\\ExampleProject1\\example one.cnc3"
//##############################################################################################################################
//##############################################################################################################################
// FILES in Local-App-Data Folder:  ############################################################################################
#define			PATHTOFILE_FILEFORFIRSTUSE					L"\\CnC Suite\\control\\fstuse.dat"
#define			PATHTOFILE_ALREADYINSTALLED					L"\\CnC Suite\\control\\lrdinst.dat"
//##############################################################################################################################
//##############################################################################################################################
// FOLDERS in Documents Folder: ################################################################################################
#define			PATHTOFOLDER_CNCSUITE_USERFOLDER				L"\\CnC Suite"
#define			PATHTOFOLDER_CNCSUITE_APPDATA					L"\\CnC Suite\\AppData"
#define			PATHTOFOLDER_CNCSUITE_APPDATA_SESSION			L"\\CnC Suite\\AppData\\Session"
#define			PATHTOFOLDER_CNCSUITE_APPDATA_SETTINGS			L"\\CnC Suite\\AppData\\Settings"
#define			PATHTOFOLDER_CNCSUITE_APPDATA_STRINGS			L"\\CnC Suite\\AppData\\Strings"
#define			PATHTOFOLDER_CNCSUITE_PROJECTS					L"\\CnC Suite\\Projects"
#define			PATHTOFOLDER_CNCSUITE_SAMPLES					L"\\CnC Suite\\Samples"
#define			PATHTOFOLDER_CNCSUITE_USERSTYLES				L"\\CnC Suite\\UserStyles"
#define			PATHTOFOLDER_CNCSUITE_USERSTYLES_EDITOR			L"\\CnC Suite\\UserStyles\\Editor"
#define			PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER			L"C:\\Program Files (x86)\\CnC Suite"
#define			PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS	L"C:\\Program Files (x86)\\CnC Suite\\fonts"
//##############################################################################################################################
//##############################################################################################################################
// FOLDERS in Local-App-Data Folder:  ##########################################################################################
#define			PATHTOFOLDER_CNCSUITE_LOCALAPPDATA				L"\\CnC Suite"
#define			PATHTOFOLDER_CNCSUITE_LOCALAPPDATA_CONTROL		L"\\CnC Suite\\control"



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
			return this->documentsPath + PATHTOFILE_COMSETUP_SYS;
		case PATHID_FILE_EDITCOLOR:
			return this->documentsPath + PATHTOFILE_EDITCOLORSAMPLE;
		case PATHID_FILE_APPSTYLE:
			return this->documentsPath + PATHTOFILE_APPSTYLE_ID;
		case PATHID_FILE_NAVROOT:
			return this->documentsPath + PATHTOFILE_NAVIGATION_FOLDER;
		case PATHID_FILE_WINDOW_DATA:
			return this->documentsPath + PATHTOFILE_WINDOWPLACEMENT_DATA;
		case PATHID_FILE_AUTOCOMPLETE:
			return this->documentsPath + PATHTOFILE_AUTOCOMPLETE_STRINGS;
		case PATHID_FILE_AUTOSYNTAX:
			return this->documentsPath + PATHTOFILE_AUTOSYNTAX_SETTINGS;
		case PATHID_FILE_SETTINGS:
			return this->documentsPath + PATHTOFILE_CNCSUITE_SETTINGS;
		case PATHID_FILE_EXSETTINGS:
			return this->documentsPath + PATHTOFILE_EXTENDED_SETTINGS;
		case PATHID_FILE_INTERNALSETUP:
			return this->documentsPath + PATHTOFILE_INTERNAL_SETTINGS;
		case PATHID_FILE_USERTEXT:
			return this->documentsPath + PATHTOFILE_USERTEXT;
		case PATHID_FILE_TABIMAGE:
			return this->documentsPath + PATHTOFILE_TABIMAGE;
		case PATHID_FILE_TREEVIEWIMAGE:
			return this->documentsPath + PATHTOFILE_TREEVIEWIMAGE;
		case PATHID_FILE_SEARCHSETUP:
			return this->documentsPath + PATHTOFILE_SEARCHSETTINGS;
		case PATHID_FILE_DESCPROP_ONE:
			return this->documentsPath + PATHTOFILE_DESCPROPERTY_ONE;
		case PATHID_FILE_DESCPROP_TWO:
			return this->documentsPath + PATHTOFILE_DESCPROPERTY_TWO;
		case PATHID_FILE_DESCPROP_THREE:
			return this->documentsPath + PATHTOFILE_DESCPROPERTY_THREE;
		case PATHID_FILE_FIRSTUSEEXAMPLE:
			return this->documentsPath + PATHTOFILE_FIRSTUSE_EXAMPLEFILE;
		case PATHID_FILE_HISTORY:
			return this->documentsPath + PATHTOFILE_FILEHISTORY_DATA;


		case PATHID_FOLDER_CNCSUITE_USERFOLDER:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_USERFOLDER;
		case PATHID_FOLDER_CNCSUITE_APPDATA:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_APPDATA;
		case PATHID_FOLDER_CNCSUITE_APPDATA_SESSION:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_APPDATA_SESSION;
		case PATHID_FOLDER_CNCSUITE_APPDATA_SETTINGS:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_APPDATA_SETTINGS;
		case PATHID_FOLDER_CNCSUITE_APPDATA_STRINGS:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_APPDATA_STRINGS;
		case PATHID_FOLDER_CNCSUITE_PROJECTS:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_PROJECTS;
		case PATHID_FOLDER_CNCSUITE_SAMPLES:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_SAMPLES;
		case PATHID_FOLDER_CNCSUITE_USERSTYLES:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_USERSTYLES;
		case PATHID_FOLDER_CNCSUITE_USERSTYLES_EDITOR:
			return this->documentsPath + PATHTOFOLDER_CNCSUITE_USERSTYLES_EDITOR;
		case PATHID_FOLDER_CNCSUITE_APPLICATIONFOLDER:
			return iString(PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER);
		case PATHID_FOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS:
			return iString(PATHTOFOLDER_CNCSUITE_APPLICATIONFOLDER_FONTS);

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
			path = new TCHAR[bufSize + 1];
			if (path != nullptr)
			{
				GetEnvironmentVariable(L"LOCALAPPDATA", path, bufSize + 1);

				this->localAppDataPath = path;

				SafeDeleteArray(&path);
			}
		}
	}
};
