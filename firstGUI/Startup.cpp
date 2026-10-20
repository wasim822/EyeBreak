#include "Startup.h"
#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/msw/registry.h>

namespace
{
    const wxString RUN_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Run";

    // Where Windows' "Startup apps" settings page records apps the user
    // switched off. Our Run entry is ignored while it's marked disabled here.
    const wxString APPROVED_KEY = "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\StartupApproved\\Run";

    const wxString SETTINGS_KEY = "Software\\EyeBreak";
    const wxString VALUE_NAME = "EyeBreak";

    wxString StartupCommand()
    {
        return "\"" + wxStandardPaths::Get().GetExecutablePath() + "\" " STARTUP_FLAG;
    }

    bool DisabledInWindowsSettings()
    {
        wxRegKey key(wxRegKey::HKCU, APPROVED_KEY);
        wxMemoryBuffer data;

        if (!key.Exists() || !key.HasValue(VALUE_NAME) || !key.QueryValue(VALUE_NAME, data) || data.GetDataLen() == 0)
        {
            return false;
        }

        // An odd first byte (e.g. 0x03) means disabled; 0x02 means enabled.
        return (static_cast<unsigned char*>(data.GetData())[0] & 1) != 0;
    }
}

bool Startup::IsEnabled()
{
    // wxRegKey reports errors with popups; we handle failures ourselves.
    wxLogNull noLog;

    wxRegKey key(wxRegKey::HKCU, RUN_KEY);
    return key.Exists() && key.HasValue(VALUE_NAME) && !DisabledInWindowsSettings();
}

bool Startup::SetEnabled(bool enable)
{
    wxLogNull noLog;

    wxRegKey key(wxRegKey::HKCU, RUN_KEY);

    if (enable)
    {
        if (!key.Create() || !key.SetValue(VALUE_NAME, StartupCommand()))
        {
            return false;
        }

        // Clear any "disabled" mark left by Windows' Startup apps page.
        wxRegKey approved(wxRegKey::HKCU, APPROVED_KEY);

        if (approved.Exists() && approved.HasValue(VALUE_NAME))
        {
            approved.DeleteValue(VALUE_NAME);
        }

        return true;
    }

    if (!key.Exists() || !key.HasValue(VALUE_NAME))
    {
        return true;
    }

    return key.DeleteValue(VALUE_NAME);
}

void Startup::RefreshPath()
{
    wxLogNull noLog;

    wxRegKey key(wxRegKey::HKCU, RUN_KEY);
    wxString current;

    if (key.Exists() && key.HasValue(VALUE_NAME) && key.QueryValue(VALUE_NAME, current)
        && current != StartupCommand())
    {
        key.SetValue(VALUE_NAME, StartupCommand());
    }
}

bool Startup::WasAsked()
{
    wxLogNull noLog;

    wxRegKey key(wxRegKey::HKCU, SETTINGS_KEY);
    return key.Exists() && key.HasValue("AskedAboutStartup");
}

void Startup::SetAsked()
{
    wxLogNull noLog;

    wxRegKey key(wxRegKey::HKCU, SETTINGS_KEY);

    if (key.Create())
    {
        key.SetValue("AskedAboutStartup", 1L);
    }
}
