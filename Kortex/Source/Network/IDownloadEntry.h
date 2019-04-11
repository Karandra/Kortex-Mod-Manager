#pragma once
#include "stdafx.h"
#include "Network/Common.h"
#include "GameInstance/GameID.h"
#include <KxFramework/KxISerializer.h>
class KxFileItem;

namespace Kortex
{
	class IGameMod;
	class INetworkModSource;

	class IModFileInfo;
	class IModDownloadInfo;
}

namespace Kortex
{
	class IGameInstance;

	class IDownloadEntry: public KxISerializer
	{
		public:
			using Vector = std::vector<std::unique_ptr<IDownloadEntry>>;
			using RefVector = std::vector<IDownloadEntry*>;

		public:
			virtual bool IsOK() const = 0;

			virtual wxString GetFullPath() const = 0;
			virtual wxString GetMetaFilePath() const = 0;
		
			virtual const IGameInstance* GetTargetGame() const = 0;
			virtual void SetTargetGame(const IGameInstance* instance) = 0;
			GameID GetTargetGameID() const;
			void SetTargetGameID(const GameID& id);

			virtual const IGameMod* GetMod() const = 0;
			virtual bool IsInstalled() const = 0;

			virtual const INetworkModSource* GetModSource() const = 0;
			virtual void SetModSource(const INetworkModSource* modSource) = 0;
			bool HasModSource() const;
			bool IsModSourceOfType(ModSourceID sourceID) const;
			template<class T> bool IsModSourceOfType() const
			{
				if (const INetworkModSource* modSource = GetModSource())
				{
					return modSource->GetID() == T::GetTypeID();
				}
				return false;
			}
			
			virtual wxDateTime GetDate() const = 0;
			virtual void SetDate(const wxDateTime& date) = 0;
			
			virtual int64_t GetDownloadedSize() const = 0;
			virtual void SetDownloadedSize(int64_t size) = 0;

			virtual int64_t GetSpeed() const = 0;
			virtual bool IsCompleted() const = 0;
			virtual bool CanRestart() const = 0;

			virtual const IModFileInfo& GetFileInfo() const = 0;
			virtual IModFileInfo& GetFileInfo() = 0;

			virtual const IModDownloadInfo& GetDownloadInfo() const = 0;
			virtual IModDownloadInfo& GetDownloadInfo() = 0;

			virtual bool IsRunning() const = 0;
			virtual bool IsPaused() const = 0;
			virtual bool IsFailed() const = 0;

			virtual bool IsHidden() const = 0;
			virtual void SetHidden(bool value) = 0;

			virtual void SetPaused(bool value) = 0;
			virtual void SetFailed(bool value) = 0;

			virtual void Stop() = 0;
			virtual void Pause() = 0;
			virtual void Resume() = 0;
			virtual void Run(int64_t resumeFrom = 0) = 0;
			virtual bool Restart() = 0;

			// Restores download info depending of this download modSource and its filename
			// which is set in 'DefaultDownloadEntry::DeSerializeDefault' if something goes wrong.
			// Restoration is performed by analyzing file name to get file id and mod id
			// and querying rest of the information form internet.
			virtual bool RepairBrokedDownload() = 0;
			virtual bool QueryInfo() = 0;

		public:
			virtual void LoadDefault(const KxFileItem& fileItem) = 0;

			bool Save() const;
			bool Load(const wxString& xmlFile, const KxFileItem& fileItem);
	};
}
