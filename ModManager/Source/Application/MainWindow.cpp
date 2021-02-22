#include "pch.hpp"
#include "MainWindow.h"
#include "IApplication.h"
#include "BookSimpleWorkspaceContainer.h"
#include "Options/OptionDatabase.h"
#include <kxf/Drawing/SizeRatio.h>

namespace
{
	using SizeRatio = kxf::Geometry::SizeRatio;
}

namespace Kortex::Application
{
	MainWindow::MainWindow()
	{
		if (kxf::UI::Frame::Create(nullptr, wxID_NONE))
		{
			m_Frame = this;
			m_Frame->SetTitle(IApplication::GetInstance().GetName());

			// Load window geometry
			m_Frame->SetInitialSize(m_Frame->FromDIP(SizeRatio::FromWidth(1024, SizeRatio::r16_9)));
			ReadGlobalOption(OName::Geometry).LoadWindowGeometry(*m_Frame);

			// Create the UI
			m_WorkspaceContainer = new BookSimpleWorkspaceContainer(m_Frame);

			OnCreated();
		}
	}
	MainWindow::~MainWindow()
	{
		OnDestroyed();
	}
}
