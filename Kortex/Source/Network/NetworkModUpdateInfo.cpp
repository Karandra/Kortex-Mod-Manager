#include "stdafx.h"
#include "NetworkModUpdateInfo.h"
#include <KxFramework/KxXML.h>

namespace Kortex
{
	void NetworkModUpdateInfo::Save(KxXMLNode& node) const
	{
		if (m_UpdateCheckDate.IsValid())
		{
			node.NewElement(wxS("UpdateCheckDate")).SetValue(m_UpdateCheckDate.FormatISOCombined());
		}
		if (m_Version.IsOK())
		{
			node.NewElement(wxS("Version")).SetValue(m_Version);
		}
		node.NewElement(wxS("State")).SetValue(UpdateStateDef::ToString(m_State));
	}
	void NetworkModUpdateInfo::Load(const KxXMLNode& node)
	{
		m_UpdateCheckDate.ParseISOCombined(node.GetFirstChildElement(wxS("UpdateCheckDate")).GetValue());
		m_Version = node.GetFirstChildElement(wxS("Version")).GetValue();
		m_State = UpdateStateDef::FromString(node.GetFirstChildElement(wxS("State")).GetValue(), UpdateState::Unknown);
	}
}
