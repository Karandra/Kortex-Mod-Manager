#pragma once
#include "Framework.hpp"

namespace Kortex
{
	class ModPackageProject;
	class FileTreeNode;
	class ModTagStore;
	class ModSourceStore;
}

namespace Kortex
{
	class IGameMod: public kxf::RTTI::Interface<IGameMod>
	{
		KxRTTI_DeclareIID(IGameMod, {0xaa4f8b27, 0xdb94, 0x4e34, {0x92, 0x3d, 0xe9, 0x4c, 0x82, 0xf8, 0xf3, 0xf5}});

		public:
			enum class Location
			{
				None = -1,

				Root,
				Content
			};
			enum class TimeAttribute
			{
				Unknown = -1,

				Creation,
				Install,
				Uninstall
			};

		public:
			virtual bool IsNull() const = 0;

			virtual bool Load(const kxf::IFileSystem& fileSystem) = 0;
			virtual bool Save(kxf::IFileSystem& fileSystem) = 0;
			
			virtual int GetOrder() const = 0;
			virtual int GetDisplayOrder() const = 0;

			virtual kxf::String GetSignature() const = 0;
			virtual kxf::String GetName() const = 0;
			virtual void SetName(const kxf::String& value) = 0;
			
			virtual kxf::String GetAuthor() const = 0;
			virtual void SetAuthor(const kxf::String& value) = 0;

			virtual kxf::Version GetVersion() const = 0;
			virtual void SetVersion(const kxf::Version& value) = 0;

			virtual kxf::String GetDescription() const = 0;
			virtual void SetDescription(const kxf::String& value) = 0;

			virtual kxf::DateTime GetTimeAttribute(TimeAttribute attribute) const = 0;
			virtual void SetTimeAttribute(TimeAttribute attribute, const kxf::DateTime& value) = 0;

			virtual const ModSourceStore& GetModSourceStore() const = 0;
			virtual ModSourceStore& GetModSourceStore() = 0;

			virtual const ModTagStore& GetTagStore() const = 0;
			virtual ModTagStore& GetTagStore() = 0;

			virtual kxf::FSPath GetPackageFile() const = 0;
			virtual void SetPackageFile(const kxf::FSPath& value) = 0;
			
			virtual const FileTreeNode& GetFileTree() const = 0;
			virtual void ClearFileTree() = 0;
			virtual void UpdateFileTree() = 0;

			virtual bool IsActive() const = 0;
			virtual void SetActive(bool value) = 0;
			virtual bool IsInstalled() const = 0;

			virtual kxf::ResourceID GetIcon() const = 0;
			virtual kxf::Color GetColor() const = 0;
			virtual void SetColor(const kxf::Color& color) = 0;

			virtual kxf::IFileSystem& GetFileSystem(Location locationID) = 0;
			virtual bool IsContentDirectoryLinked() const = 0;
			virtual void UnlinkContentDirectory() = 0;
			virtual void LinkContentDirectory(const kxf::FSPath& path) = 0;

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
