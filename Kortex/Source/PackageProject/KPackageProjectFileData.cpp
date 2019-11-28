#include "stdafx.h"
#include "KPackageProjectFileData.h"
#include "KPackageProject.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"
#include <KxFramework/KxString.h>

namespace Kortex::PackageProject
{
	KPPFFileEntry::KPPFFileEntry()
		:m_Priority(KPackageProjectFileData::ms_DefaultPriority)
	{
	}
	KPPFFileEntry::~KPPFFileEntry()
	{
	}
	
	void KPPFFileEntry::MakeUniqueID()
	{
		if (m_ID.IsEmpty())
		{
			m_ID = KxString::Format("0x%1", this);
		}
		else
		{
			m_ID = KxString::Format("%s!0x%1", this);
		}
	}
	
	bool KPPFFileEntry::IsDefaultPriority() const
	{
		return m_Priority == KPackageProjectFileData::ms_DefaultPriority;
	}
	int32_t KPPFFileEntry::GetPriority() const
	{
		return m_Priority;
	}
	void KPPFFileEntry::SetPriority(int32_t value)
	{
		m_Priority = KPackageProjectFileData::CorrectPriority(value);
	}
}

namespace Kortex::PackageProject
{
	KPPFFolderEntry::KPPFFolderEntry()
	{
	}
	KPPFFolderEntry::~KPPFFolderEntry()
	{
	}
}

namespace Kortex::PackageProject
{
	bool KPackageProjectFileData::IsPriorityValid(int32_t value)
	{
		return value >= KPackageProjectFileData::ms_MinUserPriority && value <= KPackageProjectFileData::ms_MaxUserPriority;
	}
	int32_t KPackageProjectFileData::CorrectPriority(int32_t value)
	{
		return std::clamp(value, ms_MinUserPriority, ms_MaxUserPriority);
	}
	bool KPackageProjectFileData::IsFileIDValid(const wxString& id)
	{
		if (!id.IsEmpty() && !KAux::HasForbiddenFileNameChars(id))
		{
			wxString idLower = KxString::ToLower(id);
			return idLower != "fomod";
		}
		return false;
	}
	
	KPackageProjectFileData::KPackageProjectFileData(KPackageProject& project)
		:KPackageProjectPart(project)
	{
	}
	KPackageProjectFileData::~KPackageProjectFileData()
	{
	}
	
	KPPFFileEntry* KPackageProjectFileData::FindEntryWithID(const wxString& id, size_t* index) const
	{
		const wxString idLower = KxString::ToLower(id);
		auto it = std::find_if(m_Data.cbegin(), m_Data.cend(), [&idLower](const KPPFFileEntryArray::value_type& entry)
		{
			return KxString::ToLower(entry->GetID()) == idLower;
		});
	
		if (it != m_Data.cend())
		{
			if (index)
			{
				*index = std::distance(m_Data.cbegin(), it);
			}
			return it->get();
		}
		return nullptr;
	}
}
