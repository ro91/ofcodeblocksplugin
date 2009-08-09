#include "InstallParser.h"
#include <sdk.h> // Code::Blocks SDK
#include <logmanager.h>
#include <string>

InstallParser::InstallParser(std::string sInstallFile):install_file(sInstallFile) {

}

void InstallParser::setFile(std::string sInstallFile) {
	install_file = sInstallFile;
}

std::string InstallParser::getOS() {
	return "win32";
}

bool InstallParser::parse() {
	Manager::Get()->GetLogManager()->Log(_T("Start parsing install file"));
	TiXmlDocument doc(install_file.c_str());
	if (!doc.LoadFile()) {
		Manager::Get()->GetLogManager()->Log(_T("Cannot load xml file."));
		return false;
	}

	Manager::Get()->GetLogManager()->Log(_T("Successfully loaded the install file"));
	TiXmlElement* root = doc.RootElement();
	if (strcmp(root->Value(), "install") != 0) {
		Manager::Get()->GetLogManager()->Log(_T("The install file is not formatted correctly"));
		return false;
	}

	// Files to be added to the project
	//-----------------------------------------------------------------
	TiXmlHandle handle(&doc);
	TiXmlElement* add = handle
		.FirstChild("install")
		.FirstChild("add")
		.FirstChild("src")
		.FirstChild("folder")
		.ToElement();
	if (add) {
		Manager::Get()->GetLogManager()->Log(_T("Add some files to the current project."));
		while(add) {
			wxString folder(add->Attribute("name"),wxConvUTF8);
			Manager::Get()->GetLogManager()->Log(folder);
			TiXmlElement* file_el = add->FirstChild()->ToElement();
			while(file_el) {
				if (file_el) {
					wxString file(file_el->GetText(), wxConvUTF8);
					struct FileInfo project_file = {file};
					project_files.push_back(project_file);
				//	Manager::Get()->GetLogManager()->Log(file);
				}
				else {
					Manager::Get()->GetLogManager()->Log(_T("no file found"));
				}
				file_el = file_el->NextSiblingElement();
			}
			add = add->NextSiblingElement();
		}
	}


	// @todo keep track of the includes
	// Include dirs to be added.
	//-----------------------------------------------------------------
	TiXmlElement* includes = handle
		.FirstChild("install")
		.FirstChild("add")
		.FirstChild("include")
		.FirstChild("path")
		.ToElement();

	if (includes) {
		while(includes) {
			wxString include_dir(includes->GetText(),wxConvUTF8);
			Manager::Get()->GetLogManager()->Log(include_dir);
			include_dirs.push_back(include_dir);
			includes = includes->NextSiblingElement();
		}
	}
	else {
		Manager::Get()->GetLogManager()->Log(_T("No includes dir found."));
	}

	// @todo keep track of the linker files
	// Linker values.
	//-----------------------------------------------------------------
	std::string os = getOS();
	TiXmlElement* link = handle
		.FirstChild("install")
		.FirstChild("add")
		.FirstChild("link")
		.FirstChild("lib")
		.ToElement();

	while(link) {
		if(strcmp(link->Attribute("os"),os.c_str()) == 0
			&& std::string(link->Attribute("compiler")).find("codeblocks") != std::string::npos)
		{
			wxString link_file(link->GetText(),wxConvUTF8);
			link_libs.push_back(link_file);
			Manager::Get()->GetLogManager()->Log(link_file);
		}
		link = link->NextSiblingElement();
	}

	// Shared objects.
	//-----------------------------------------------------------------
	/*
	Manager::Get()->GetLogManager()->Log(_T("Add linker files."));
	std::string os = getOS();
	TiXmlElement* link = handle
		.FirstChild("install")
		.FirstChild("add")
		.FirstChild("link")
		.FirstChild("lib")
		.ToElement();

	while(link) {
		if(strcmp(link->Attribute("os"),os.c_str()) == 0
			&& std::string(link->Attribute("compiler")).find("codeblocks") != std::string::npos)
		{
			wxString link_file(link->GetText(),wxConvUTF8);
			Manager::Get()->GetLogManager()->Log(link_file);
		}
		link = link->NextSiblingElement();
	}
	*/

	// Required addons
	TiXmlElement* required = handle
		.FirstChild("install")
		.FirstChild("requires")
		.ToElement();
	if(required) {
		wxString required_addon(required->GetText(),wxConvUTF8);
		required_addons.push_back(required_addon);
	}



}

std::vector<FileInfo> InstallParser::getProjectFiles() {
	return project_files;
}

std::vector<wxString> InstallParser::getRequiredAddons() {
	return required_addons;
}

std::vector<wxString> InstallParser::getIncludeDirs() {
	return include_dirs;
}
std::vector<wxString> InstallParser::getLinkLibs() {
	return link_libs;
}


