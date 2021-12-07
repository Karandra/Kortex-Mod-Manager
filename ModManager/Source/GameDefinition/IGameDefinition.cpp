#include "pch.hpp"
#include "IGameDefinition.h"
#include "IGameInstance.h"
#include "Application/IApplication.h"
#include "Application/SystemApplication.h"

#include <kxf/Utility/Enumerator.h>

namespace
{
	constexpr size_t g_MaxNameLength = 64;
}

namespace Kortex
{
	bool IGameDefinition::ValidateName(const kxf::String& name, kxf::String* validName)
	{
		if (name.IsEmpty())
		{
			return false;
		}

		// Restrict max ID length to 64 symbols
		const kxf::String forbiddenCharacters = IApplication::GetInstance().GetFileSystem(FileSystemOrigin::Unscoped).GetForbiddenPathNameCharacters();
		if (name.length() > g_MaxNameLength || name.ContainsAnyOfCharacters(forbiddenCharacters))
		{
			if (validName)
			{
				*validName = name;
				validName->Truncate(g_MaxNameLength);

				for (const auto& c: forbiddenCharacters)
				{
					validName->Replace(c, kxS('_'));
				}
			}
			return false;
		}
		return true;
	}

	kxf::Enumerator<IGameInstance&> IGameDefinition::EnumLinkedInstances() const
	{
		if (!IsNull())
		{
			return kxf::Utility::MakeForwardingEnumerator([name = GetName()](IGameInstance& instance, kxf::IEnumerator& enumerator) -> kxf::optional_ref<IGameInstance>
			{
				if (instance.GetDefinition().GetName() == name)
				{
					return instance;
				}
				else
				{
					enumerator.SkipCurrent();
					return {};
				}
			}, IApplication::GetInstance(), &IApplication::EnumGameInstances);
		}
		return {};
	}
	IGameInstance* IGameDefinition::GetLinkedInstanceByName(const kxf::String& name) const
	{
		for (IGameInstance& instance: EnumLinkedInstances())
		{
			if (instance.GetName() == name)
			{
				return &instance;
			}
		}
		return nullptr;
	}
}
