#include "ProjectAnalyzer.h"
#include "cbproject.h"
#include "configmanager.h"
#include "projectmanager.h"

#include <logmanager.h>
#include <string>
#include <wx/utils.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/tokenzr.h>



ProjectAnalyzer::ProjectAnalyzer() {
}

/**
 * This method checks which addons have been added to the project. We assume
 * that each addon has at least one file in the project.
 *
 */
void ProjectAnalyzer::analyze(cbProject* pProject) {
	addon_files.clear();

	if (!pProject) return;
	Manager::Get()->GetLogManager()->Log(_T("Project title:" ) +pProject->GetTitle());
	unsigned int file_count = pProject->GetFilesCount();
	if (file_count > 10000) return; // we wont have more than 10.000 files (we got sometimes 1323.2323 filescount... while testing)

	wxString project_path = pProject->GetBasePath();
	for(int i = 0; i < file_count; ++i) {
		ProjectFile* project_file = pProject->GetFile(i);
		if (project_file != NULL) {
			wxString base_name = project_file->GetBaseName();
			wxString addons_dir = wxFileName::GetPathSeparator() + _T("addons");
			if (base_name.Contains(addons_dir)) {
				wxString partial_dir = project_path +wxFileName::GetPathSeparator();
				wxStringTokenizer dir_tokens(project_file->GetBaseName(), wxFileName::GetPathSeparator());
				bool start = false;
				while(dir_tokens.HasMoreTokens()) {
					// Get next token and check if we are in the addons dir.
					wxString token = dir_tokens.GetNextToken();
					if (token.Contains(_T("addons"))) {
						start = true;
					}

					// Does the directory exists?
					partial_dir += token +wxFileName::GetPathSeparator();
					if (!wxDir::Exists(partial_dir) ) {
						continue;
					}

					// Check for the install file
					wxDir curr_dir(partial_dir);
					if (start && curr_dir.HasFiles(_T("install.xml"))) {
						AddonFile af = {token, curr_dir.GetName() +wxFileName::GetPathSeparator() +_T("install.xml")};
						addon_files.push_back(af);
					}
				}
			}
		}
	}
}

std::vector<AddonFile> ProjectAnalyzer::getUsedAddons() {
	return addon_files;
}
