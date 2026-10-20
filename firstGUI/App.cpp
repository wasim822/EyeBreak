#include "App.h"
#include "MainFrame.h"
#include "Startup.h"
#include <wx/msw/wrapwin.h>

wxIMPLEMENT_APP(App);

bool App::OnInit()
{
    // Windows passes this flag when it starts the app at sign-in. In that
    // case run silently; otherwise the user opened it, so show the info window.
    bool startedBySystem = false;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i] == STARTUP_FLAG)
        {
            startedBySystem = true;
        }
    }

    if (!m_instanceChecker.Create("EyeBreakSingleInstance") || m_instanceChecker.IsAnotherRunning())
    {
        if (!startedBySystem)
        {
            // Ask the copy that's already running to open its info window,
            // and let it come to the front.
            ::AllowSetForegroundWindow(ASFW_ANY);
            ::PostMessageW(HWND_BROADCAST, MainFrame::ShowInfoMessageId(), 0, 0);
        }

        return false;
    }

    // Needed once so the embedded PNG image can be loaded.
    wxImage::AddHandler(new wxPNGHandler());

    Startup::RefreshPath();

    BreakSettings settings;
    settings.Load();

    // Frame stays hidden; it only runs the timer and owns the tray icon.
    MainFrame* mainFrame = new MainFrame(settings);

    if (!startedBySystem)
    {
        // The first time the app is opened, ask about starting with Windows.
        mainFrame->ShowInfo(!Startup::WasAsked());
    }

    return true;
}
