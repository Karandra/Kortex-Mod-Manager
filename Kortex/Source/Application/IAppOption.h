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
	class IAppOption: 
		public KxXDocumentNode<IAppOption>,
		public Application::OptionSerializer::UILayout
	{
		friend class KxXDocumentNode<IAppOption>;

		public:
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
			
		protected:
			wxString DoGetValue(const wxString& defaultValue = wxEmptyString) const override;
			bool DoSetValue(const wxString& value, bool isCDATA = false) override;

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
			IAppOption() = default;

		public:
			/* General */
			bool IsOK() const override
			{
				return m_ConfigNode.IsOK() && m_Disposition != Disposition::None;
			}
			
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
			void SaveDataViewLayout(KxDataViewCtrl* dataView)
			{
				DataViewLayout(*this, SerializationMode::Save, dataView);
			}
			void LoadDataViewLayout(KxDataViewCtrl* dataView)
			{
				DataViewLayout(*this, SerializationMode::Load, dataView);
			}

			void SaveDataViewLayout(KxDataView2::View* dataView)
			{
				DataView2Layout(*this, SerializationMode::Save, dataView);
			}
			void LoadDataViewLayout(KxDataView2::View* dataView)
			{
				DataView2Layout(*this, SerializationMode::Load, dataView);
			}

			void SaveSplitterLayout(KxSplitterWindow* splitter)
			{
				SplitterLayout(*this, SerializationMode::Save, splitter);
			}
			void LoadSplitterLayout(KxSplitterWindow* splitter)
			{
				SplitterLayout(*this, SerializationMode::Load, splitter);
			}
	
			void SaveWindowLayout(wxTopLevelWindow* window)
			{
				WindowSize(*this, SerializationMode::Save, window);
			}
			void LoadWindowLayout(wxTopLevelWindow* window)
			{
				WindowSize(*this, SerializationMode::Load, window);
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

			virtual void OnConfigChanged(IAppOption& option) = 0;
			virtual void SaveConfig() = 0;
	};
}

