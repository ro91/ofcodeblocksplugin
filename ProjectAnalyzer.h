#ifndef PROJECTANALYZERH
#define PROJECTANALYZERH

#include <cbplugin.h>
#include <vector>
#include "AddonDialog.h" // for AddonFile struct

/*
struct AddonFile {
	wxString name;
	wxString install_file;
};
*/
struct AddonFile;

class ProjectAnalyzer {
	public:
		ProjectAnalyzer();
		void analyze(cbProject* pProject);
		std::vector<AddonFile> getUsedAddons();

	private:
		std::vector<AddonFile> addon_files;
};

#endif
