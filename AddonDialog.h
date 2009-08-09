#ifndef ADDONDIALOGH
#define ADDONDIALOGH


#include <wx/wx.h>
#include <wx/window.h>
#include <wx/dialog.h>
#include <wx/string.h>
#include <wx/utils.h>
#include <wx/filepicker.h>
#include <wx/dir.h>
#include <wx/filefn.h>

//#include <cbplugin.h> // for "class cbToolPlugin"
#include <logmanager.h>
#include <configmanager.h>
#include <vector>

#include "cbproject.h"
#include "projectmanager.h"
#include "ProjectAnalyzer.h"

enum {
	OF_DIR_BUTTON = 2048
	,OF_OK_BUTTON
	,OF_CANCEL_BUTTON
};

struct AddonFile {
	wxString name;
	wxString install_file;
};

class AddonDialog : public wxDialog{
	public:
		AddonDialog(wxWindow* pParent, wxWindowID nID, const wxString& sTitle);
		~AddonDialog();

		void showOFDirDialog();
		void onOFDirButtonClick(wxCommandEvent & WXUNUSED(oEv));
		void onOkButtonClick(wxCommandEvent & WXUNUSED(oEv));
		void onCancelButtonClick(wxCommandEvent & WXUNUSED(oEv));
		void setOFDir(wxString sDir);
		std::vector<AddonFile> findAddons(wxString inDir);
		std::vector<AddonFile> getFoundAddons();
		std::vector<AddonFile> getSelectedAddons(); // gets the checked addons
		std::vector<AddonFile> getNewSelectedAddons(); // get the addons which have been added 'this time'
		std::vector<AddonFile> getDeselectedAddons();
		std::vector<AddonFile> getAddonsUsedInCurrentProject();

		wxString getAddonInstallFile(wxString sForAddon);

	private:
		wxWindow* parent_win;
		//wxStaticText* of_dir_txt;
		wxTextCtrl* of_dir_txt;
		wxButton* of_dir_btn;
		wxButton* of_ok_btn;
		wxButton* of_cancel_btn;
		wxStaticBox* of_dir_box;
		wxStaticBox* of_addon_box;
		wxCheckListBox* of_addons_list;
		wxString of_dir;
		std::vector<AddonFile> addons; // addons found with an install script.
		std::vector<AddonFile> selected_addons; // addons which should be installed.
};

#endif

