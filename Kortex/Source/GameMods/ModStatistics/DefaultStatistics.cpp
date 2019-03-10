#include "stdafx.h"
#include "DefaultStatistics.h"
#include <Kortex/ModManager.hpp>

namespace
{
	using namespace Kortex;

	enum class CountMode
	{
		All,
		Active,
		Inactive,
	};
	enum class FSElementType
	{
		File,
		Folder,
		Any
	};

	int64_t CountMods(CountMode mode)
	{
		int64_t count = 0;
		for (const IGameMod* mod: IModManager::GetInstance()->GetAllMods(false, true))
		{
			switch (mode)
			{
				case CountMode::Active:
				{
					if (mod->IsActive())
					{
						count++;
					}
					break;
				}
				case CountMode::Inactive:
				{
					if (!mod->IsActive())
					{
						count++;
					}
					break;
				}
				default:
				{
					count++;
					break;
				}
			};
		}
		return count;
	}
	int64_t CountFilesAndFolders(FSElementType type, CountMode mode)
	{
		int64_t count = 0;
		for (const IGameMod* modEntry: IModManager::GetInstance()->GetAllMods(false, true))
		{
			modEntry->GetFileTree().WalkTree([&count, type, mode](const FileTreeNode& rootNode)
			{
				for (const FileTreeNode& node: rootNode.GetChildren())
				{
					if (type == FSElementType::Any || (node.IsFile() && type == FSElementType::File) || (node.IsDirectory() && type == FSElementType::Folder))
					{
						switch (mode)
						{
							case CountMode::Active:
							{
								if (rootNode.GetMod().IsActive())
								{
									count++;
								}
								break;
							}
							case CountMode::Inactive:
							{
								if (!rootNode.GetMod().IsActive())
								{
									count++;
								}
								break;
							}
							default:
							{
								count++;
								break;
							}
						};
					}
				}
				return true;
			});
		}
		return count;
	}
	int64_t CalcUsedSize(CountMode mode)
	{
		int64_t totalSize = 0;
		for (const IGameMod* modEntry: IModManager::GetInstance()->GetAllMods(false, true))
		{
			if ((mode == CountMode::Active && !modEntry->IsActive()) || (mode == CountMode::Inactive && modEntry->IsActive()))
			{
				continue;
			}

			modEntry->GetFileTree().WalkTree([&totalSize](const FileTreeNode& rootNode)
			{
				for (const FileTreeNode& node: rootNode.GetChildren())
				{
					if (node.IsFile())
					{
						totalSize += node.GetFileSize();
					}
				}
				return true;
			});
		}
		return totalSize;
	}
}

namespace Kortex::ModStatistics
{
	int64_t DefaultStatistics::GetStatValueInt(const StatInfo& stat) const
	{
		switch ((StatIndex)stat.GetIndex())
		{
			case StatIndex::TotalMods:
			{
				return CountMods(CountMode::All);
			}
			case StatIndex::ActiveMods:
			{
				return CountMods(CountMode::Active);
			}
			case StatIndex::InactiveMods:
			{
				return CountMods(CountMode::Inactive);
			}
			case StatIndex::FilesCount:
			{
				return CountFilesAndFolders(FSElementType::File, CountMode::Active);
			}
			case StatIndex::FoldersCount:
			{
				return CountFilesAndFolders(FSElementType::Folder, CountMode::Active);
			}
			case StatIndex::FilesAndFoldersCount:
			{
				return CountFilesAndFolders(FSElementType::Any, CountMode::Active);
			}
			case StatIndex::UsedSpace:
			{
				return CalcUsedSize(CountMode::All);
			}
		};
		return -1;
	}

	size_t DefaultStatistics::GetStatCount() const
	{
		return (size_t)StatIndex::MAX;
	}
	bool DefaultStatistics::HasStat(const StatInfo& stat) const
	{
		return stat.GetIndex() >= (int)StatIndex::MIN && stat.GetIndex() < (int)StatIndex::MAX;
	}
	
	wxString DefaultStatistics::GetStatName(const StatInfo& stat) const
	{
		switch ((StatIndex)stat.GetIndex())
		{
			case StatIndex::TotalMods:
			{
				return KTr("ModManager.Statistics.ModCountTotal");
			}
			case StatIndex::ActiveMods:
			{
				return KTr("ModManager.Statistics.ModCountActive");
			}
			case StatIndex::InactiveMods:
			{
				return KTr("ModManager.Statistics.ModCountInactive");
			}
			case StatIndex::FilesCount:
			{
				return KTr("ModManager.Statistics.FilesCount");
			}
			case StatIndex::FoldersCount:
			{
				return KTr("ModManager.Statistics.FoldersCount");
			}
			case StatIndex::FilesAndFoldersCount:
			{
				return KTr("ModManager.Statistics.FilesAndFoldersCount");
			}
			case StatIndex::UsedSpace:
			{
				return KTr("ModManager.Statistics.UsedSpace");
			}
		};
		return wxEmptyString;
	}
	wxString DefaultStatistics::GetStatValue(const StatInfo& stat) const
	{
		return std::to_wstring(GetStatValueInt(stat));
	}
}
