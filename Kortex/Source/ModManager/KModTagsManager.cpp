#include "stdafx.h"
#include "KModTagsManager.h"
#include "KModEntry.h"
#include "GameInstance/KInstanceManagement.h"
#include "PackageManager/KPackageManager.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxComparator.h>
#include <KxFramework/KxFileStream.h>
#include <KxFramework/KxXML.h>

KModTag::KModTag(const wxString& value, const wxString& label, bool isSystemTag)
	:KLabeledValue(value, label), m_IsSystemTag(isSystemTag)
{
}
KModTag::~KModTag()
{
}

//////////////////////////////////////////////////////////////////////////
wxString KModTagsManager::GetDefaultTagsFile()
{
	return KApp::Get().GetDataFolder() + wxS("\\ModManager\\DefaultTags.xml");
}

void KModTagsManager::LoadTagsFromFile(const wxString& filePath)
{
	KxFileStream stream(filePath, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	KxXMLDocument xml(stream);

	bool hasSE = KPackageManager::GetInstance()->HasScriptExtender();
	for (KxXMLNode node = xml.GetFirstChildElement("Tags").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		bool requiresSE = node.GetAttributeBool("RequiresScriptExtender");
		if (requiresSE && !hasSE)
		{
			break;
		}

		wxString id = node.GetAttribute("ID");
		wxString label = node.GetAttribute("Label", id);
		bool isSuccess = false;
		wxString labelT = KTranslation::GetTranslation().GetString("ModManager.Tag." + label, &isSuccess);
		if (isSuccess)
		{
			label = labelT;
		}

		KModTag& tag = m_Tags.emplace_back(KModTag(id, V(label), isSuccess));
		tag.SetNexusID(node.GetAttributeInt("NexusID"));
	}
}
void KModTagsManager::OnInit()
{
	LoadUserTags();
	if (IsTagListEmpty())
	{
		LoadDefaultTags();
	}
	
	// Remove duplicates
	auto it = std::unique(m_Tags.begin(), m_Tags.end(), [](const KModTag& tag1, const KModTag& tag2)
	{
		return KxComparator::IsEqual(tag1.GetValue(), tag2.GetValue(), false);
	});
	m_Tags.erase(it, m_Tags.end());
}

KModTagsManager::KModTagsManager()
{
}
KModTagsManager::~KModTagsManager()
{
	SaveUserTags();
}

const KModTag* KModTagsManager::FindModTag(const wxString& tagValue, KModTagArray::const_iterator* itOut) const
{
	auto it = std::find_if(m_Tags.begin(), m_Tags.end(), [tagValue](const KModTag& tag)
	{
		return tagValue == tag.GetValue();
	});

	if (it != m_Tags.cend())
	{
		KxUtility::SetIfNotNull(itOut, it);
		return &(*it);
	}
	else
	{
		KxUtility::SetIfNotNull(itOut, m_Tags.end());
		return NULL;
	}
}
const KModTag* KModTagsManager::AddModTag(const KModTag& tag)
{
	const KModTag* thisTag = FindModTag(tag.GetValue());
	if (!thisTag)
	{
		m_Tags.push_back(tag);
		return &m_Tags.back();
	}
	return thisTag;
}
bool KModTagsManager::RemoveModTag(const wxString& tagValue)
{
	KModTagArray::const_iterator it;
	const KModTag* tag = FindModTag(tagValue, &it);
	if (tag && !tag->IsSystemTag())
	{
		m_Tags.erase(it);
		return true;
	}
	return false;
}
const wxString& KModTagsManager::GetTagName(const wxString& tagID) const
{
	const KModTag* tag = FindModTag(tagID);
	if (tag)
	{
		return tag->GetLabel();
	}
	return wxNullString;
}
void KModTagsManager::LoadTagsFromEntry(const KModEntry* entry)
{
	for (const wxString& tagName: entry->GetTags())
	{
		AddModTag(tagName);
	}
}

wxString KModTagsManager::GetUserTagsFile() const
{
	return KGameInstance::GetActive()->GetModTagsFile();
}
void KModTagsManager::SaveUserTags() const
{
	KxXMLDocument xml;
	KxXMLNode rootNode = xml.NewElement("Tags");

	for (const KModTag& tag: m_Tags)
	{
		KxXMLNode node = rootNode.NewElement("Entry");
		node.SetAttribute("ID", tag.GetValue());
		node.SetAttribute("Name", (tag.IsSystemTag() || tag.GetValue() == tag.GetLabel() ? tag.GetValue() : tag.GetLabel()));
	}

	KxFileStream stream(GetUserTagsFile(), KxFS_ACCESS_WRITE, KxFS_DISP_CREATE_ALWAYS, KxFS_SHARE_READ);
	xml.Save(stream);
}
