#pragma once

#include <wx/wx.h>
#include <wx/snglinst.h>

class App : public wxApp
{
public:
    bool OnInit() override;

private:
    wxSingleInstanceChecker m_instanceChecker;
};
