#pragma once

#include <wx/wx.h>
#include <wx/spinctrl.h>
#include "BreakSettings.h"

class MainFrame;

// Window shown when the user opens the app (desktop icon or tray icon).
// Closing it only closes the window; the break timer keeps running.
class InfoFrame : public wxFrame
{
public:
    InfoFrame(MainFrame* owner, const BreakSettings& settings);

    // First-run question: "Start Eye Break when Windows starts?"
    void AskAboutStartup();

private:
    MainFrame* m_owner;
    BreakSettings m_settings;

    // Running / paused indicator
    wxPanel* m_statusPill;
    wxStaticText* m_statusLabel;
    wxButton* m_pauseBtn;

    // Schedule tiles
    wxStaticText* m_eyeBreakValue;
    wxStaticText* m_eyeBreakDetail;
    wxStaticText* m_longBreakValue;
    wxStaticText* m_longBreakDetail;

    wxSpinCtrl* m_longBreakSpin;
    wxCheckBox* m_startupCheck;

    void UpdateScheduleText();
    void UpdateStatus();
    void OnPauseClicked(wxCommandEvent& event);
    void OnLongBreakChanged(wxSpinEvent& event);
    void OnStartupToggled(wxCommandEvent& event);
    void OnStopClicked(wxCommandEvent& event);
};
