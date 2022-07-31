#pragma once
#ifndef _CNCSUITE_EXTERNAL_DEPENDENCIES_
#define _CNCSUITE_EXTERNAL_DEPENDENCIES_

#include<sdkddkver.h>		// TARGET VERIFICATION

#define		WIN32_LEAN_AND_MEAN		// EXCLUDE RARELY USED WINDOWS PARTS

// LIBRARIES >>

#pragma comment(lib,"Wininet.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shcore.lib")
#pragma comment(lib, "Setupapi.lib")
#pragma comment(lib, "Dwmapi.lib")

//	EXTERNAL DEPENDENCIES >>

//	WINDOWS:
#include<Windows.h>
#include<windowsx.h>
#include<Shlwapi.h>
#include<shellapi.h>
#include<ShlObj.h>
#include<ShObjIdl.h>
#include<Unknwn.h>
#include<Richedit.h>
#include<WinInet.h>
#include<commdlg.h>
#include<sal.h>
#include<dwmapi.h>

// C-RUNTIME:
#include<stdio.h>
#include<stdarg.h>
#include<tchar.h>
#include<strsafe.h>
#include<math.h>
#include<malloc.h>

// SPECIFIC DEPENDENCIES
#include<shellscalingapi.h>
#include<SetupAPI.h>

// OWN:
#include"Resource.h"
#include"ExtendedResources.h"
#include"Global_String_IDs.h"
#include"dataAccessorID.h"
#include"CnC Suite.h"
#include"StringProcessHelper.h"
#include"cObject.h"
//#include"Global.h"
//#include"GlobalStyles.h"

#endif// _CNCSUITE_EXTERNAL_DEPENDENCIES_


