#include "stdafx.h"
#include "KSetEnabledWrapper.h"
#include <KxFramework/KxAuiToolBar.h>
#include <KxFramework/KxMenu.h>

enum KSetEnabledWrapperTypes
{
	KSEW_TYPE_WINDOW,
	KSEW_TYPE_MENU_ITEM,
	KSEW_TYPE_TOOLBAR_ITEM,
};

KSetEnabledWrapper::~KSetEnabledWrapper()
{
}

bool KSetEnabledWrapper::IsEnabled() const
{
	switch (m_Objects.index())
	{
		case KSEW_TYPE_WINDOW:
		{
			return std::get<wxWindow*>(m_Objects)->IsEnabled();
		}
		case KSEW_TYPE_MENU_ITEM:
		{
			return std::get<KxMenuItem*>(m_Objects)->IsEnabled();
		}
		case KSEW_TYPE_TOOLBAR_ITEM:
		{
			return std::get<KxAuiToolBarItem*>(m_Objects)->IsEnabled();
		}
	};
	return false;
}
void KSetEnabledWrapper::SetEnabled(bool isEnabled)
{
	switch (m_Objects.index())
	{
		case KSEW_TYPE_WINDOW:
		{
			std::get<wxWindow*>(m_Objects)->Enable(isEnabled);
			break;
		}
		case KSEW_TYPE_MENU_ITEM:
		{
			std::get<KxMenuItem*>(m_Objects)->Enable(isEnabled);
			break;
		}
		case KSEW_TYPE_TOOLBAR_ITEM:
		{
			std::get<KxAuiToolBarItem*>(m_Objects)->SetEnabled(isEnabled);
			break;
		}
	};
}
