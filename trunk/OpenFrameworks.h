/***************************************************************
 * Name:      OpenFrameworks
 * Purpose:   Code::Blocks plugin
 * Author:    Diederick (diederick@apollomedia.nl)
 * Created:   2009-07-28
 * Copyright: Diederick
 * License:   GPL
 **************************************************************/

#ifndef OpenFrameworks_H_INCLUDED
#define OpenFrameworks_H_INCLUDED

// For compilers that support precompilation, includes <wx/wx.h>
#include <wx/wxprec.h>

#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <cbplugin.h> // for "class cbToolPlugin"
#include <tinyxml.h>
#include <vector>

#include "InstallParser.h"
#include "AddonDialog.h"

/**
 * This is the main class for the openFrameworks plugin which
 * let you easily install/add new addons to your openFramework
 * project.
 *
 * This plugin uses the install.xml files of the addons to add
 * addons to your project.
 *
 * Please contact me for questions or improvements!
 *
 * Note:
 * ------------------------------------------------------------
 * 1) Creating a .cbplugin file
 * ------------------------------------------------------------
 * There were some things I needed to change/create when I started
 * this plugin. On of the things were the creation of the installable
 * plugin for code::blocks. To create a .cbplugin file you need to
 * package your plugin in a zip. On windows you can do this by
 * adding these lines to you post build options:
 *
 * "C:\Program Files\7-Zip\7z.exe" a -tzip OpenFrameworks.zip manifest.xml
 * "C:\Program Files\7-Zip\7z.exe" a -tzip OpenFrameworks.cbplugin OpenFrameworks.dll OpenFrameworks.zip
 *
 * We used the opensource 7-zip here.
 *
 * ------------------------------------------------------------
 * 2) Linking against the correct file..
 * ------------------------------------------------------------
 * While developing I used the svn version. But when I wanted to
 * test this plugin I had to link it against the installed code::blocks
 * on my computer if I remember correctly.
 * See the build options > search directories.
 *
 *
 * ------------------------------------------------------------
 * 3) Link order
 * ------------------------------------------------------------
 * I had some trouble fixing the install.xml of ofxOsc. Somehow it
 * wouldnt link agains ws2_32 on windows. The "oscpack.a" linkage must
 * be above the -ws2_32 linker options. Therefore I changed the order
 * at which the libs are added to the linker options. All libs are
 * added to the top of the libs. So In your install.xml you need to
 * put the lib which must be linked against first at the bottom
 * of the <link> items.
 *
 * @author      Diederick Huijbers <diederick@apollomedia.nl
 *
 */
class OpenFrameworks : public cbToolPlugin
{
    public:
        /** Constructor. */
        OpenFrameworks();
        /** Destructor. */
        virtual ~OpenFrameworks();

        /** @brief Execute the plugin.
          *
          * This is the only function needed by a cbToolPlugin.
          * This will be called when the user selects the plugin from the "Plugins"
          * menu.
          */
        virtual int Execute();

        void addAddons(std::vector<AddonFile> oAddons, std::vector<AddonFile> oAvailableAddons);
        void removeAddons(std::vector<AddonFile> oAddons);

    protected:
        /** Any descendent plugin should override this virtual method and
          * perform any necessary initialization. This method is called by
          * Code::Blocks (PluginManager actually) when the plugin has been
          * loaded and should attach in Code::Blocks. When Code::Blocks
          * starts up, it finds and <em>loads</em> all plugins but <em>does
          * not</em> activate (attaches) them. It then activates all plugins
          * that the user has selected to be activated on start-up.\n
          * This means that a plugin might be loaded but <b>not</b> activated...\n
          * Think of this method as the actual constructor...
          */
        virtual void OnAttach();

        /** Any descendent plugin should override this virtual method and
          * perform any necessary de-initialization. This method is called by
          * Code::Blocks (PluginManager actually) when the plugin has been
          * loaded, attached and should de-attach from Code::Blocks.\n
          * Think of this method as the actual destructor...
          * @param appShutDown If true, the application is shutting down. In this
          *         case *don't* use Manager::Get()->Get...() functions or the
          *         behaviour is undefined...
          */
        virtual void OnRelease(bool appShutDown);

	private:

};

#endif // OpenFrameworks_H_INCLUDED
