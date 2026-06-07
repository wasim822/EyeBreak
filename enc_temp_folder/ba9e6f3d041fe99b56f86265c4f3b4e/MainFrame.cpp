


#include "MainFrame.h"
#include "EyeTimerFrame.h"
#include "BreakFrame.h"

// ui for main screen with info of the app 
MainFrame::MainFrame(const wxString& title)
    : wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, wxSize(400, 250)),
    m_timer(this)
{
    Bind(wxEVT_TIMER, &MainFrame::OnTimer, this);

	wxTextCtrl* textCtrl = new wxTextCtrl(this, wxID_ANY, "Focus on your work! I'll remind you to take breaks.", wxDefaultPosition, wxDefaultSize, wxTE_READONLY | wxTE_MULTILINE);

    m_state = State::WAIT_20_MIN;

    Hide();

    m_timer.Start(1000);
}

void MainFrame::OnTimer(wxTimerEvent&)
{
    if (m_state == State::WAIT_20_MIN)
    {
        m_20min--;

        if (m_20min <= 0)
        {
            m_timer.Stop();

            // For testing, use 5.
            // For real app, use 20 * 60.
            m_20min = 5;
            // m_20min = 20 * 60;

            m_cycles++;

            if (m_cycles < 4)
            {
                m_state = State::EYE_BREAK;

                eyeWindow = new EyeTimerFrame("Eye Break", m_20sec);

                eyeWindow->Bind(wxEVT_DESTROY, [this](wxWindowDestroyEvent&)
                    {
                        this->eyeWindow = nullptr;
                        this->m_20sec = 20;
                        this->m_state = State::WAIT_20_MIN;
                        this->m_timer.Start(1000);
                    });

                eyeWindow->Show();
            }
            else
            {
                m_state = State::LONG_BREAK;

                breakWindow = new BreakFrame("Long Break", m_5min);

                breakWindow->Bind(wxEVT_DESTROY, [this](wxWindowDestroyEvent&)
                    {
                        this->breakWindow = nullptr;
                        this->m_5min = 5 * 60;
                        this->m_state = State::WAIT_20_MIN;
                        this->m_timer.Start(1000);
                    });

                breakWindow->Show();

                m_cycles = 0;
            }
        }
    }
}