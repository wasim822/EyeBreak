#pragma once

#include <wx/wx.h>
#include <wx/timer.h>

class EyeTimerFrame;
class BreakFrame;

class MainFrame : public wxFrame
{
public:
    MainFrame(const wxString& title);

private:
    enum class State
    {
        WAIT_20_MIN,
        EYE_BREAK,
        LONG_BREAK
    };

    void OnTimer(wxTimerEvent& event);

private:
    wxTimer m_timer;

    State m_state = State::WAIT_20_MIN;

    int m_20min = 20 * 60;       // testing
    // int m_20min = 20 * 60; // real app

    int m_20sec = 20;
    int m_5min = 5 * 60;
    int m_cycles = 0;

    EyeTimerFrame* eyeWindow = nullptr;
    BreakFrame* breakWindow = nullptr;
};