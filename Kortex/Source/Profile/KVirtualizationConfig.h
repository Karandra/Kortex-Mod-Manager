#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class KProfile;

enum KPVEntryType
{
	KPVE_MIRRORED,
};

class KVirtualizationEntry
{
	public:
		using Vector = std::vector<KVirtualizationEntry>;

	private:
		wxString m_Source;
		wxString m_Target;

	public:
		KVirtualizationEntry(KxXMLNode& node);

	public:
		bool IsOK() const
		{
			return !m_Source.IsEmpty() && !m_Target.IsEmpty();
		}

		const wxString& GetSource() const
		{
			return m_Source;
		}
		const wxString& GetDestination() const
		{
			return m_Target;
		}
};

//////////////////////////////////////////////////////////////////////////
class KVirtualizationConfig: public KxSingletonPtr<KVirtualizationConfig>
{
	private:
		KVirtualizationEntry::Vector m_MirroredLocations;
		KxStringVector m_MandatoryVirtualFolders;

	public:
		KVirtualizationConfig(KProfile& profile, KxXMLNode& node);

	public:
		size_t GetEntriesCount(KPVEntryType index) const;
		const KVirtualizationEntry* GetEntryAt(KPVEntryType index, size_t i) const;

		const KxStringVector& GetMandatoryVirtualFolders() const
		{
			return m_MandatoryVirtualFolders;
		}
};
