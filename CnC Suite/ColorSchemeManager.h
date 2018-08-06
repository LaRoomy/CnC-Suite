#pragma once
#include"external.h"
#include"CommonControls\CustomButton.h"
#include"CommonControls\ItemCollection.h"
#include"CommonControls\ComboBox.h"
#include"CommonControls\SingleLineEdit.h"
#include"XML Parsing\XML_Parsing.h"
#include"EditControl.h"

#define		SCHEMEMANAGERCLASS		L"SCHEMEMANGERCLASS"

#define		CTRLID_EXIT				100
#define		CTRLID_CHOOSECOLOR		101
#define		CTRLID_SCHEMECOMBO		102
#define		CTRLID_NAMEEDIT			103
#define		CTRLID_TRIGGERCOMBO		104
#define		CTRLID_SAVESCHEME		105
#define		CTRLID_SHOWEDIT			106
#define		CTRLID_RESETTEXTCOLOR	107
#define		CTRLID_RESETBACKGROUND	108

#define		CMD_REINIT_COLORSCHEME			10
#define		CMD_REFRESH_COLORSCHEMELIST		11
#define		CMD_SELECT_NEWCOLORSCHEME		12


class colorSchemeManager
	: public ObjectRelease<colorSchemeManager>,
	public customButtonEventSink,
	public comboBoxEventSink,
	public XMLParsingEventSink
{
public:
	colorSchemeManager(HINSTANCE hInst);
	~colorSchemeManager();

	HRESULT init(HWND Parent, HWND mainWindow);

	void colorSchemeManager::onCustomButtonClick(cObject sender, CTRLID ctrlID){
		this->onButtonClicked(reinterpret_cast<CustomButton*>(sender), ctrlID);
	}

	void colorSchemeManager::onComboBoxSelectionChanged(cObject sender, int selectedIndex){
		this->onComboSelchanged(reinterpret_cast<comboBox*>(sender), selectedIndex);
	}

	void colorSchemeManager::ParsingCompleted(cObject sender, itemCollection<iXML_Tag>* documentStructure){

		if (colorSchemeManager::xmlToColorStruct(documentStructure, &this->editStyleColors))
		{
			// apply colors and set controls
			auto triggercombo
				= reinterpret_cast<comboBox*>(
					SendMessage(
						GetDlgItem(this->hwndSchemeManager, CTRLID_TRIGGERCOMBO),
						WM_GETWNDINSTANCE,
						0, 0)
					);
			if (triggercombo != nullptr)
			{
				auto schemecombo
					= reinterpret_cast<comboBox*>(
						SendMessage(
							GetDlgItem(this->hwndSchemeManager, CTRLID_SCHEMECOMBO),
							WM_GETWNDINSTANCE,
							0, 0)
						);
				if (schemecombo != nullptr)
				{
					auto editbox
						= reinterpret_cast<singleLineEdit*>(
							GetWindowLongPtr(
								GetDlgItem(this->hwndSchemeManager, CTRLID_NAMEEDIT),
								GWLP_USERDATA)
							);
					if (editbox != nullptr)
					{
						editbox->setContent(
							schemecombo->Items->GetAt(
								schemecombo->getSelectedIndex()
							)
						);

						this->currentColorForCharacter = this->colorFromIndex(
							triggercombo->getSelectedIndex()
						);
						this->editControl->ConfigureComponent(&this->editStyleColors, &this->editControlProperties, TRUE, TRUE);
					}
				}
			}
		}
		// release parser
		auto parser = reinterpret_cast<XML_Parser*>(sender);
		SafeRelease(&parser);
	}
	void colorSchemeManager::ParsingFailed(cObject sender, LPPARSINGERROR pErr) {
		UNREFERENCED_PARAMETER(sender);
		UNREFERENCED_PARAMETER(pErr);
	}

	static bool loadSchemeList(itemCollection<iString>* list_out);
	static bool xmlToColorStruct(itemCollection<iXML_Tag>* xmlColorStructure, LPEDITSTYLECOLORS esc);
	static void indexToColor(int index, COLORREF color, LPEDITSTYLECOLORS esc);

private:
	HINSTANCE hInstance;
	HWND hwndSchemeManager;
	HWND parent;
	HWND MainWindow;
	HWND edit;

	EDITSTYLECOLORS editStyleColors;
	EDITCONTROLPROPERTIES editControlProperties;

	TCHAR currentTriggerCharacter;
	COLORREF currentColorForCharacter;

	APPSTYLEINFO sInfo;
	itemCollection<iString>* colorSchemes;

	EditControl* editControl;

	HRESULT registerSchemeManagerClass();
	HRESULT createControls();

	static LRESULT CALLBACK schemeProc(HWND, UINT, WPARAM, LPARAM);

	LRESULT onPaint(HWND);

	void onButtonClicked(CustomButton* sender, CTRLID ctrlID);
	void onComboSelchanged(comboBox* sender, int selIndex);

	void setTriggerCharFromSelIndex(int);
	void currentColorToEditbox();
	void setCurrentColorForCharacter();
	void resetText();
	void resetBackground();
	void redrawEditbox();

	COLORREF getColorFromUser(COLORREF);
	COLORREF colorFromIndex(int);
	bool saveCurrentScheme();

	void Close();
};