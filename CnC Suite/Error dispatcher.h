#pragma once
#include "external.h"
#ifndef  _ERROR_DISPATCHER_H_
#define  _ERROR_DISPATCHER_H_

#define		EDSP_ERROR		0
#define		EDSP_WARNING	1
#define		EDSP_INFO		2

typedef struct _EDSPSTRUCT {
	int type;
	LPCTSTR Code;
	LPCTSTR Description;
	LPCTSTR Location;
}EDSPSTRUCT, *LPEDSPSTRUCT;

// displays messages in the codebox - if one of the pointer is null, nothing is displayed
BOOL DispatchEWINotification(int type, LPCTSTR Code, LPCTSTR Description, LPCTSTR Location);
// displays a system error translated in the system-language
void DispatchSystemError();

#endif  //_ERROR_DISPATCHER_H_

// NOTIFICATION STRUCTURE (example)		TC 0001		TC == COMPONENT /	0001 == ERROR INDEX

// APPLICATION CORE
// AC0001	- filetype not supported				// error
// AC0002	- export failed							// error
// AC0003	- opening file failed					// error
// AC0004	- importing file failed					// error

// TABCONTROL
// TC0001	- maximum Tab-count reached				// info
// TC0002	- last Tab cannot be closed				// info
// TC0003	- not all tabs got a valid target-path	// warning
// TC0004	- file is already open					// info
// TC0005	- cnc3file object error					// error

// USERINTERFACE
// UI0001	- ??

// FILENAVIGATOR
// FN0001	- File was successful converted		// info
// FN0002	- File was successful imported		// info
// FN0003	- Filename invalid					// error
// FN0004	- Rootfolder invalidated at runtime // error
// FN0005	- Rootfolder loading failed			// error
// FN0006	- Driveloading not permitted		// error
// FN0007	- Error opening root folder			// error

// UPDATEAGENT
// UA0001	- An update is available			// info
// UA0002	- searching for updates failed		// warning

// SAMPLEMANAGER
// SMP001	- Maximum Filter levels reached		// info
// SMP002	- Samplename invalid				// error
// SMP003	- No Target selected				// info

// Settings-Window
// STP001	- app-restart is required			// info
// STP002	- no device is selected				// info

// Editcontol
// EDC0001	- Format Error (exception) in method  'FormatAll'	// error

// History Component
// HST0001	- No history content to display

// Data Exchange Window
// DEC0001 - No data to set to tab
