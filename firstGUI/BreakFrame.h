#pragma once

#include <wx/wx.h>
#include <functional>

// Popup shown during a break. Used for both the short eye break and the
// long break; only the text and duration differ.
class BreakFrame : public wxFrame
{
public:
    BreakFrame(const wxString& title,
        const wxString& message,
        const wxString& footer,
        int seconds,
        std::function<void()> onClosed);

private:
    wxTimer m_timer;
    int m_timeLeft;
    bool m_showMinutes;
    wxStaticText* m_countdownLabel;
    std::function<void()> m_onClosed;

    wxString FormatTimeLeft() const;
    void OnTimer(wxTimerEvent& event);
    void OnClose(wxCloseEvent& event);
};
