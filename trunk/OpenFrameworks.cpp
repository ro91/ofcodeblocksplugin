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

	// remove files.
	for(int i = 0; i < oAddons.size(); ++i) {
		std::string install_xml = std::string(oAddons.at(i).install_file.mb_str());
		parser->setFile(install_xml);
		parser->parse();

		std::vector<FileInfo>project_files = parser->getProjectFiles();
		wxArrayInt targets;

		project->BeginRemoveFiles();
		for(int j = 0; j < project_files.size(); ++j) {
			for(int k = 0; k < project->GetFilesCount(); ++k) {
				// Reformat to native path representation
				ProjectFile* project_file 	= project->GetFile(k);
				wxFileName wx_project_file 	= project_file->file;
				wx_project_file.MakeRelativeTo(project->GetBasePath());
				wxString rel_project_file 	= wx_project_file.GetPath() +wx_project_file.GetPathSeparator() +wx_project_file.GetFullName();
				wxFileName wx_addon_file 	= wxFileName(project_files.at(j).file);
				wxString rel_addon_file 	= wx_addon_file.GetPath() + wx_addon_file.GetPathSeparator() +wx_addon_file.GetFullName();

				// Is the current project file the same as the addon file?
				if (rel_project_file == rel_addon_file) {
					project->RemoveFile(k);
					Manager::Get()->GetLogManager()->Log(_T("Removed from project: ") +rel_addon_file);
				}
			}
		}
		// @todo remove linkers
		// @todo remove includes
		project->EndRemoveFiles();
	}
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
			//project->AddCompilerOption(includes.at(i));
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
