#include "stdafx.h"
#include "DisplayModel.h"
#include "Dialog.h"
#include "Archive/GenericArchive.h"
#include "VirtualFileSystem/IVFSService.h"
#include "Utility/UI.h"
#include <Kortex/Application.hpp>
#include <Kortex/PluginManager.hpp>
#include <KxFramework/KxHTMLWindow.h>
#include <KxFramework/KxWebSocket.h>
#include <KxFramework/KxTextFile.h>
#include <KxFramework/KxCrypto.h>
#include <KxFramework/KxJSON.h>
#include <KxFramework/KxCURL.h>
#include <KxFramework/KxINI.h>
#include <KxFramework/KxXML.h>

namespace
{
	using namespace KxDataView2;
	enum class ColumnRef
	{
		Name,
		Version,
		License,
		URL
	};
}

namespace Kortex::Application::About
{
	wxAny DisplayModel::GetValue(const Node& node, const Column& column) const
	{
		const INode& item = GetItem(node);
		switch (column.GetID<ColumnRef>())
		{
			case ColumnRef::Name:
			{
				return BitmapTextValue(item.GetName(), ImageProvider::GetBitmap(item.GetIconID()));
			}
			case ColumnRef::Version:
			{
				return item.GetVersion().ToString();
			}
			case ColumnRef::License:
			{
				if (item.HasLicense())
				{
					return ImageProvider::GetBitmap(ImageResourceID::Cheque);
				}
				break;
			}
			case ColumnRef::URL:
			{
				return item.GetURI().BuildUnescapedURI();
			}
		};
		return {};
	}
	bool DisplayModel::GetAttributes(const Node& node, const Column& column, const CellState& cellState, CellAttributes& attributes) const
	{
		const INode& item = GetItem(node);
		switch (column.GetID<ColumnRef>())
		{
			case ColumnRef::Version:
			{
				attributes.Options().SetAlignment(wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
				return true;
			}
			case ColumnRef::License:
			{
				if (cellState.IsHotTracked() && column.IsHotTracked() && item.HasLicense())
				{
					attributes.Options().Enable(CellOption::HighlightItem);
				}
				return true;
			}
			case ColumnRef::URL:
			{
				if (cellState.IsHotTracked())
				{
					KxColor linkColor = KxColor(GetView()->GetBackgroundColour()).GetContrastColor(KxColor(0, 0, 255), KxColor(25, 25, 100));
					attributes.Options().SetForegroundColor(linkColor);
					attributes.FontOptions().Enable(CellFontOption::Underlined);
					return true;
				}
				return false;
			}
		};
		return false;
	}

	void DisplayModel::OnActivateItem(Event& event)
	{
		if (event.GetNode() && event.GetColumn())
		{
			const INode& item = GetItem(*event.GetNode());
			switch (event.GetColumn()->GetID<ColumnRef>())
			{
				case ColumnRef::License:
				{
					wxString license = item.GetLicense();
					if (!license.IsEmpty())
					{
						KxHTMLWindow* window = m_Dialog.CreateHTMLWindow();
						window->SetValue(license);

						m_Dialog.CreateTemporaryTab(window,
													KxString::Format("%1 \"%2\"", KTr("Generic.License"), item.GetName()),
													ImageProvider::GetBitmap(item.GetIconID())
						);
					}
					break;
				}
				case ColumnRef::URL:
				{
					if (KxURI uri = item.GetURI())
					{
						Utility::UI::AskOpenURL(uri, GetView());
					}
					break;
				}
			};
		}
	}

	DisplayModel::DisplayModel(AboutDialog& dialog)
		:m_Dialog(dialog)
	{
	}

	void DisplayModel::RefreshItems()
	{
		m_DataVector.clear();

		// Add all modules
		IModule::ForEachModule([this](const IModule& module)
		{
			m_DataVector.emplace_back(std::make_unique<ModuleNode>(module));
		});

		// Add third party software
		AddSoftwareNode("wxWidgets", IApplication::GetInstance()->GetWxWidgetsVersion(), "https://www.wxwidgets.org", ImageResourceID::Block);

		IVFSService* vfsService = IVFSService::GetInstance();
		if (vfsService)
		{
			AddSoftwareNode(vfsService->GetLibraryName(), vfsService->GetLibraryVersion(), vfsService->GetLibraryURL(), ImageResourceID::Jar);
			if (vfsService->HasNativeLibrary())
			{
				AddSoftwareNode(vfsService->GetNativeLibraryName(), vfsService->GetNativeLibraryVersion(), vfsService->GetNativeLibraryURL(), ImageResourceID::Dokany);
			}
		}

		AddSoftwareNode(KxINI::GetLibraryName(), KxINI::GetLibraryVersion(), "https://github.com/brofield/simpleini", ImageResourceID::DocumentPencil);
		AddSoftwareNode(KxXMLDocument::GetLibraryName(), KxXMLDocument::GetLibraryVersion(), "https://github.com/leethomason/tinyxml2", ImageResourceID::EditCode);
		AddSoftwareNode("OpenSSL", KxCrypto::GetOpenSSLVersion(), "https://www.openssl.org", ImageResourceID::LockSSL);
		AddSoftwareNode("7-Zip", GenericArchive::GetLibraryVersion(), "https://www.7-zip.org", ImageResourceID::SevenZip);
		AddSoftwareNode(KxCURL::GetLibraryName(), KxCURL::GetLibraryVersion(), "https://curl.haxx.se", ImageResourceID::LibCURL);
		AddSoftwareNode(KxWebSocket::GetLibraryName(), KxWebSocket::GetLibraryVersion(), "https://github.com/zaphoyd/websocketpp", ImageResourceID::WebSocket);
		AddSoftwareNode(KxJSON::GetLibraryName(), KxJSON::GetLibraryVersion(), "https://github.com/nlohmann/json", ImageResourceID::JSON);

		if (PluginManager::LibLoot* lootAPI = PluginManager::LibLoot::GetInstance())
		{
			AddSoftwareNode(lootAPI->GetLibraryName(), lootAPI->GetLibraryVersion(), "https://github.com/loot/loot-api", ImageResourceID::LOOT);
		}

		// Add resources
		AddResourceNode("Fugue Icons", "http://p.yusukekamiyamane.com");
		AddResourceNode("Tango 7-zip", "https://laoism.deviantart.com/art/Tango-7-zip-2-1-113983312");

		SetItemCount(m_DataVector.size());
	}
	void DisplayModel::CreateView(wxWindow* parent)
	{
		View* view = new View(parent, KxID_NONE, CtrlStyle::NoHeader|CtrlStyle::FitLastColumn);
		view->ToggleWindowStyle(wxBORDER_NONE);
		view->AssignModel(this);

		view->Bind(EvtITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);

		view->AppendColumn<BitmapTextRenderer>(KTr("Generic.Name"), ColumnRef::Name);
		view->AppendColumn<TextRenderer>(KTr("Generic.Version"), ColumnRef::Version);
		view->AppendColumn<BitmapRenderer>(KTr("Generic.License"), ColumnRef::License);
		view->AppendColumn<TextRenderer>(KTr("Generic.Address"), ColumnRef::URL);
	}
}
