#pragma once

#include <wx/wx.h>

class EyeTimerFrame : public wxFrame
{
public:
    EyeTimerFrame(const wxString& title, int seconds);

private:
    wxTimer m_timer;
    int m_timeLeft;
    wxStaticText* m_countdownLabel;

    void OnTimer(wxTimerEvent& event);
};