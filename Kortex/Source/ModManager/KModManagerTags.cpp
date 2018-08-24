#include "stdafx.h"
#include "KModManagerTags.h"
#include "KModEntry.h"
#include "Profile/KProfile.h"
#include "PackageManager/KPackageManager.h"
#include "KApp.h"
#include "KAux.h"
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
wxString KModManagerTags::GetDefaultTagsFile()
{
	return KApp::Get().GetDataFolder() + "\\ModManager\\DefaultTags.xml";
}
wxString KModManagerTags::GetUserTagsFile(const wxString& templateID, const wxString& configID)
{
	return KProfile::GetDataPath(templateID, configID) + '\\' + "ModTags.xml";
}

void KModManagerTags::LoadTagsFromFile(const wxString& filePath)
{
	KxFileStream stream(filePath, KxFS_ACCESS_READ, KxFS_DISP_OPEN_EXISTING, KxFS_SHARE_READ);
	KxXMLDocument xml(stream);

	bool hasSE = KPackageManager::Get().HasScriptExtender();
	for (KxXMLNode node = xml.GetFirstChildElement("Tags").GetFirstChildElement(); node.IsOK(); node = node.GetNextSiblingElement())
	{
		bool requiresSE = node.GetAttributeBool("RequiresSE");
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

		m_Tags.emplace_back(KModTag(id, V(label), isSuccess));
	}
}

KModManagerTags::KModManagerTags()
{
	LoadUserTags();
	if (IsTagListEmpty())
	{
		LoadDefaultTags();
	}
}
KModManagerTags::~KModManagerTags()
{
	SaveUserTags();
}

const KModTag* KModManagerTags::FindModTag(const wxString& tagValue, KModTagArray::const_iterator* itOut) const
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
const KModTag* KModManagerTags::AddModTag(const KModTag& tag)
{
	const KModTag* thisTag = FindModTag(tag.GetValue());
	if (!thisTag)
	{
		m_Tags.push_back(tag);
		return &m_Tags.back();
	}
	return thisTag;
}
bool KModManagerTags::RemoveModTag(const wxString& tagValue)
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
const wxString& KModManagerTags::GetTagName(const wxString& tagID) const
{
	const KModTag* tag = FindModTag(tagID);
	if (tag)
	{
		return tag->GetLabel();
	}
	return wxNullString;
}
void KModManagerTags::LoadTagsFromEntry(const KModEntry* entry)
{
	for (const wxString& tagName: entry->GetTags())
	{
		AddModTag(tagName);
	}
}

wxString KModManagerTags::GetUserTagsFile() const
{
	return GetUserTagsFile(KApp::Get().GetCurrentTemplateID(), KApp::Get().GetCurrentConfigID());
}
void KModManagerTags::SaveUserTags() const
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
