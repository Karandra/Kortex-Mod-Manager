#pragma once
#include "Framework.hpp"
#include "Options/OptionSerializer.h"
#include <kxf/Serialization/XML.h>
#include <kxf/RTTI/RTTI.h>

namespace Kortex
{
	class IGameProfile;
	class IGameInstance;

	class IWorkspaceContainer;
}

namespace Kortex
{
	class KORTEX_API AppOption: public kxf::XDocument::XNode<AppOption>
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
				return kxf::String::ConcatWithSeparator(kxS('/'), std::forward<Args>(arg)...);
			}

		private:
			kxf::XMLNode m_ConfigNode;
			IGameInstance* m_Instance = nullptr;
			IGameProfile* m_Profile = nullptr;

			Application::OptionSerializer::UILayout m_UISerializer;
			
		protected:
			// kxf::IXNode
			std::optional<kxf::String> DoGetValue() const override;
			bool DoSetValue(const kxf::String& value, WriteEmpty writeEmpty, AsCDATA asCDATA = AsCDATA::Auto) override;

			std::optional<kxf::String> DoGetAttribute(const kxf::String& name) const override;
			bool DoSetAttribute(const kxf::String& name, const kxf::String& value, WriteEmpty writeEmpty) override;

			// AppOption
			void AssignNode(kxf::XMLNode node)
			{
				m_ConfigNode = std::move(node);
			}
			void AssignInstance(IGameInstance& instance)
			{
				m_Instance = &instance;
			}
			void AssignProfile(IGameProfile& profile)
			{
				m_Profile = &profile;
			}

		public:
			AppOption();
			~AppOption();

		protected:
			AppOption(const AppOption& other, const kxf::XMLNode& node);
			AppOption(const AppOption&) noexcept;

		public:
			// kxf::IXNode
			bool IsNull() const override;

			kxf::String GetXPath() const override
			{
				return m_ConfigNode.GetXPath();
			}
			kxf::String GetName() const override
			{
				return m_ConfigNode.GetName();
			}

			// kxf::XNode<T>
			AppOption QueryElement(const kxf::String& XPath) const override;
			AppOption ConstructElement(const kxf::String& XPath) override;

			// AppOption
			kxf::XMLNode GetNode() const
			{
				return m_ConfigNode;
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

			void SaveWidgetGeometry(const kxf::IWidget& widget)
			{
				m_UISerializer.WidgetGeometry(*this, SerializationMode::Save, const_cast<kxf::IWidget&>(widget));
			}
			void LoadWidgetGeometry(kxf::IWidget& widget) const
			{
				m_UISerializer.WidgetGeometry(const_cast<AppOption&>(*this), SerializationMode::Load, widget);
			}

		public:
			AppOption& operator=(const AppOption&) noexcept;
	};
}
