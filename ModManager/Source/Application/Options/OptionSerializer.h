#pragma once
#include "Framework.hpp"

namespace kxf
{
	class IWidget;
}
namespace kxf::UI
{
	class SplitterWindow;
	namespace DataView
	{
		class View;
	}
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
	class KORTEX_API UILayout final
	{
		public:
			static void WidgetGeometry(AppOption& option, SerializationMode mode, kxf::IWidget& widget);
			static void DataViewLayout(AppOption& option, SerializationMode mode, kxf::UI::DataView::View& dataView);
			static void SplitterLayout(AppOption& option, SerializationMode mode, kxf::UI::SplitterWindow& window);
			static void WorkspaceContainerLayout(AppOption& option, SerializationMode mode, IWorkspaceContainer& container);
	};
}
