#pragma once
#include "Framework.hpp"
#include "Options/OptionSerializer.h"
#include <kxf/Serialization/XML.h>
#include <kxf/RTTI/QueryInterface.h>
class wxTopLevelWindow;

namespace Kortex
{
	class IGameProfile;
	class IGameInstance;
	class IConfigurableGameInstance;

	class IWorkspaceContainer;
}

namespace Kortex
{
	class AppOption: public kxf::XDocument::XNode<AppOption>
	{
		friend class kxf::XDocument::XNode<AppOption>;

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
			template<class... Args>
			static kxf::String MakeXPath(Args&&... arg)
			{
				return kxf::String::ConcatWithSeparator(wxS('/'), std::forward<Args>(arg)...);
			}

		private:
			kxf::XMLNode m_ConfigNode;
			IGameInstance* m_Instance = nullptr;
			IGameProfile* m_Profile = nullptr;
			Disposition m_Disposition = Disposition::None;

			Application::OptionSerializer::UILayout m_UISerializer;
			
		protected:
			std::optional<kxf::String> DoGetValue() const override;
			bool DoSetValue(const kxf::String& value, WriteEmpty writeEmpty, AsCDATA asCDATA = AsCDATA::Auto) override;

			std::optional<kxf::String> DoGetAttribute(const kxf::String& name) const override;
			bool DoSetAttribute(const kxf::String& name, const kxf::String& value, WriteEmpty writeEmpty) override;

			void AssignNode(const kxf::XMLNode& node)
			{
				m_ConfigNode = node;
			}
			void AssignDisposition(Disposition disposition)
			{
				m_Disposition = disposition;
			}
			
			bool AssignInstance(IGameInstance& instance);
			bool AssignActiveInstance();
			
			void AssignProfile(IGameProfile& profile);
			void AssignActiveProfile();

		protected:
			AppOption();
			AppOption(const AppOption& other, const kxf::XMLNode& node);
			AppOption(const AppOption&) noexcept;
			~AppOption();

		public:
			// General
			bool IsNull() const override;
			AppOption QueryElement(const kxf::String& XPath) const override;
			AppOption ConstructElement(const kxf::String& XPath) override;

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

			kxf::XMLNode GetNode() const
			{
				return m_ConfigNode;
			}
			kxf::String GetXPath() const override
			{
				return m_ConfigNode.GetXPath();
			}
			kxf::String GetName() const override
			{
				return m_ConfigNode.GetName();
			}

			void NotifyChange();

		public:
			void SaveDataViewLayout(const kxf::UI::DataView::View& dataView)
			{
				m_UISerializer.DataViewLayout(*this, SerializationMode::Save, const_cast<kxf::UI::DataView::View&>(dataView));
			}
			void LoadDataViewLayout(kxf::UI::DataView::View& dataView) const
			{
				m_UISerializer.DataViewLayout(const_cast<AppOption&>(*this), SerializationMode::Load, dataView);
			}

			void SaveSplitterLayout(const kxf::UI::SplitterWindow& splitter)
			{
				m_UISerializer.SplitterLayout(*this, SerializationMode::Save, const_cast<kxf::UI::SplitterWindow&>(splitter));
			}
			void LoadSplitterLayout(kxf::UI::SplitterWindow& splitter) const
			{
				m_UISerializer.SplitterLayout(const_cast<AppOption&>(*this), SerializationMode::Load, splitter);
			}
			
			void SaveWorkspaceContainerLayout(const IWorkspaceContainer& container)
			{
				m_UISerializer.WorkspaceContainerLayout(*this, SerializationMode::Save, const_cast<IWorkspaceContainer&>(container));
			}
			void LoadWorkspaceContainerLayout(IWorkspaceContainer& container) const
			{
				m_UISerializer.WorkspaceContainerLayout(const_cast<AppOption&>(*this), SerializationMode::Load, container);
			}

			void SaveWindowGeometry(const wxTopLevelWindow& window)
			{
				m_UISerializer.WindowGeometry(*this, SerializationMode::Save, const_cast<wxTopLevelWindow&>(window));
			}
			void LoadWindowGeometry(wxTopLevelWindow& window) const
			{
				m_UISerializer.WindowGeometry(const_cast<AppOption&>(*this), SerializationMode::Load, window);
			}

		public:
			AppOption& operator=(const AppOption&) noexcept;
	};
}

namespace Kortex::Application
{
	class IWithConfig: public kxf::RTTI::Interface<IWithConfig>
	{
		KxRTTI_DeclareIID(IWithConfig, {0xeff53acc, 0xbd93, 0x49d0, {0x84, 0x9b, 0x49, 0xde, 0x3e, 0xc3, 0x8a, 0xce}});

		protected:
			virtual ~IWithConfig() = default;

		public:
			virtual const kxf::XMLDocument& GetConfig() const = 0;
			virtual kxf::XMLDocument& GetConfig() = 0;

			virtual void OnConfigChanged(AppOption& option) = 0;
			virtual void SaveConfig() = 0;
	};
}
