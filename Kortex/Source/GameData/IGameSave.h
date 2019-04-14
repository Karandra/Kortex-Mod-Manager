#pragma once
#include "stdafx.h"
#include "SaveManager/SaveInfoPair.h"
#include <KxFramework/KxFileItem.h>

namespace Kortex
{
	namespace SaveManager
	{
		class DisplayModel;
	}

	class IGameSave: public RTTI::IInterface<IGameSave>
	{
		friend class SaveManager::DisplayModel;

		public:
			using Vector = std::vector<std::unique_ptr<IGameSave>>;
			using RefVector = std::vector<IGameSave*>;
			using CRefVector = std::vector<const IGameSave*>;

			using InfoPairVector = std::vector<SaveManager::SaveInfoPair>;

		public:
			static wxImage ReadImageRGB(const KxUInt8Vector& rgbData, int width, int height, int alphaOverride = -1, bool isStaticData = false);
			static wxImage ReadImageRGBA(const KxUInt8Vector& rgbaData, int width, int height, int alphaOverride = -1);
			static wxBitmap ReadBitmapRGB(const KxUInt8Vector& rgbData, int width, int height, int alphaOverride = -1)
			{
				return wxBitmap(ReadImageRGB(rgbData, width, height, alphaOverride, true), 32);
			}
			static wxBitmap ReadBitmapRGBA(const KxUInt8Vector& rgbaData, int width, int height, int alphaOverride = -1)
			{
				return wxBitmap(ReadImageRGBA(rgbaData, width, height, alphaOverride), 32);
			}

		protected:
			virtual bool OnCreate(KxFileItem& fileItem) = 0;
			virtual bool OnRead(const KxFileItem& fileItem) = 0;

			virtual wxBitmap GetThumbBitmap() const = 0;
			virtual bool HasThumbBitmap() const = 0;
			virtual void SetThumbBitmap(const wxBitmap& bitmap) = 0;
			virtual void ResetThumbBitmap() = 0;

		public:
			virtual bool IsOK() const = 0;
			virtual bool Create(const wxString& filePath) = 0;
			virtual bool ReadFile() = 0;

			virtual const KxFileItem& GetFileItem() const = 0;
			virtual KxFileItem& GetFileItem() = 0;

			virtual wxBitmap GetBitmap() const = 0;
			virtual const InfoPairVector& GetBasicInfo() const = 0;

			virtual wxString GetDisplayName() const = 0;
	};
}
