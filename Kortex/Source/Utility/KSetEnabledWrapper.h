#pragma once
#include "stdafx.h"
class wxWindow;
class KxMenuItem;
class KxAuiToolBarItem;
enum KSetEnabledWrapperTypes;

class KSetEnabledWrapper
{
	public:
		template<class T> static bool IsAllEnabled(const T& array)
		{
			for (auto& v: array)
			{
				if (!v.IsEnabled())
				{
					return false;
				}
			}
			return true;
		}
		template<class T> static void SetAllEnabled(T& array, bool isEnabled)
		{
			for (auto& v: array)
			{
				v.SetEnabled(isEnabled);
			}
		}

	private:
		std::variant<wxWindow*, KxMenuItem*, KxAuiToolBarItem*> m_Objects;

	public:
		KSetEnabledWrapper()
			:m_Objects((wxWindow*)NULL)
		{
		}
		template<class T> KSetEnabledWrapper(T* object)
			:m_Objects(object)
		{
		}
		~KSetEnabledWrapper();

	public:
		bool IsEnabled() const;
		void SetEnabled(bool isEnabled);
};
typedef std::vector<KSetEnabledWrapper> KSetEnabledWrapperArray;
