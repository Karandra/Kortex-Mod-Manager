#pragma once
#include "stdafx.h"
#include "IImageProvider.h"
#include "Application/IApplication.h"

namespace Kortex::Application
{
	class DefaultImageProvider: public IImageProvider
	{
		friend class DefaultApplication;

		private:
			enum class Type
			{
				Default = 0,
				Icon
			};

		private:
			KxImageList m_ImageList;
			KxImageSet m_ImageSet;

		protected:
			template<class TImageID, class TImage> void AddSingleItem(TImageID id, const wxChar* name, const TImage& value)
			{
				m_ImageList.Replace(static_cast<int>(id), value);
				m_ImageSet.Set(name, value);
			};
			template<class TImageID> void LoadItem(TImageID id, const wxChar* name, Type type = Type::Default)
			{
				wxBitmap img(KxString::Format(wxS("%1\\UI\\%2.%3"),
							 IApplication::GetInstance()->GetDataFolder(),
							 name,
							 type == Type::Default ? wxS("png") : wxS("ico")),
							 type == Type::Default ? wxBITMAP_TYPE_PNG : wxBITMAP_TYPE_ICO
				);
				if (img.IsOk())
				{
					AddSingleItem(id, name, img);
				}
			}

			void LoadImages();

		public:
			DefaultImageProvider();

		public:
			const KxImageList& GetImageList() const override
			{
				return m_ImageList;
			}
			const KxImageSet& GetImageSet() const override
			{
				return m_ImageSet;
			}
	};
}
