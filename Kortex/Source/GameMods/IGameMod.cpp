#include "stdafx.h"
#include "IGameMod.h"
#include "IModManager.h"
#include <Kortex/GameInstance.hpp>
#include "Utility/Common.h"
#include <KxFramework/KxFile.h>
#include <KxFramework/KxCrypto.h>

namespace
{
	wxString GetRootPath(const wxString& signature)
	{
		using namespace Kortex;

		if (const IGameInstance* instance = IGameInstance::GetActive())
		{
			return instance->GetModsDir() + wxS('\\') + signature;
		}
		return wxEmptyString;
	}
	wxString GetRootRelativePath(const wxString& signature, const wxString& name)
	{
		return GetRootPath(signature) + wxS('\\') + name;
	}
	wxString GetRootRelativePath(const wxString& signature, const wxChar* name)
	{
		return GetRootPath(signature) + wxS('\\') + name;
	}
}

namespace Kortex
{
	wxString IGameMod::GetSignatureFromID(const wxString& id)
	{
		auto utf8 = id.ToUTF8();
		wxMemoryInputStream stream(utf8.data(), utf8.length());
		return KxCrypto::MD5(stream);
	}
	
	bool IGameMod::LoadUsingID(const wxString& id)
	{
		SetID(id);
		return LoadUsingSignature(GetSignature());
	}
	bool IGameMod::CreateFromProject(const ModPackageProject& config)
	{
		return false;
	}
	
	wxString IGameMod::GetSafeName() const
	{
		return Utility::MakeSafeFileName(GetName());
	}
	bool IGameMod::IsPackageFileExist() const
	{
		return KxFile(GetPackageFile()).IsFileExist();
	}

	wxString IGameMod::GetRootDir() const
	{
		return GetRootPath(GetSignature());
	}
	wxString IGameMod::GetDescriptionFile() const
	{
		return GetRootRelativePath(GetSignature(), wxS("Description.txt"));
	}
	wxString IGameMod::GetInfoFile() const
	{
		return GetRootRelativePath(GetSignature(), wxS("Info.xml"));
	}
	wxString IGameMod::GetImageFile() const
	{
		return GetRootRelativePath(GetSignature(), wxS("Image.img"));
	}
	wxString IGameMod::GetDefaultModFilesDir() const
	{
		return GetRootRelativePath(GetSignature(), wxS("ModFiles"));
	}
	wxString IGameMod::GetModFilesDir() const
	{
		return GetDefaultModFilesDir();
	}
}
