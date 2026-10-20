#pragma once

#include <wx/wx.h>
#include <wx/timer.h>
#include <wx/power.h>
#include <wx/weakref.h>
#include "BreakSettings.h"

class BreakFrame;
class InfoFrame;
class wxTaskBarIcon;

// Hidden window that runs the break schedule in the background.
class MainFrame : public wxFrame
{
public:
    explicit MainFrame(const BreakSettings& settings);

    // Opens the info window, or brings it to the front if already open.
    void ShowInfo(bool askAboutStartup = false);

    void Quit();

    // Pausing stops all reminders until Resume() starts a fresh cycle.
    void Pause();
    void Resume();
    bool IsPaused() const { return m_paused; }

    // Changes the long break length and saves it. Takes effect from the
    // next long break.
    void SetLongBreakSeconds(int seconds);

    // Message a second copy of the app sends to ask this one to ShowInfo().
    static unsigned int ShowInfoMessageId();

protected:
    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override;

private:
    void OnTimer(wxTimerEvent& event);

#ifdef wxHAS_POWER_EVENTS
    void OnSuspended(wxPowerEvent& event);
    void OnResume(wxPowerEvent& event);
#endif

    void StartWorkPeriod();
    void ScheduleNextCheck();
    void ShowBreak();
    void CloseBreakWindow();
    void RestartCycle();

private:
    BreakSettings m_settings;

    // One-shot timer. It wakes up at most every 30 seconds (or exactly when
    // the next break is due), so the app uses almost no CPU while waiting.
    wxTimer m_timer;

    // Wall-clock times in milliseconds (wxGetUTCTimeMillis).
    wxLongLong m_breakDueAt;
    wxLongLong m_expectedCheckAt;

    int m_breakCount = 0;

    bool m_paused = false;

    BreakFrame* m_breakWindow = nullptr;

    // Becomes null by itself when the user closes the info window.
    wxWeakRef<InfoFrame> m_infoWindow;

    wxTaskBarIcon* m_trayIcon = nullptr;
};
