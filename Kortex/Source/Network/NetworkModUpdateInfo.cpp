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
		if (m_ActivityHash != 0)
		{
			node.NewElement(wxS("ActivityHash")).SetValue(static_cast<int64_t>(m_ActivityHash));
		}
		if (m_Version.IsOK())
		{
			node.NewElement(wxS("Version")).SetValue(m_Version);
		}

		// State
		KxXMLNode stateNode = node.NewElement(wxS("State"));
		stateNode.SetValue(UpdateStateDef::ToString(m_State));

		if (m_Details != UpdateDetails::None)
		{
			stateNode.SetAttribute(wxS("Details"), UpdateDetailsDef::ToOrExpression(m_Details));
		}
	}
	void NetworkModUpdateInfo::Load(const KxXMLNode& node)
	{
		m_UpdateCheckDate.ParseISOCombined(node.GetFirstChildElement(wxS("UpdateCheckDate")).GetValue());
		m_ActivityHash = static_cast<size_t>(node.GetFirstChildElement(wxS("ActivityHash")).GetValueInt());
		m_Version = node.GetFirstChildElement(wxS("Version")).GetValue();

		// State
		const KxXMLNode detailsNode = node.GetFirstChildElement(wxS("State"));
		m_State = UpdateStateDef::FromString(detailsNode.GetValue(), UpdateState::Unknown);
		m_Details = UpdateDetailsDef::FromOrExpression(detailsNode.GetAttribute(wxS("Details")), UpdateDetails::None);
	}
}
