#include "stdafx.h"
#include "KModManagerDispatcher.h"
#include "KModManager.h"
#include "KModEntry.h"
#include "KVariablesDatabase.h"
#include "KApp.h"
#include "KAux.h"
#include <KxFramework/KxFileFinder.h>
#include <wx/filename.h>

wxString KMMDispatcherCollision::GetLocalizedCollisionName(KMMDispatcherCollisionType type)
{
	switch (type)
	{
		case KMM_DCT_NONE:
		{
			return T(KxID_NONE);
		}
		case KMM_DCT_UNKNOWN:
		{
			return T("ModExplorer.Collision.Unknown");
		}
		case KMM_DCT_OVERWRITTEN:
		{
			return T("ModExplorer.Collision.Overwritten");
		}
		case KMM_DCT_OVERWRITES:
		{
			return T("ModExplorer.Collision.Owerwrites");
		}
	};
	return wxEmptyString;
}

//////////////////////////////////////////////////////////////////////////
wxString KModManagerDispatcher::GetTargetPath(const wxString& relativePath, KModEntry** owningMod) const
{
	// This is absolute path, return it as is
	if (relativePath.Length() >= 2 && relativePath[1] == wxT(':'))
	{
		KxUtility::SetIfNotNull(owningMod, nullptr);
		return relativePath;
	}

	wxString outPath;
	auto CheckIn = [&outPath, &relativePath](KModEntry* modEntry)
	{
		if (modEntry->IsEnabled())
		{
			wxString path = modEntry->GetLocation(KMM_LOCATION_MOD_FILES) + '\\' + relativePath;
			if (wxFileName::Exists(path))
			{
				outPath = path;
				return true;
			}
		}
		return false;
	};
	auto CheckIn2 = [&outPath, &relativePath](KModEntry* modEntry)
	{
		if (modEntry->IsEnabledUnchecked())
		{
			KFileTreeNode* node = KFileTreeNode::NavigateToAny(modEntry->GetFileTree(), relativePath);
			if (node)
			{
				outPath = node->GetFullPath();
				return true;
			}
		}
		return false;
	};
	
	KModEntry* modEntry = IterateOverMods(CheckIn2, IterationOrder::Reversed);
	KxUtility::SetIfNotNull(owningMod, modEntry);

	// Fallback to write target
	if (outPath.IsEmpty())
	{
		return KModManager::Get().GetModEntry_WriteTarget()->GetLocation(KMM_LOCATION_MOD_FILES) + '\\' + relativePath;
	}
	return outPath;
}
KModManagerDispatcher::FilesVector KModManagerDispatcher::FindFiles(const wxString& relativePath, const wxString& filter, KxFileSearchType type, bool recurse) const
{
	struct Comparator
	{
		bool operator()(const wxString& lhs, const wxString& rhs) const
		{
			return KxString::ToLower(lhs) == KxString::ToLower(rhs);
		}
	};
	std::unordered_set<wxString, std::hash<wxString>, Comparator> hash;
	hash.reserve(KModManager::Get().GetEntries().size());

	FilesVector finalResults;
	finalResults.reserve(KModManager::Get().GetEntries().size());

	auto FindIn = [&relativePath, &filter, type, &hash, &finalResults](const KModEntry* modEntry)
	{
		if (modEntry->IsEnabled())
		{
			KxFileFinder tFinder(modEntry->GetLocation(KMM_LOCATION_MOD_FILES) + '\\' + relativePath, filter);

			KxFileFinderItem item = tFinder.FindNext();
			while (item.IsOK())
			{
				if (item.IsNormalItem() && item.IsElementType(type))
				{
					const auto& v = hash.insert(item.GetFullPath());
					if (v.second)
					{
						finalResults.push_back(item);
					}
				}
				item = tFinder.FindNext();
			}
		}

		return false;
	};
	auto FindIn2 = [&relativePath, &filter, type, &hash, &finalResults](const KModEntry* modEntry)
	{
		if (modEntry->IsEnabledUnchecked())
		{
			KFileTreeNode* folderNode = KFileTreeNode::NavigateToFolder(modEntry->GetFileTree(), relativePath);
			if (folderNode)
			{
				for (const KFileTreeNode& node: folderNode->GetChildren())
				{
					if (node.GetItem().IsElementType(type) && node.GetName().Matches(filter))
					{
						const auto& v = hash.insert(node.GetFullPath());
						if (v.second)
						{
							finalResults.push_back(node.GetItem());
						}
					}
				}
			}
		}
		return false;
	};

	if (recurse)
	{
		// TODO: Implement recursive search
	}
	else
	{
		IterateOverMods(FindIn2, IterationOrder::Reversed);
	}
	return finalResults;
}
KModManagerDispatcher::CollisionVector KModManagerDispatcher::FindCollisions(const KModEntry* scannedMod, const wxString& relativePath) const
{
	CollisionVector collisions;
	KMMDispatcherCollisionType type = KMM_DCT_OVERWRITTEN;
	auto CheckMod = [scannedMod, &collisions, &type, &relativePath](KModEntry* modEntry)
	{
		// Change collision type after this mod is found
		if (modEntry == scannedMod)
		{
			type = KMM_DCT_OVERWRITES;

			// Don't count scanned mod itself
			return false;
		}

		if (modEntry->IsEnabledUnchecked())
		{
			wxString path = modEntry->GetLocation(KMM_LOCATION_MOD_FILES) + '\\' + relativePath;
			if (wxFileName::Exists(path))
			{
				collisions.emplace_back(modEntry, type);
			}
		}
		return false;
	};
	
	IterateOverMods(CheckMod, IterationOrder::Reversed);
	std::reverse(collisions.begin(), collisions.end());
	return collisions;
}
