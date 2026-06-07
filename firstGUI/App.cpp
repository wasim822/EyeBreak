#include "App.h"
#include "MainFrame.h"
#include <wx/wx.h>

wxIMPLEMENT_APP(App);

bool App::OnInit() {

	MainFrame* mainFrame = new MainFrame("C++ GUI"); // window title 
	mainFrame->SetClientSize(800, 600); // window size

	bool openedFromStartup = false;

	mainFrame->Show();

	return true;

	
}

