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

namespace Util
{
	template<class T, class VectorT> T* FindModTag(VectorT& tags, const wxString& tagValue, typename VectorT::const_iterator* iterator = nullptr)
	{
		auto it = std::find_if(tags.begin(), tags.end(), [tagValue](const KModTag& tag)
		{
			return tagValue == tag.GetValue();
		});

		if (it != tags.end())
		{
			KxUtility::SetIfNotNull(iterator, it);
			return &(*it);
		}
		else
		{
			KxUtility::SetIfNotNull(iterator, tags.end());
			return nullptr;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
KModTag::KModTag(const wxString& value, const wxString& label, bool isSystemTag)
	:KLabeledValue(value, label), m_IsSystemTag(isSystemTag)
{
}
KModTag::~KModTag()
{
}

//////////////////////////////////////////////////////////////////////////
wxString KModTagsManager::GetUserTagsFile() const
{
	return KGameInstance::GetActive()->GetModTagsFile();
}
void KModTagsManager::LoadTagsFromFile(const wxString& filePath)
{
	KxFileStream stream(filePath, KxFileStream::Access::Read, KxFileStream::Disposition::OpenExisting, KxFileStream::Share::Read);
	KxXMLDocument xml(stream);

	const bool hasSE = KPackageManager::GetInstance()->HasScriptExtender();
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
		wxString labelTranslated = KTranslation::GetAppTranslation().GetString("ModManager.Tag." + label, &isSuccess);
		if (isSuccess)
		{
			label = labelTranslated;
		}

		KModTag& tag = m_Tags.emplace_back(KModTag(id, KVarExp(label), isSuccess));
		tag.SetNexusID(node.GetAttributeInt("NexusID"));

		// Color
		KxXMLNode colorNode = node.GetFirstChildElement("Color");
		if (colorNode.IsOK())
		{
			int r = colorNode.GetAttributeInt("R", -1);
			int g = colorNode.GetAttributeInt("G", -1);
			int b = colorNode.GetAttributeInt("B", -1);
			if (r >= 0 && g >= 0 && b >= 0)
			{
				tag.SetColor(KxColor(r, g, b, 200));
			}
		}
	}
}
void KModTagsManager::LoadDefaultTags()
{
	LoadTagsFromFile(KApp::Get().GetDataFolder() + wxS("\\ModManager\\DefaultTags.xml"));
}

void KModTagsManager::OnInit()
{
	LoadUserTags();
	if (HasTags())
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
void KModTagsManager::LoadUserTags()
{
	LoadTagsFromFile(GetUserTagsFile());
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

		// Color
		if (tag.HasColor())
		{
			KxXMLNode colorNode = node.NewElement("Color");
			
			KxColor color = tag.GetColor();
			colorNode.SetAttribute("R", color.GetR());
			colorNode.SetAttribute("G", color.GetG());
			colorNode.SetAttribute("B", color.GetB());
		}
	}

	KxFileStream stream(GetUserTagsFile(), KxFileStream::Access::Write, KxFileStream::Disposition::CreateAlways, KxFileStream::Share::Read);
	xml.Save(stream);
}

KModTagsManager::KModTagsManager()
{
}
KModTagsManager::~KModTagsManager()
{
	SaveUserTags();
}

const KModTag* KModTagsManager::FindModTag(const wxString& tagValue, KModTag::Vector::const_iterator* iterator) const
{
	return Util::FindModTag<const KModTag>(m_Tags, tagValue);
}
KModTag* KModTagsManager::FindModTag(const wxString& tagValue, KModTag::Vector::const_iterator* iterator)
{
	return Util::FindModTag<KModTag>(m_Tags, tagValue);
}

KModTag* KModTagsManager::AddModTag(const KModTag& tag)
{
	KModTag* thisTag = FindModTag(tag.GetValue());
	if (!thisTag)
	{
		m_Tags.emplace_back(tag);
		return &m_Tags.back();
	}
	return thisTag;
}
KModTag* KModTagsManager::AddModTag(const wxString& name)
{
	// Add tag with value only.
	return AddModTag(KModTag(name));
}

bool KModTagsManager::RemoveModTag(const wxString& tagValue)
{
	KModTag::Vector::const_iterator it;
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
void KModTagsManager::LoadTagsFromEntry(const KModEntry& entry)
{
	for (const wxString& tagName: entry.GetTags())
	{
		AddModTag(tagName);
	}
}
