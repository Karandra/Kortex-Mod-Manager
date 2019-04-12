#pragma once
#include "stdafx.h"
#include "Common.h"
#include "NetworkProviderRequest.h"
#include "NetworkProviderReply.h"
#include "GameInstance/GameID.h"
#include "Utility/KImageProvider.h"
#include <KxFramework/KxQueryInterface.h>
#include <KxFramework/KxSecretStore.h>
#include <KxFramework/KxVersion.h>

namespace Kortex
{
	class IDownloadEntry;
}

namespace Kortex
{
	class IModSource: public KxRTTI::IInterface<IModSource>
	{
		friend class INetworkManager;

		public:
			using Vector = std::vector<std::unique_ptr<IModSource>>;
			using RefVector = std::vector<IModSource*>;

		public:
			static KImageEnum GetGenericIcon();

		private:
			template<class T> static std::unique_ptr<T> Create()
			{
				auto modSource = std::make_unique<T>();
				modSource->Init();
				return modSource;
			}

		private:
			wxBitmap m_UserPicture;

		protected:
			wxString ConstructIPBModURL(int64_t modID, const wxString& modSignature = wxEmptyString) const;
			wxBitmap DownloadSmallBitmap(const wxString& url) const;

			virtual void Init();

		public:
			bool IsDefault() const;

			bool HasUserPicture() const
			{
				return m_UserPicture.IsOk();
			}
			wxBitmap GetUserPicture() const
			{
				return HasUserPicture() ? m_UserPicture : KGetBitmap(GetIcon());
			}
			void SetUserPicture(const wxBitmap& picture)
			{
				m_UserPicture = picture;
				m_UserPicture.SaveFile(GetUserPictureFile(), wxBITMAP_TYPE_PNG);
			}

			wxString GetCacheFolder() const;
			wxString GetUserPictureFile() const;

			virtual KImageEnum GetIcon() const = 0;
			virtual wxString GetName() const = 0;
			virtual wxString GetGameID(const GameID& id = GameIDs::NullGameID) const = 0;
			virtual wxString& ConvertDescriptionToHTML(wxString& description) const
			{
				return description;
			}
			virtual wxString GetModURLBasePart(const GameID& id = GameIDs::NullGameID) const = 0;
			virtual wxString GetModURL(const ProviderRequest& request) = 0;

			virtual std::unique_ptr<IModInfo> NewModInfo() const = 0;
			virtual std::unique_ptr<IModFileInfo> NewModFileInfo() const = 0;
			virtual std::unique_ptr<IModDownloadInfo> NewModDownloadInfo() const = 0;
			virtual std::unique_ptr<IModEndorsementInfo> NewModEndorsementInfo() const = 0;

			virtual bool RestoreBrokenDownload(const wxString& filePath, IDownloadEntry& download)
			{
				return false;
			}

			virtual std::unique_ptr<IModInfo> GetModInfo(const ProviderRequest& request) const = 0;
			virtual std::unique_ptr<IModFileInfo> GetFileInfo(const ProviderRequest& request) const = 0;
			virtual IModFileInfo::Vector GetFilesList(const ProviderRequest& request) const = 0;
			virtual IModDownloadInfo::Vector GetFileDownloadLinks(const ProviderRequest& request) const = 0;
			virtual std::unique_ptr<IModEndorsementInfo> EndorseMod(const ProviderRequest& request, ModEndorsement state) = 0;
	};
}
