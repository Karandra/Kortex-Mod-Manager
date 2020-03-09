#pragma once
#include <cstdint>
#include <KxFramework/KxXML.h>

namespace Kortex::IPC
{
	class Serializer final
	{
		private:
			template<class Head, class... Tail>
			static void SerializeAux(Serializer& serializer, Head&& head, Tail&&... tail)
			{
				serializer.PushValue(head);
				(void)std::initializer_list<int> {((serializer.PushValue(tail)), 0)...};
			}

			template<class... Args>
			struct ConvertToTuple
			{
				template<size_t... t_Sequence>
				std::tuple<Args...> operator()(const Serializer& serializer, std::index_sequence<t_Sequence...>)
				{
					return std::make_tuple(Args {serializer.Get<Args>(t_Sequence)}...);
				}
			};

		public:
			template<class... Args>
			static wxString Serialize(Args&&... arg)
			{
				Serializer serializer;
				SerializeAux(serializer, arg...);
				return serializer.ToString();
			}
			
			template<class... Args>
			static std::tuple<Args...> Deserialize(const wxString& serialized)
			{
				Serializer serializer(serialized);
				return ConvertToTuple<Args...>()(serializer, std::make_index_sequence<sizeof...(Args)>());
			}

		private:
			KxXMLDocument m_XML;
			KxXMLNode m_RootNode;

			const wxString m_ItemName = wxS("Item");
			const wxString m_RootName = wxS("Serialized");

		private:
			KxXMLNode GetNthNode(const size_t index) const
			{
				size_t i = 0;
				for (KxXMLNode node = m_RootNode.GetFirstChildElement(); node; node = node.GetNextSiblingElement())
				{
					if (i == index)
					{
						return node;
					}
					i++;
				}
				return {};
			}

		public:
			Serializer()
			{
				m_RootNode = m_XML.NewElement(m_RootName);
			}
			Serializer(const wxString& serialized)
				:m_XML(serialized)
			{
				m_RootNode = m_XML.ConstructElement(m_RootName);
			}

		public:
			wxString ToString() const
			{
				return m_XML.GetXML();
			}

		public:
			template<class T>
			void PushValue(T&& value)
			{
				using Tx = std::remove_reference_t<std::remove_cv_t<T>>;

				if constexpr (std::is_integral_v<Tx> || std::is_enum_v<Tx>)
				{
					m_RootNode.NewElement(m_ItemName).SetValue(static_cast<int64_t>(value));
				}
				else if constexpr (std::is_floating_point_v<Tx>)
				{
					m_RootNode.NewElement(m_ItemName).SetValue(static_cast<double>(value));
				}
				else
				{
					static_assert(false, "unsupported data type");
				}
			}
			
			template<>
			void PushValue(const wxString& value)
			{
				m_RootNode.NewElement(m_ItemName).SetValue(value);
			}
	
			template<class T>
			T Get(size_t index, const T& defaultValue = {}) const
			{
				using Tx = std::remove_reference_t<std::remove_cv_t<T>>;

				if constexpr (std::is_integral_v<Tx> || std::is_enum_v<Tx>)
				{
					return static_cast<Tx>(GetNthNode(index).GetValueInt(defaultValue));
				}
				else if constexpr (std::is_floating_point_v<Tx>)
				{
					return static_cast<Tx>(GetNthNode(index).GetValueFloat(defaultValue));
				}
				else
				{
					static_assert(false, "unsupported data type");
				}
				return {};
			}
			
			template<>
			wxString Get(size_t index, const wxString& defaultValue) const
			{
				return GetNthNode(index).GetValue(defaultValue);
			}
	};
}
