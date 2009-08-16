#include <sdk.h> // Code::Blocks SDK
#include <configurationpanel.h>
#include "OpenFrameworks.h"
#include <logmanager.h>
#include <string>
#include <wx/utils.h>
#include <wx/filename.h>


#include "cbproject.h"
#include "configmanager.h"
#include "projectmanager.h"


namespace {
    PluginRegistrant<OpenFrameworks> reg(_T("OpenFrameworks"));
}

OpenFrameworks::OpenFrameworks() {
    if (!Manager::LoadResource(_T("OpenFrameworks.zip"))) {
        NotifyMissingFile(_T("OpenFrameworks.zip"));
    }
}

OpenFrameworks::~OpenFrameworks() {
}

void OpenFrameworks::OnAttach() {
}

void OpenFrameworks::OnRelease(bool appShutDown) {
}

int OpenFrameworks::Execute() {
    if (!IsAttached()) {
        return -1;
    }

    Manager::Get()->GetLogManager()->Log(_T("openFrameworks Addon"));

	// Add files to the project.
	ProjectManager* project_mng = Manager::Get()->GetProjectManager();
    cbProject* project = project_mng->GetActiveProject();

 	// Begin test: dialog
	int dlg_id = wxNewId();
	AddonDialog dlg(
		Manager::Get()->GetAppWindow()
		,dlg_id
		,wxString(_T("openFrameworks Addon Plugin"))
	);
	PlaceWindow(&dlg);

	if (dlg.ShowModal() == wxID_OK) {
		std::vector<AddonFile> selected_addons = dlg.getNewSelectedAddons();
		std::vector<AddonFile> deselected_addons = dlg.getDeselectedAddons();
		std::vector<AddonFile> available_addons = dlg.getFoundAddons();
		addAddons(selected_addons, available_addons);
		removeAddons(deselected_addons);
		project_mng->RebuildTree();

	}
	else {
		Manager::Get()->GetLogManager()->Log(_T("No addons selected."));
	}
    return 0;
}

void OpenFrameworks::removeAddons(std::vector<AddonFile> oAddons) {
	InstallParser* parser = new InstallParser("");
	ProjectManager* project_mng = Manager::Get()->GetProjectManager();
    cbProject* project = project_mng->GetActiveProject();
	std::vector<int> ids;

	// remove files.
	for(int i = 0; i < oAddons.size(); ++i) {
		std::string install_xml = std::string(oAddons.at(i).install_file.mb_str());
		parser->setFile(install_xml);
		parser->parse();

		std::vector<FileInfo>addon_files = parser->getProjectFiles();
		std::map<wxString, wxString> mapped_addon_files;
		wxArrayInt targets;

		// create a lookup table.
		std::vector<FileInfo>::const_iterator it = addon_files.begin();
		for(;it != addon_files.end();++it) {
			wxFileName addon_file 	= it->file;
			mapped_addon_files[addon_file.GetFullPath()] = addon_file.GetFullPath();
		}

		// now find the ids of the project files for the addon files.
		for(int k = 0; k < project->GetFilesCount(); ++k) {
			if (mapped_addon_files.find(project->GetFile(k)->relativeFilename) != mapped_addon_files.end()) {
				ids.push_back(k);
			}
		}
		// @todo remove linkers
		// @todo remove includes
	}

	// And remove all found project files
	project->BeginRemoveFiles();
	for(std::vector<int>::iterator p = ids.begin(); p != ids.end(); p++) {
		project->RemoveFile(*p);
	}
	project->EndRemoveFiles();
}

/**
 *
 *
 * @param	vector with addons to add.
 * @param	vector with all available addons (necessary for addon dependencies)
 */
void OpenFrameworks::addAddons(std::vector<AddonFile> oAddons, std::vector<AddonFile> oAvailableAddons) {
	InstallParser* parser = new InstallParser("");
	ProjectManager* project_mng = Manager::Get()->GetProjectManager();
    cbProject* project = project_mng->GetActiveProject();
	if(!project) {
		return;
	}

	for(int i = 0; i < oAddons.size(); ++i) {
		Manager::Get()->GetLogManager()->Log(_T("Add addon: ") +oAddons.at(i).name);
		std::string install_xml = std::string(oAddons.at(i).install_file.mb_str());
		parser->setFile(install_xml);
		parser->parse();

		// Add project files.
		std::vector<FileInfo>project_files = parser->getProjectFiles();
		wxArrayInt targets;
		for(int i = 0; i <project_files.size(); ++i) {
			project_mng->AddFileToProject(
				project_files[i].file
				,project
				,targets
			);
		}

		// add linkers. We add them at the "top" of the libs list. So
		// in your install.xml file, you'll need to add the libs in a
		// last-first order.
		std::vector<wxString> libs = parser->getLinkLibs();
		wxArrayString curr_link_opts =project->GetLinkerOptions();
		for(int i = 0; i < libs.size(); ++i) {
			curr_link_opts.Insert(libs.at(i),0);
		}
		project->SetLinkerOptions(curr_link_opts);

		// add includes.
		std::vector<wxString> includes = parser->getIncludeDirs();
		for(int i = 0; i < includes.size(); ++i) {
			project->AddIncludeDir(includes.at(i));
		}

		// Install required addons.
		std::vector<wxString> required = parser->getRequiredAddons();
		for(int k = 0; k < required.size(); k++) {
			for(int l = 0; l < oAvailableAddons.size(); l++) {
				if (oAvailableAddons.at(l).name == required.at(k)) {
					std::vector<AddonFile> add_required;
					add_required.push_back(oAvailableAddons.at(l));
					addAddons(add_required, oAvailableAddons);
					break;
				}
			}
		}
    }
}
