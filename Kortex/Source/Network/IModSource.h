#pragma once
#include "stdafx.h"
#include "Common.h"
#include "ModRepositoryReply.h"
#include "ModRepositoryRequest.h"
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
			wxString ConstructIPBModURL(int64_t modID, const wxString& modSignature = {}) const;
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
			virtual wxString GetGameID(const GameID& id = {}) const = 0;
			virtual wxString& ConvertDescriptionToHTML(wxString& description) const
			{
				return description;
			}
			virtual wxString GetModURLBasePart(const GameID& id = {}) const = 0;
			virtual wxString GetModURL(const ModRepositoryRequest& request) = 0;
	};
}
