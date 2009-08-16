#ifndef INSTALLPARSERH
#define INSTALLPARSERH


#include <string>
#include <vector>

#include <tinyxml.h>
#include <wx/string.h>

struct FileInfo {
	wxString file;

};

class InstallParser {
	public:
		InstallParser(std::string sFile);
		void setFile(std::string sFile);
		std::string getOS();

		bool parse();
		std::vector<FileInfo> getProjectFiles();
		std::vector<wxString> getRequiredAddons();
		std::vector<wxString> getIncludeDirs();
		std::vector<wxString> getLinkLibs();

	private:
		std::string install_file;
		std::vector<FileInfo> project_files;
		std::vector<wxString> required_addons;
		std::vector<wxString> include_dirs;
		std::vector<wxString> link_libs;
};
#endif
