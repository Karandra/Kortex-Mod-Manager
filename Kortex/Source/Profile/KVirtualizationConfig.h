#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>
class KProfile;

class KVirtualizationMirroredEntry
{
	public:
		using Vector = std::vector<KVirtualizationMirroredEntry>;

	private:
		KxStringVector m_Sources;
		wxString m_Target;

	public:
		KVirtualizationMirroredEntry(KxXMLNode& parentNode);

	public:
		bool IsOK() const
		{
			return !m_Sources.empty() && !m_Target.IsEmpty();
		}
		bool ShouldUseMultiMirror() const
		{
			return m_Sources.size() > 1;
		}

		KxStringVector GetSources() const;
		wxString GetSource() const;
		wxString GetTarget() const;
};

class KVirtualizationMandatoryEntry
{
	public:
		using Vector = std::vector<KVirtualizationMandatoryEntry>;

	private:
		wxString m_Source;
		wxString m_Name;

	public:
		KVirtualizationMandatoryEntry(KxXMLNode& parentNode);

	public:
		bool IsOK() const
		{
			return !m_Source.IsEmpty();
		}

		wxString GetSource() const;
		wxString GetName() const;
};

//////////////////////////////////////////////////////////////////////////
class KVirtualizationConfig: public KxSingletonPtr<KVirtualizationConfig>
{
	private:
		KVirtualizationMirroredEntry::Vector m_MirroredLocations;
		KVirtualizationMandatoryEntry::Vector m_MandatoryVirtualFolders;

	public:
		KVirtualizationConfig(KProfile& profile, KxXMLNode& node);

	public:
		const KVirtualizationMirroredEntry::Vector& GetMirroredLocations() const
		{
			return m_MirroredLocations;
		}
		const KVirtualizationMandatoryEntry::Vector& GetMandatoryLocations() const
		{
			return m_MandatoryVirtualFolders;
		}
};
