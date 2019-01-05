#pragma once
#include <cstdint>
#include <KxFramework/KxXML.h>

namespace Kortex::IPC
{
	class Serializer
	{
		private:
			template<class Head, class... Tail>
			static void SerializeAux(Serializer& serializer, Head&& head, Tail&&... tail)
			{
				serializer.AddValue(head);
				(void)std::initializer_list<int> {((serializer.AddValue(tail)), 0)...};
			}

		public:
			template<class... Args> static wxString Serialize(Args&&... arg)
			{
				Serializer serializer;
				SerializeAux(serializer, arg...);
				return serializer.ToString();
			}

		private:
			KxXMLDocument m_XML;
			KxXMLNode m_RootNode;

			const wxString m_EntryName = wxS("Entry");
			const wxString m_RootName = wxS("Serialized");

		private:
			KxXMLNode GetNthNode(const size_t index) const
			{
				size_t i = 0;
				for (KxXMLNode node = m_RootNode.GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
				{
					if (i == index)
					{
						return node;
					}
					i++;
				}
				return KxXMLNode();
			}

		public:
			Serializer()
			{
				m_RootNode = m_XML.NewElement(m_RootName);
			}
			Serializer(const wxString& serializedData)
				:m_XML(serializedData)
			{
				m_RootNode = m_XML.QueryOrCreateElement(m_RootName);
			}

		public:
			wxString ToString() const
			{
				return m_XML.GetXML();
			}

		public:
			template<class T> void AddValue(const T& value)
			{
				static_assert(std::is_integral_v<T> || std::is_enum_v<T>);

				m_RootNode.NewElement(m_EntryName).SetValue(static_cast<int64_t>(value));
			}
			template<> void AddValue(const wxString& value)
			{
				m_RootNode.NewElement(m_EntryName).SetValue(value, true);
			}
	
			template<class T> T GetInt(size_t index, T defaultValue = {}) const
			{
				return GetNthNode(index).GetValueInt(defaultValue);
			}
			wxString GetString(size_t index, wxString defaultValue = {}) const
			{
				return GetNthNode(index).GetValue(defaultValue);
			}
	};
}
