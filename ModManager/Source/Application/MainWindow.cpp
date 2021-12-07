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
	// IWidget
	bool MainWindow::CreateWidget(std::shared_ptr<IWidget> parent, const kxf::String& text, kxf::Point pos, kxf::Size size)
	{
		if (kxf::Widgets::Window::CreateWidget(nullptr, IApplication::GetInstance().GetName()))
		{
			m_Frame = this;

			// Load window geometry
			m_Frame->SetSize(m_Frame->FromDIP(SizeRatio::FromWidth(1024, SizeRatio::r16_9)), kxf::WidgetSizeFlag::Widget|kxf::WidgetSizeFlag::WidgetMin);
			ReadGlobalOption(OName::Geometry).LoadWidgetGeometry(*m_Frame);

			// Create the UI
			//m_WorkspaceContainer = new BookSimpleWorkspaceContainer(m_Frame);

			return true;
		}
		return false;
	}
}
