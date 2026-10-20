#include "MainFrame.h"
#include "BreakFrame.h"
#include "InfoFrame.h"
#include <wx/taskbar.h>
#include <wx/msw/wrapwin.h>

namespace
{
    // Longest time the timer sleeps between checks.
    const long MAX_CHECK_INTERVAL_MS = 30 * 1000;

    // System tray icon with a right-click menu, so the user can see the app
    // is running, open the info window, and exit it.
    class TrayIcon : public wxTaskBarIcon
    {
    public:
        explicit TrayIcon(MainFrame* owner)
            : m_owner(owner)
        {
            SetIcon(wxIcon("app_icon", wxBITMAP_TYPE_ICO_RESOURCE), "Eye Break");

            Bind(wxEVT_MENU, [this](wxCommandEvent&) { m_owner->ShowInfo(); }, wxID_ABOUT);
            Bind(wxEVT_MENU, [this](wxCommandEvent&) { m_owner->Quit(); }, wxID_EXIT);
            Bind(wxEVT_TASKBAR_LEFT_DCLICK, [this](wxTaskBarIconEvent&) { m_owner->ShowInfo(); });
        }

    protected:
        wxMenu* CreatePopupMenu() override
        {
            wxMenu* menu = new wxMenu();
            menu->Append(wxID_ABOUT, "Open Eye Break");
            menu->AppendSeparator();
            menu->Append(wxID_EXIT, "Exit Eye Break");
            return menu;
        }

    private:
        MainFrame* m_owner;
    };
}

MainFrame::MainFrame(const BreakSettings& settings)
    : wxFrame(nullptr, wxID_ANY, "Eye Break"),
    m_settings(settings),
    m_timer(this)
{
    Bind(wxEVT_TIMER, &MainFrame::OnTimer, this, m_timer.GetId());

#ifdef wxHAS_POWER_EVENTS
    Bind(wxEVT_POWER_SUSPENDED, &MainFrame::OnSuspended, this);
    Bind(wxEVT_POWER_RESUME, &MainFrame::OnResume, this);
#endif

    m_trayIcon = new TrayIcon(this);

    StartWorkPeriod();
}

unsigned int MainFrame::ShowInfoMessageId()
{
    static const UINT id = ::RegisterWindowMessageW(L"EyeBreak.ShowInfo");
    return id;
}

WXLRESULT MainFrame::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    if (message == ShowInfoMessageId())
    {
        ShowInfo();
        return 0;
    }

    return wxFrame::MSWWindowProc(message, wParam, lParam);
}

void MainFrame::ShowInfo(bool askAboutStartup)
{
    if (!m_infoWindow)
    {
        m_infoWindow = new InfoFrame(this, m_settings);
    }

    m_infoWindow->Show();
    m_infoWindow->Iconize(false);
    m_infoWindow->Raise();

    if (askAboutStartup)
    {
        // Ask once the window is on screen, so the question appears over it.
        m_infoWindow->CallAfter(&InfoFrame::AskAboutStartup);
    }
}

void MainFrame::Quit()
{
    m_timer.Stop();
    CloseBreakWindow();

    if (m_infoWindow)
    {
        m_infoWindow->Destroy();
    }

    if (m_trayIcon != nullptr)
    {
        m_trayIcon->RemoveIcon();
        m_trayIcon->Destroy();
        m_trayIcon = nullptr;
    }

    // Last top-level window is gone, so the app exits.
    Destroy();
}

void MainFrame::Pause()
{
    m_paused = true;
    m_timer.Stop();
    CloseBreakWindow();
}

void MainFrame::Resume()
{
    m_paused = false;
    RestartCycle();
}

void MainFrame::SetLongBreakSeconds(int seconds)
{
    m_settings.longBreakSeconds = seconds;
    m_settings.Save();
}

void MainFrame::StartWorkPeriod()
{
    m_breakDueAt = wxGetUTCTimeMillis() + (long)m_settings.workSeconds * 1000;
    ScheduleNextCheck();
}

void MainFrame::ScheduleNextCheck()
{
    wxLongLong now = wxGetUTCTimeMillis();
    wxLongLong remaining = m_breakDueAt - now;

    long delay = MAX_CHECK_INTERVAL_MS;

    if (remaining < delay)
    {
        delay = remaining > 0 ? remaining.ToLong() : 1;
    }

    m_expectedCheckAt = now + delay;
    m_timer.StartOnce(delay);
}

void MainFrame::OnTimer(wxTimerEvent&)
{
    wxLongLong now = wxGetUTCTimeMillis();

    // The timer fired much later than asked for: the PC was asleep or the
    // app was paused. Treat that time away as a break and start over.
    if (now - m_expectedCheckAt > (long)m_settings.sleepResetSeconds * 1000)
    {
        RestartCycle();
        return;
    }

    if (now >= m_breakDueAt)
    {
        ShowBreak();
    }
    else
    {
        ScheduleNextCheck();
    }
}

void MainFrame::ShowBreak()
{
    m_breakCount++;

    bool longBreak = m_breakCount % m_settings.breaksPerLongBreak == 0;

    // Called when the user closes the break or its countdown ends.
    auto onClosed = [this]()
        {
            m_breakWindow = nullptr;
            StartWorkPeriod();
        };

    if (longBreak)
    {
        m_breakCount = 0;

        m_breakWindow = new BreakFrame(
            "Time for a Screen Break",
            "Stand up, stretch, and drink some water.\nGive your eyes and body a reset.",
            "Small breaks help reduce fatigue",
            m_settings.longBreakSeconds,
            onClosed);
    }
    else
    {
        m_breakWindow = new BreakFrame(
            "Time for an Eye Break",
            "Every 20 minutes, look at something 20\nfeet away for 20 seconds.",
            "Reducing digital eye strain",
            m_settings.eyeBreakSeconds,
            onClosed);
    }

    m_breakWindow->Show();
}

// Removes the break window without running its onClosed callback.
void MainFrame::CloseBreakWindow()
{
    if (m_breakWindow != nullptr)
    {
        m_breakWindow->Destroy();
        m_breakWindow = nullptr;
    }
}

void MainFrame::RestartCycle()
{
    m_timer.Stop();
    CloseBreakWindow();
    m_breakCount = 0;
    StartWorkPeriod();
}

#ifdef wxHAS_POWER_EVENTS
void MainFrame::OnSuspended(wxPowerEvent& event)
{
    m_timer.Stop();
    CloseBreakWindow();

    event.Skip();
}

void MainFrame::OnResume(wxPowerEvent& event)
{
    if (!m_paused)
    {
        RestartCycle();
    }

    event.Skip();
}
#endif
