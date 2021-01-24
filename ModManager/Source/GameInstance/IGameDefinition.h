#pragma once
#include "Framework.hpp"
#include "GameID.h"

namespace kxf
{
	class IImage2D;
	class XMLDocument;
}

namespace Kortex
{
	class IGameDefinition: public kxf::RTTI::Interface<IGameDefinition>
	{
		KxRTTI_DeclareIID(IGameDefinition, {0xdc0d3d32, 0x72fa, 0x4ab3, {0xbc, 0xfc, 0x3c, 0x19, 0x8, 0x1f, 0x56, 0x2}});

		public:
			enum class Location
			{
				None = -1,

				Root,
				Resources,
				Game,
				Mods,
				Profiles
			};

		public:
			static bool ValidateID(const kxf::String& id, kxf::String* validID = nullptr);
			static std::unique_ptr<kxf::IImage2D> GetGenericIcon();

		protected:
			std::unique_ptr<kxf::IImage2D> LoadIcon(const kxf::IFileSystem& fs, const kxf::FSPath& path) const;

		public:
			virtual ~IGameDefinition() = default;

		public:
			virtual bool IsNull() const = 0;

			virtual kxf::IVariablesCollection& GetVariables() = 0;
			virtual kxf::String ExpandVariables(const kxf::String& variables) const = 0;
			virtual kxf::String ExpandVariablesLocally(const kxf::String& variables) const = 0;

			virtual int GetSortOrder() const = 0;
			virtual GameID GetGameID() const = 0;
			virtual kxf::String GetGameName() const = 0;
			virtual kxf::String GetGameShortName() const = 0;
			virtual const kxf::IImage2D& GetIcon() const = 0;
			
			virtual kxf::IFileSystem& GetFileSystem(Location locationID) = 0;
			const kxf::IFileSystem& GetFileSystem(Location locationID) const
			{
				return const_cast<IGameDefinition&>(*this).GetFileSystem(locationID);
			}

		public:
			explicit operator bool() const noexcept
			{
				return !IsNull();
			}
			bool operator!() const noexcept
			{
				return IsNull();
			}
	};
}

namespace Kortex
{
	class IEditableGameDefinition: public kxf::RTTI::Interface<IEditableGameDefinition>
	{
		KxRTTI_DeclareIID(IEditableGameDefinition, {0x2ef792f5, 0x69e7, 0x4f81, {0xb1, 0xbc, 0xfd, 0xfb, 0xc5, 0x4c, 0xe0, 0xda}});

		public:
			virtual kxf::XMLDocument& GetDefinitionData() = 0;
			virtual const kxf::XMLDocument& GetDefinitionData() const = 0;
			virtual bool SaveDefinitionData() = 0;
	};
}
