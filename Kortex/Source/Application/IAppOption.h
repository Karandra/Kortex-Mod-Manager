#pragma once
#include "stdafx.h"
#include "Options/OptionSerializer.h"
#include <KxFramework/KxXDocumentNode.h>
#include <KxFramework/KxXML.h>

namespace Kortex
{
	class IGameInstance;
	class IConfigurableGameInstance;
}

namespace Kortex
{
	class IAppOption:
		public KxXDocumentNode<IAppOption>,
		public Application::OptionSerializer::UILayout
	{
		private:
			KxXMLNode* m_ConfigNode = nullptr;
			IConfigurableGameInstance* m_Instance = nullptr;
			
		protected:
			bool DoSetValue(const wxString& value, bool isCDATA = false) override;
			bool DoSetAttribute(const wxString& name, const wxString& value) override;
			void ChangeNotify();

			void AssignNode(KxXMLNode& node)
			{
				m_ConfigNode = &node;
			}
			bool AssignInstance(const IConfigurableGameInstance* instance);
			bool AssignActiveInstance();

		protected:
			IAppOption() = default;
			IAppOption(KxXMLNode& node, const IConfigurableGameInstance* instance = nullptr)
			{
				AssignNode(node);
				AssignInstance(instance);
			}

		public:
			/* General */
			bool IsOK() const override
			{
				return m_ConfigNode && m_ConfigNode->IsOK();
			}
			bool IsInstanceOption() const
			{
				return m_Instance != nullptr;
			}
			bool IsGlobalOption() const
			{
				return !IsInstanceOption();
			}
			void Save();

			KxXMLNode GetNode() const
			{
				return m_ConfigNode ? *m_ConfigNode : KxXMLNode();
			}
			Node QueryElement(const wxString& XPath) const override
			{
				return Node();
			}
			Node QueryOrCreateElement(const wxString& XPath) override
			{
				return Node();
			}

			/* Node */
			size_t GetIndexWithinParent() const override
			{
				return 0;
			}

			wxString GetName() const override
			{
				return m_ConfigNode->GetName();
			}
			bool SetName(const wxString& name) override
			{
				const bool res = m_ConfigNode->SetName(name);
				ChangeNotify();
				return res;
			}

			size_t GetChildrenCount() const override
			{
				return 0;
			}
			NodeVector GetChildren() const override
			{
				return {};
			}
		
			/* Value */
			wxString GetValue(const wxString& default = wxEmptyString) const override
			{
				return m_ConfigNode->GetValue(default);
			}

			/* Attributes */
			size_t GetAttributeCount() const override
			{
				return 0;
			}
			KxStringVector GetAttributes() const override
			{
				return m_ConfigNode->GetAttributes();
			}
			bool HasAttribute(const wxString& name) const override
			{
				return m_ConfigNode->HasAttribute(name);
			}
			wxString GetAttribute(const wxString& name, const wxString& default = wxEmptyString) const override
			{
				return m_ConfigNode->GetAttribute(name, default);
			}
	
			/* Navigation */
			Node GetElementByAttribute(const wxString& name, const wxString& value) const override
			{
				return Node();
			}
			Node GetElementByTag(const wxString& tagName) const override
			{
				return Node();
			}
			Node GetParent() const override
			{
				return Node();
			}
			Node GetPreviousSibling() const override
			{
				return Node();
			}
			Node GetNextSibling() const override
			{
				return Node();
			}
			Node GetFirstChild() const override
			{
				return Node();
			}
			Node GetLastChild() const override
			{
				return Node();
			}
		
		public:
			void SaveDataViewLayout(KxDataViewCtrl* dataView)
			{
				DataViewLayout(*this, SerializationMode::Save, dataView);
			}
			void LoadDataViewLayout(KxDataViewCtrl* dataView)
			{
				DataViewLayout(*this, SerializationMode::Load, dataView);
			}

			void SaveSplitterLayout(KxSplitterWindow* splitter)
			{
				SplitterLayout(*this, SerializationMode::Save, splitter);
			}
			void LoadDataViewLayout(KxSplitterWindow* splitter)
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
