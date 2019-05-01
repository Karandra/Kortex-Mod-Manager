#include "stdafx.h"
#include "DisplayModel.h"
#include "Dialog.h"
#include "Archive/KArchive.h"
#include "VirtualFileSystem/IVFSService.h"
#include "Utility/KAux.h"
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
	enum ColumnID
	{
		Name,
		Version,
		License,
		URL
	};
}

namespace Kortex::Application::About
{
	void DisplayModel::OnInitControl()
	{
		GetView()->Bind(KxEVT_DATAVIEW_ITEM_ACTIVATED, &DisplayModel::OnActivateItem, this);
		GetView()->SetUniformRowHeight(GetView()->GetDefaultRowHeight(KxDVC_ROW_HEIGHT_EXPLORER));

		// Columns
		GetView()->AppendColumn<KxDataViewBitmapTextRenderer>(KTr("Generic.Name"), ColumnID::Name, KxDATAVIEW_CELL_INERT);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Version"), ColumnID::Version, KxDATAVIEW_CELL_INERT);
		GetView()->AppendColumn<KxDataViewBitmapRenderer>(KTr("Generic.License"), ColumnID::License, KxDATAVIEW_CELL_INERT);
		GetView()->AppendColumn<KxDataViewTextRenderer>(KTr("Generic.Address"), ColumnID::URL, KxDATAVIEW_CELL_INERT);
	}

	void DisplayModel::GetValueByRow(wxAny& value, size_t row, const KxDataViewColumn* column) const
	{
		if (const INode* node = GetDataEntry(row))
		{
			switch (column->GetID())
			{
				case ColumnID::Name:
				{
					value = KxDataViewBitmapTextValue(node->GetName(), ImageProvider::GetBitmap(node->GetIconID()));
					break;
				}
				case ColumnID::Version:
				{
					value = node->GetVersion().ToString();
					break;
				}
				case ColumnID::License:
				{
					if (!node->GetLicense().IsEmpty())
					{
						value = ImageProvider::GetBitmap(ImageResourceID::Cheque);
					}
					break;
				}
				case ColumnID::URL:
				{
					value = node->GetURL();
					break;
				}
			};
		}
	}
	bool DisplayModel::SetValueByRow(const wxAny& value, size_t row, const KxDataViewColumn* column)
	{
		return false;
	}
	bool DisplayModel::IsEnabledByRow(size_t row, const KxDataViewColumn* column) const
	{
		return false;
	}
	bool DisplayModel::GetItemAttributesByRow(size_t row, const KxDataViewColumn* column, KxDataViewItemAttributes& attribute, KxDataViewCellState cellState) const
	{
		if (const INode* node = GetDataEntry(row))
		{
			auto GetLinkColor = [this]()
			{
				return KxColor(GetView()->GetBackgroundColour()).GetContrastColor(KxColor(0, 0, 255), KxColor(25, 25, 100));
			};

			switch (column->GetID())
			{
				case ColumnID::Version:
				{
					attribute.SetAlignment(wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT);
					return true;
				}
				case ColumnID::License:
				{
					attribute.SetBold();
					if (cellState & KxDATAVIEW_CELL_HIGHLIGHTED)
					{
						attribute.SetForegroundColor(GetLinkColor());
						attribute.SetUnderlined();
					}
					return true;
				}
				case ColumnID::URL:
				{
					if (cellState & KxDATAVIEW_CELL_HIGHLIGHTED)
					{
						
						attribute.SetForegroundColor(GetLinkColor());
						attribute.SetUnderlined();
						return true;
					}
					break;
				}
			};
		}
		return false;
	}

	void DisplayModel::OnActivateItem(KxDataViewEvent& event)
	{
		KxDataViewColumn* column = event.GetColumn();
		KxDataViewItem item = event.GetItem();

		if (const INode* node = GetDataEntry(GetRow(item)))
		{
			switch (column->GetID())
			{
				case ColumnID::License:
				{
					wxString license = node->GetLicense();
					if (!license.IsEmpty())
					{
						KxHTMLWindow* window = m_Dialog.CreateHTMLWindow();
						window->SetTextValue(node->GetLicense());

						m_Dialog.CreateTemporaryTab(window,
													KxString::Format("%1 \"%2\"", KTr("Generic.License"), node->GetName()),
													ImageProvider::GetBitmap(node->GetIconID())
						);
					}
					break;
				}
				case ColumnID::URL:
				{
					wxString url = node->GetURL();
					if (!url.IsEmpty())
					{
						KAux::AskOpenURL(url, GetViewTLW());
					}
					break;
				}
			};
		}
	}

	DisplayModel::DisplayModel(AboutDialog& dialog)
		:m_Dialog(dialog)
	{
		SetDataViewFlags(KxDataViewCtrl::DefaultStyle|KxDV_NO_HEADER);
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
			AddSoftwareNode(vfsService->GetLibraryName(), vfsService->GetLibraryVersion(), vfsService->GetLibraryURL(), ImageResourceID::JarEmpty);
			if (vfsService->HasNativeLibrary())
			{
				AddSoftwareNode(vfsService->GetNativeLibraryName(), vfsService->GetNativeLibraryVersion(), vfsService->GetNativeLibraryURL(), ImageResourceID::Jar);
			}
		}

		AddSoftwareNode(KxINI::GetLibraryName(), KxINI::GetLibraryVersion(), "https://github.com/brofield/simpleini", ImageResourceID::DocumentPencil);
		AddSoftwareNode(KxXMLDocument::GetLibraryName(), KxXMLDocument::GetLibraryVersion(), "https://github.com/leethomason/tinyxml2", ImageResourceID::EditCode);
		AddSoftwareNode("OpenSSL", KxCrypto::GetOpenSSLVersion(), "https://www.openssl.org", ImageResourceID::LockSSL);
		AddSoftwareNode("7-Zip", KArchive::GetLibraryVersion(), "https://www.7-zip.org", ImageResourceID::SevenZip);
		AddSoftwareNode(KxCURL::GetLibraryName(), KxCURL::GetLibraryVersion(), "https://curl.haxx.se", ImageResourceID::CURL);
		AddSoftwareNode(KxWebSocket::GetLibraryName(), KxWebSocket::GetLibraryVersion(), "https://github.com/zaphoyd/websocketpp", ImageResourceID::WebSocket);
		AddSoftwareNode(KxJSON::GetLibraryName(), KxJSON::GetLibraryVersion(), "https://github.com/nlohmann/json", ImageResourceID::JSON);

		if (PluginManager::LibLoot* lootAPI = PluginManager::LibLoot::GetInstance())
		{
			AddSoftwareNode(lootAPI->GetLibraryName(), lootAPI->GetLibraryVersion(), "https://github.com/loot/loot-api", ImageResourceID::LOOT);
		}

		// Add resources
		AddResourceNode("Fugue Icons", "http://p.yusukekamiyamane.com");
		AddResourceNode("Tango 7-zip", "https://laoism.deviantart.com/art/Tango-7-zip-2-1-113983312");

		KxDataViewVectorListModelEx::SetDataVector(&m_DataVector);
		KxDataViewVectorListModelEx::RefreshItems();
	}
}
