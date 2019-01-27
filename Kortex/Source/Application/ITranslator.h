#pragma once
#include "stdafx.h"
#include <KxFramework/KxFormat.h>
#include <optional>

namespace Kortex
{
	class IGameInstance;

	class ITranslator
	{
		public:
			using OpString = std::optional<wxString>;

		public:
			static const ITranslator& GetAppTranslator();

			static wxString GetVariable(const wxString& variable, const wxString& variableNamespace = {});
			static wxString GetVariable(const IGameInstance& instance, const wxString& variable, const wxString& variableNamespace = {});
			static wxString ExpandVariables(const wxString& variables);
			static wxString ExpandVariables(const IGameInstance& instance, const wxString& variables);

		private:
			template<class T> wxString ConstructTranslationVar(const T& id) const
			{
				return KxString::Format(wxS("$T(%1)"), id);
			}

		protected:
			virtual OpString DoGetString(const wxString& id) const = 0;
			virtual OpString DoGetString(KxStandardID id) const = 0;
			OpString DoGetString(wxStandardID id) const
			{
				return DoGetString(static_cast<KxStandardID>(id));
			}
			
			template<class T> OpString FetchString(const T& id) const
			{
				OpString value = DoGetString(id);
				if (value)
				{
					return ExpandVariables(*value);
				}
				return std::nullopt;
			}
			template<class T> OpString FetchString(const IGameInstance& instance, const T& id) const
			{
				OpString value = DoGetString(id);
				if (value)
				{
					return ExpandVariables(instance, *value);
				}
				return std::nullopt;
			}

		public:
			virtual ~ITranslator() = default;

		public:
			template<class T> wxString GetString(const T& id) const
			{
				OpString value = FetchString(id);
				return value ? *value : ConstructTranslationVar(id);
			}
			template<class T> wxString GetString(const IGameInstance& instance, const T& id) const
			{
				OpString value = FetchString(instance, id);
				return value ? *value : ConstructTranslationVar(id);
			}

			template<class T> OpString TryGetString(const T& id) const
			{
				return FetchString(id);
			}
			template<class T> OpString TryGetString(const IGameInstance& instance, const T& id) const
			{
				return FetchString(instance, id);
			}

			template<class T, class... Args> wxString FormatString(const T& id, Args&&... arg) const
			{
				return KxString::Format(GetString(id), std::forward<Args>(arg)...);
			}
			template<class T, class... Args> wxString FormatString(const IGameInstance& instance, const T& id, Args&&... arg) const
			{
				return KxString::Format(GetString(instance, id), std::forward<Args>(arg)...);
			}

			template<class T, class... Args> OpString TryFormatString(const T& id, Args&&... arg) const
			{
				OpString value = TryGetString(id);
				if (value)
				{
					value = KxString::Format(*value, std::forward<Args>(arg)...);
				}
				return value;
			}
			template<class T, class... Args> wxString TryFormatString(const IGameInstance& instance, const T& id, Args&&... arg) const
			{
				OpString value = TryGetString(instance, id);
				if (value)
				{
					value = KxString::Format(*value, std::forward<Args>(arg)...);
				}
				return value;
			}
	};
}
