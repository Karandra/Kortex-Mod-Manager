#include "stdafx.h"
#include "KProgramManagerConfig.h"
#include "KProfile.h"
#include "KApp.h"
#include "KAux.h"

KProgramManagerConfig::KProgramManagerConfig(KProfile& profile, const KxXMLNode& node)
{
	auto LoadArray = [&node](KRMProgramEntryArray& array, const char* name)
	{
		for (KxXMLNode entryNode = node.GetFirstChildElement(name).GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
		{
			if (!array.emplace_back(KProgramManagerEntry(entryNode)).IsOK())
			{
				array.pop_back();
			}
		}
	};

	LoadArray(m_EntriesMain, "Main");
	LoadArray(m_EntriesPre, "PreMain");
	LoadArray(m_EntriesPost, "PostMain");
}

size_t KProgramManagerConfig::GetEntriesCount(ProgramType type) const
{
	switch (type)
	{
		case ProgramType::Main:
		{
			return m_EntriesMain.size();
		}
		case ProgramType::PreMain:
		{
			return m_EntriesPre.size();
		}
		case ProgramType::PostMain:
		{
			return m_EntriesPost.size();
		}
	};
	return 0;
}
const KProgramManagerEntry* KProgramManagerConfig::GetEntryAt(ProgramType type, size_t i) const
{
	const KRMProgramEntryArray* pEntries = NULL;
	switch (type)
	{
		case ProgramType::Main:
		{
			pEntries = &m_EntriesMain;
			break;
		}
		case ProgramType::PreMain:
		{
			pEntries = &m_EntriesPre;
			break;
		}
		case ProgramType::PostMain:
		{
			pEntries = &m_EntriesPost;
			break;
		}
	};

	if (pEntries && i < pEntries->size())
	{
		return &pEntries->at(i);
	}
	return NULL;
}
