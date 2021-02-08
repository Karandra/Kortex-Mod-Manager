#pragma once
#include "Framework.hpp"

namespace kxf
{
	class IImage2D;
	class XMLDocument;
}
namespace Kortex
{
	class IGameInstance;
}

namespace Kortex
{
	class IGameDefinition: public kxf::RTTI::Interface<IGameDefinition>
	{
		KxRTTI_DeclareIID(IGameDefinition, {0xdc0d3d32, 0x72fa, 0x4ab3, {0xbc, 0xfc, 0x3c, 0x19, 0x8, 0x1f, 0x56, 0x2}});

		public:
			enum class Location
			{
				Root,
				Game,
				Resources
			};

		public:
			static bool ValidateName(const kxf::String& name, kxf::String* validName = nullptr);
			static std::unique_ptr<kxf::IImage2D> GetGenericIcon();

		protected:
			std::unique_ptr<kxf::IImage2D> LoadIcon(const kxf::IFileSystem& fs, const kxf::FSPath& path) const;

		public:
			virtual ~IGameDefinition() = default;

		public:
			virtual bool IsNull() const = 0;

			virtual kxf::XMLDocument& GetDefinitionData() = 0;
			virtual const kxf::XMLDocument& GetDefinitionData() const = 0;
			virtual bool LoadDefinitionData(const kxf::IFileSystem& fileSystem) = 0;
			virtual bool SaveDefinitionData() = 0;

			virtual kxf::IVariablesCollection& GetVariables() = 0;
			virtual const kxf::IVariablesCollection& GetVariables() const = 0;
			virtual kxf::String ExpandVariables(const kxf::String& variables) const = 0;

			virtual int GetSortOrder() const = 0;
			virtual kxf::String GetName() const = 0;
			virtual kxf::String GetGameName() const = 0;
			virtual kxf::String GetGameShortName() const = 0;
			virtual const kxf::IImage2D& GetIcon() const = 0;
			
			virtual kxf::IFileSystem& GetFileSystem(Location locationID) = 0;
			virtual const kxf::IFileSystem& GetFileSystem(Location locationID) const = 0;

		public:
			kxf::Enumerator<IGameInstance&> EnumLinkedInstances() const;
			IGameInstance* GetLinkedInstanceByName(const kxf::String& name) const;

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
