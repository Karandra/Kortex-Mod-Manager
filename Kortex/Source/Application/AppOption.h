#pragma once
#include "stdafx.h"
#include "Utility/String.h"
#include "Options/OptionSerializer.h"
#include <KxFramework/KxXDocumentNode.h>
#include <KxFramework/KxXML.h>

namespace Kortex
{
	class IGameProfile;
	class IGameInstance;
	class IConfigurableGameInstance;
}

namespace Kortex
{
	class AppOption: public KxXDocumentNode<AppOption>
	{
		friend class KxXDocumentNode<AppOption>;

		public:
			using SerializationMode = Application::OptionSerializer::SerializationMode;
			enum class Disposition
			{
				None,
				Global,
				Instance,
				Profile,
			};

		public:
			static wxString MakeXPath()
			{
				return wxString();
			}
			template<class Args> static wxString MakeXPath(Args&& arg)
			{
				return arg;
			}
			template<class... Args> static wxString MakeXPath(Args&&... arg)
			{
				return Utility::String::ConcatWithSeparator(wxS('/'), std::forward<Args>(arg)...);
			}

		private:
			KxXMLNode m_ConfigNode;
			IConfigurableGameInstance* m_Instance = nullptr;
			IGameProfile* m_Profile = nullptr;
			Disposition m_Disposition = Disposition::None;

			Application::OptionSerializer::UILayout m_UISerializer;
			
		protected:
			wxString DoGetValue(const wxString& defaultValue = wxEmptyString) const override;
			bool DoSetValue(const wxString& value, AsCDATA asCDATA = AsCDATA::Auto) override;

			wxString DoGetAttribute(const wxString& name, const wxString& defaultValue = wxEmptyString) const override;
			bool DoSetAttribute(const wxString& name, const wxString& value) override;

			void AssignNode(const KxXMLNode& node)
			{
				m_ConfigNode = node;
			}
			void AssignDisposition(Disposition disposition)
			{
				m_Disposition = disposition;
			}
			
			bool AssignInstance(const IConfigurableGameInstance* instance);
			bool AssignActiveInstance();
			
			void AssignProfile(IGameProfile* profile);
			void AssignActiveProfile();

		protected:
			AppOption(const AppOption& other, const KxXMLNode& node);
			AppOption() = default;

		public:
			// General
			bool IsOK() const override;
			AppOption QueryElement(const wxString& XPath) const override;
			AppOption QueryOrCreateElement(const wxString& XPath) override;

			Disposition GetDisposition() const
			{
				return m_Disposition;
			}
			bool IsGlobalOption() const
			{
				return m_Disposition == Disposition::Global;
			}
			bool IsInstanceOption() const
			{
				return m_Disposition == Disposition::Instance;
			}
			bool IsProfileOption() const
			{
				return m_Disposition == Disposition::Profile;
			}

			KxXMLNode GetNode() const
			{
				return m_ConfigNode;
			}
			wxString GetXPath() const override
			{
				return m_ConfigNode.GetXPath();
			}
			wxString GetName() const override
			{
				return m_ConfigNode.GetName();
			}

			void NotifyChange();

		public:
			void SaveDataViewLayout(const KxDataViewCtrl* dataView)
			{
				m_UISerializer.DataViewLayout(*this, SerializationMode::Save, const_cast<KxDataViewCtrl*>(dataView));
			}
			void LoadDataViewLayout(KxDataViewCtrl* dataView) const
			{
				m_UISerializer.DataViewLayout(const_cast<AppOption&>(*this), SerializationMode::Load, dataView);
			}

			void SaveDataViewLayout(const KxDataView2::View* dataView)
			{
				m_UISerializer.DataView2Layout(*this, SerializationMode::Save, const_cast<KxDataView2::View*>(dataView));
			}
			void LoadDataViewLayout(KxDataView2::View* dataView) const
			{
				m_UISerializer.DataView2Layout(const_cast<AppOption&>(*this), SerializationMode::Load, dataView);
			}

			void SaveSplitterLayout(const KxSplitterWindow* splitter)
			{
				m_UISerializer.SplitterLayout(*this, SerializationMode::Save, const_cast<KxSplitterWindow*>(splitter));
			}
			void LoadSplitterLayout(KxSplitterWindow* splitter) const
			{
				m_UISerializer.SplitterLayout(const_cast<AppOption&>(*this), SerializationMode::Load, splitter);
			}
	
			void SaveWindowGeometry(const wxTopLevelWindow* window)
			{
				m_UISerializer.WindowGeometry(*this, SerializationMode::Save, const_cast<wxTopLevelWindow*>(window));
			}
			void LoadWindowGeometry(wxTopLevelWindow* window) const
			{
				m_UISerializer.WindowGeometry(const_cast<AppOption&>(*this), SerializationMode::Load, window);
			}
	};
}

namespace Kortex::Application
{
	class IWithConfig
	{
		protected:
			virtual ~IWithConfig() = default;

		public:
			virtual const KxXMLDocument& GetConfig() const = 0;
			virtual KxXMLDocument& GetConfig() = 0;

			virtual void OnConfigChanged(AppOption& option) = 0;
			virtual void SaveConfig() = 0;
	};
}

