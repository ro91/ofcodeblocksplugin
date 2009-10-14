#include "AddonDialog.h"

#include "cbproject.h"
#include "configmanager.h"
#include "projectmanager.h"
#include <map>

AddonDialog::AddonDialog(wxWindow* pParent,wxWindowID nID, const wxString& sTitle):wxDialog(pParent, nID, sTitle) {
	int max_w = 420;
	SetSize(wxSize(max_w,500));
	max_w -= 10;
	ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("openframeworks"));
	wxString saved_path = _T("");

	if (cfg) {
		saved_path = cfg->Read(_T("of_dir"),_T("You need to select the openFrameworks directory."));
	}

	of_dir_box = new wxStaticBox(
		this
		,wxNewId()
		,_T("openFrameworks location")
		,wxPoint(10,10)
		,wxSize(385, 60)
	);

	of_dir_txt = new wxTextCtrl(
		this
		,wxNewId()
		,saved_path
		,wxPoint(20,35)
		,wxSize(330,20)
	);
	of_dir_txt->SetEditable(false);

	of_dir_btn = new wxButton(
		this
		,OF_DIR_BUTTON
		,_T("...")
		,wxPoint(355,35)
		,wxSize(25,20)
	);

	Connect(
		OF_DIR_BUTTON
		,wxEVT_COMMAND_BUTTON_CLICKED
		,wxCommandEventHandler(AddonDialog::onOFDirButtonClick)
	);

	of_addon_box = new wxStaticBox(
		this
		,wxNewId()
		,_T("Select addons for your project")
		,wxPoint(10,80)
		,wxSize(385, 330)
	);

	of_addons_list = new wxCheckListBox(
		this
		,wxNewId()
		,wxPoint(20,105)
		,wxSize(360,280)
	);

	// Add selected addons to project.
	of_ok_btn = new wxButton(
		this
		,OF_OK_BUTTON
		,_T("Add addons to project")
		,wxPoint(10,420)
	);

	Connect(
		OF_OK_BUTTON
		,wxEVT_COMMAND_BUTTON_CLICKED
		,wxCommandEventHandler(AddonDialog::onOkButtonClick)
	);

	of_cancel_btn = new wxButton(
		this
		,OF_CANCEL_BUTTON
		,_T("Close")
		,wxPoint(GetSize().GetWidth()-100,of_ok_btn->GetPosition().y)
	);
	Connect(
		OF_CANCEL_BUTTON
		,wxEVT_COMMAND_BUTTON_CLICKED
		,wxCommandEventHandler(AddonDialog::onCancelButtonClick)
	);

	setOFDir(saved_path);

	// setOFDir fills and checks the addons checkboxes;
	// when they're filled we get the currently selected ones.
	prev_selected_addons = getSelectedAddons();
}

AddonDialog::~AddonDialog() {
	delete of_dir_txt;
	delete of_dir_btn;
	delete of_ok_btn;
	delete of_cancel_btn;
	delete of_dir_box;
	delete of_addon_box;
	delete of_addons_list;
}

void AddonDialog::onOFDirButtonClick(wxCommandEvent & WXUNUSED(oEv)) {
	Manager::Get()->GetLogManager()->Log(_T("ofDirSelect clicked"));
	showOFDirDialog();
}

void AddonDialog::onOkButtonClick(wxCommandEvent & WXUNUSED(oEv)) {
	Manager::Get()->GetLogManager()->Log(_T("OK clicked."));
	Manager::Get()->GetLogManager()->Log(wxString::Format(wxT("total %i"),of_addons_list->GetCount()));

	// close dialog.
	EndModal(wxID_OK);
}

void AddonDialog::onCancelButtonClick(wxCommandEvent & WXUNUSED(oEv)) {
	EndModal(wxID_CLOSE);
}

wxString AddonDialog::getAddonInstallFile(wxString sForAddon) {
	for (int i = 0; i < addons.size(); ++i) {
		if (addons[i].name == sForAddon) {
			return addons[i].install_file;
		}
	}
	return _T("");
}

void AddonDialog::showOFDirDialog() {
	// get last selected directory.
	ConfigManager* cfg = Manager::Get()->GetConfigManager(_T("openframeworks"));
	wxString saved_path = _T("");
	if (cfg) {
		saved_path = cfg->Read(_T("of_dir"),_T(""));
	}

	// select new directory.
	wxDirDialog dir_dlg(this, _T("Select the openFramworks directory"), saved_path);
	if (dir_dlg.ShowModal() == wxID_OK) {
		wxString selected_path = dir_dlg.GetPath();
		wxDir dir(selected_path);
		// did the user select the correct oF directory?
		if (dir.HasSubDirs(_T("addons"))
			&& dir.HasSubDirs(_T("apps"))
			&& dir.HasSubDirs(_T("libs"))
		)
		{
			setOFDir(selected_path);
		}
		else {
			// incorrect oF directory, show error.
			wxMessageDialog *err = new wxMessageDialog(
				NULL
				,wxT("Incorrect openFrameworks location.\nPlease select the 'base' directory where you unpakked openFrameworks.\nThis directory contains the addons, libs, apps, etc. directories.")
				,wxT("Error")
				,wxOK | wxICON_ERROR
			);
		   	err->ShowModal();
		   	delete err;
		}

		// store the selected path.
		if (cfg) {
			Manager::Get()->GetLogManager()->Log(_T("Saving ofdir"));
			cfg->Write(_T("of_dir"), selected_path, false);
		}
	}
}

void AddonDialog::setOFDir(wxString sDir) {
	of_dir = sDir;
	addons.clear();

	// find addons in given directory.
	wxString addon_path = 	sDir
							+ wxFileName::GetPathSeparator()
							+_T("addons")
							+wxFileName::GetPathSeparator();
	findAddons(addon_path);
	wxArrayString addons_array;
	for(int i = 0; i < addons.size(); ++i) {
		addons_array.Add(addons[i].name);
	}

	// clear the checkbox list and insert the new items.
	of_addons_list->Clear();
	of_addons_list->InsertItems(addons_array,0);

	// find addons which are used in the project alreay.
	std::vector<AddonFile> used_addons = getAddonsUsedInCurrentProject();
	for (int i = 0; i < used_addons.size(); ++i) {
		wxString used_addon = used_addons.at(i).name;
		for(int j = 0; j < of_addons_list->GetCount();++j) {
			if (of_addons_list->GetString(j) == used_addon) {
				of_addons_list->Check(j);
			}
		}
	}
}

std::vector<AddonFile> AddonDialog::findAddons(wxString inDir) {
	wxDir dir(inDir);
	Manager::Get()->GetLogManager()->Log(_T("Finding addons in"));
	Manager::Get()->GetLogManager()->Log(inDir);

	wxString file;
	bool cont = dir.GetFirst(&file, wxEmptyString, wxDIR_DIRS);
	while(cont) {
		wxString addon_dir_name = inDir;
		addon_dir_name.append(file);
		wxDir addon_dir(addon_dir_name);
		if (addon_dir.HasFiles(_T("install.xml"))) {
			AddonFile addon_info = {
				file
				,addon_dir_name
				,addon_dir_name
				+wxFileName::GetPathSeparator()
				+_T("install.xml")
			};
			addons.push_back(addon_info );
		}
		cont = dir.GetNext(&file);
	}
	return addons;
}

std::vector<AddonFile> AddonDialog::getFoundAddons() {
	return addons;
}

std::vector<AddonFile> AddonDialog::getDeselectedAddons() {
	std::vector<AddonFile> prev_selected 	= getPreviouslySelectedAddons();
	std::vector<AddonFile> new_selected 	= getSelectedAddons();
	std::vector<AddonFile> deselected;

	for(std::vector<AddonFile>::const_iterator p = prev_selected.begin(); p != prev_selected.end(); ++p) {
		bool still_selected = false;
		for(std::vector<AddonFile>::const_iterator k = new_selected.begin(); k != new_selected.end(); ++k) {
			if (p->name == k->name) {
				still_selected = true;
				break;
			}
		}
		if (!still_selected) {
			deselected.push_back(*p);
		}
	}
	return deselected;
}

std::vector<AddonFile> AddonDialog::getNewSelectedAddons() {
	std::vector<AddonFile> selected 	= getSelectedAddons();
	std::vector<AddonFile> used 		= getAddonsUsedInCurrentProject();
	std::vector<AddonFile> new_selected;

	for(int i = 0; i < selected.size(); ++i) {
		bool is_new_addon = true;
		for(int j = 0; j < used.size(); ++j) {
			if (used.at(j).name == selected.at(i).name) {
				is_new_addon = false;
				break;
			}
		}
		if (is_new_addon) {
			new_selected.push_back(selected.at(i));
		}
	}
	return new_selected;
}

std::vector<AddonFile> AddonDialog::getPreviouslySelectedAddons() {
	return prev_selected_addons;
}

// returns a vector with all the selected addons in the checkbox list.
std::vector<AddonFile> AddonDialog::getSelectedAddons() {
	std::vector<AddonFile> sel_addons;
	for(int j = 0; j < of_addons_list->GetCount();++j) {
		if (of_addons_list->IsChecked(j)) {
			AddonFile addon_to_add = {
				of_addons_list->GetString(j)
				,_T("")
				,getAddonInstallFile(of_addons_list->GetString(j))
			};
			sel_addons.push_back(addon_to_add);
		}
	}
	return sel_addons;
}

std::vector<AddonFile> AddonDialog::getAddonsUsedInCurrentProject() {
	ProjectManager* project_mng = Manager::Get()->GetProjectManager();
    cbProject* project = project_mng->GetActiveProject();
	ProjectAnalyzer* analyzer = new ProjectAnalyzer();
	analyzer->analyze(project);

	std::vector<AddonFile> used_addons = analyzer->getUsedAddons();
	delete analyzer;
	return used_addons;
}
