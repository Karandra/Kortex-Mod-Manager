#pragma once
#include "stdafx.h"
#include "Application/AppOption.h"
#include <KxFramework/KxString.h>

namespace Kortex::NetworkManager::NXMHandler
{
	class OptionStore
	{
		public:
			struct Instance
			{
				wxString ID;

				explicit operator bool() const
				{
					return !ID.IsEmpty();
				}
				bool operator!() const
				{
					return ID.IsEmpty();
				}
			};
			struct Command
			{
				wxString Executable;
				wxString Arguments;

				explicit operator bool() const
				{
					return !Executable.IsEmpty();
				}
				bool operator!() const
				{
					return Executable.IsEmpty();
				}
			};

		private:
			template<class T, class TItems> static T* DoGetOption(TItems&& items, const wxString& nexusID)
			{
				auto it = items.find(KxString::ToLower(nexusID));
				if (it !=  items.end())
				{
					using T2 = std::remove_cv_t<T>;
					if (auto value = std::get_if<T2>(&it->second))
					{
						return value;
					}
					else if (auto value = std::get_if<T2>(&it->second))
					{
						return value;
					}
				}
				return nullptr;
			}

		private:
			std::unordered_map<wxString, std::variant<Instance, Command>> m_Options;

		public:
			void Save(AppOption& option) const;
			void Load(const AppOption& option);

			template<class T> const T* GetOption(const wxString& nexusID) const;
			template<> const Instance* GetOption(const wxString& nexusID) const
			{
				return DoGetOption<const Instance>(m_Options, nexusID);
			}
			template<> const Command* GetOption(const wxString& nexusID) const
			{
				return DoGetOption<const Command>(m_Options, nexusID);
			}

			void SetOption(const wxString& nexusID, const Instance& value)
			{
				m_Options.insert_or_assign(KxString::ToLower(nexusID), value);
			}
			void SetOption(const wxString& nexusID, const Command& value)
			{
				m_Options.insert_or_assign(KxString::ToLower(nexusID), value);
			}
			void RemoveOption(const wxString& nexusID)
			{
				m_Options.erase(KxString::ToLower(nexusID));
			}
	};
}
