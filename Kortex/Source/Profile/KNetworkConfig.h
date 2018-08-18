#pragma once
#include "stdafx.h"
#include "KLabeledValue.h"
#include <KxFramework/KxSingleton.h>
class KProfile;
class KNetwork;

class KNetworkConfig: public KxSingletonPtr<KNetworkConfig>
{
	private:
		KNetwork* m_Network = NULL;
		wxString m_NexusID;

	public:
		KNetworkConfig(KProfile& profile, KxXMLNode& node);
		~KNetworkConfig();

	public:
		bool HasNexusID() const
		{
			return !m_NexusID.IsEmpty();
		}
		const wxString& GetNexusID() const
		{
			return m_NexusID;
		}
};
