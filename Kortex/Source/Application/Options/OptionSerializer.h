#pragma once
#include "stdafx.h"
class KxDataViewCtrl;
class KxSplitterWindow;
class wxTopLevelWindow;

namespace KxDataView2
{
	class View;
}

namespace Kortex
{
	class AppOption;
	class IWorkspaceContainer;
}

namespace Kortex::Application::OptionSerializer
{
	enum class SerializationMode
	{
		Save,
		Load
	};
}

namespace Kortex::Application::OptionSerializer
{
	class UILayout
	{
		public:
			static void DataViewLayout(AppOption& option, SerializationMode mode, KxDataViewCtrl* dataView);
			static void DataView2Layout(AppOption& option, SerializationMode mode, KxDataView2::View* dataView);
			static void SplitterLayout(AppOption& option, SerializationMode mode, KxSplitterWindow* window);
			static void WorkspaceContainerLayout(AppOption& option, SerializationMode mode, IWorkspaceContainer& container);
			static void WindowGeometry(AppOption& option, SerializationMode mode, wxTopLevelWindow* window);
	};
}
