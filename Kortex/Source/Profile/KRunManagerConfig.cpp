#include "stdafx.h"
#include "KRunManagerConfig.h"
#include "KProfile.h"
#include "KApp.h"
#include "KAux.h"

KxSingletonPtr_Define(KRunManagerConfig);

KRunManagerConfig::KRunManagerConfig(KProfile& profile, KxXMLNode& node)
{
	auto LoadArray = [&node](KRMProgramEntryArray& array, const char* name)
	{
		for (KxXMLNode entryNode = node.GetFirstChildElement(name).GetFirstChildElement(); entryNode.IsOK(); entryNode = entryNode.GetNextSiblingElement())
		{
			if (!array.emplace_back(KRunManagerProgram(entryNode)).IsOK())
			{
				array.pop_back();
			}
		}
	};

	LoadArray(m_EntriesMain, "Main");
	LoadArray(m_EntriesPre, "PreMain");
	LoadArray(m_EntriesPost, "PostMain");
}

size_t KRunManagerConfig::GetEntriesCount(KPRCEType type) const
{
	switch (type)
	{
		case KPRCE_TYPE_MAIN:
		{
			return m_EntriesMain.size();
		}
		case KPRCE_TYPE_PREMAIN:
		{
			return m_EntriesPre.size();
		}
		case KPRCE_TYPE_POSTMAIN:
		{
			return m_EntriesPost.size();
		}
	};
	return 0;
}
const KRunManagerProgram* KRunManagerConfig::GetEntryAt(KPRCEType type, size_t i) const
{
	const KRMProgramEntryArray* pEntries = NULL;
	switch (type)
	{
		case KPRCE_TYPE_MAIN:
		{
			pEntries = &m_EntriesMain;
			break;
		}
		case KPRCE_TYPE_PREMAIN:
		{
			pEntries = &m_EntriesPre;
			break;
		}
		case KPRCE_TYPE_POSTMAIN:
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
