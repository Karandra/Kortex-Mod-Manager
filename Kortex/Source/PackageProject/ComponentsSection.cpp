#include "stdafx.h"
#include "ComponentsSection.h"
#include "ModPackageProject.h"
#include <Kortex/Application.hpp>
#include "Utility/KAux.h"

namespace
{
	constexpr auto KPPC_DESCRIPTOR_OPTIONAL_STRING = wxS("Optional");
	constexpr auto KPPC_DESCRIPTOR_REQUIRED_STRING = wxS("Required");
	constexpr auto KPPC_DESCRIPTOR_RECOMMENDED_STRING = wxS("Recommended");
	constexpr auto KPPC_DESCRIPTOR_COULD_BE_USABLE_STRING = wxS("CouldBeUsable");
	constexpr auto KPPC_DESCRIPTOR_NOT_USABLE_STRING = wxS("NotUsable");

	constexpr auto KPPC_SELECT_ANY_STRING = wxS("Any");
	constexpr auto KPPC_SELECT_EXACTLY_ONE_STRING = wxS("ExactlyOne");
	constexpr auto KPPC_SELECT_AT_LEAST_ONE_STRING = wxS("AtLeastOne");
	constexpr auto KPPC_SELECT_AT_MOST_ONE_STRING = wxS("AtMostOne");
	constexpr auto KPPC_SELECT_ALL_STRING = wxS("All");
}

namespace Kortex::PackageProject
{
	wxString FlagItem::GetDeletedFlagPrefix()
	{
		return wxS("DELETED_");
	}
	
	FlagItem::FlagItem(const wxString& value, const wxString& name)
		:KLabeledValue(value, name)
	{
	}
	FlagItem::~FlagItem()
	{
	}
}

namespace Kortex::PackageProject
{
	ComponentItem::ComponentItem()
		:m_TypeDescriptorDefault(ComponentsSection::ms_DefaultTypeDescriptor)
	{
	}
	ComponentItem::~ComponentItem()
	{
	}
}

namespace Kortex::PackageProject
{
	ComponentGroup::ComponentGroup()
		:m_SelectionMode(ComponentsSection::ms_DefaultSelectionMode)
	{
	}
	ComponentGroup::~ComponentGroup()
	{
	}
}

namespace Kortex::PackageProject
{
	ComponentStep::ComponentStep()
	{
	}
	ComponentStep::~ComponentStep()
	{
	}
}

namespace Kortex::PackageProject
{
	ConditionalComponentStep::ConditionalComponentStep()
	{
	}
	ConditionalComponentStep::~ConditionalComponentStep()
	{
	}
}

namespace Kortex::PackageProject
{
	TypeDescriptor ComponentsSection::StringToTypeDescriptor(const wxString& name, TypeDescriptor default)
	{
		if (name == KPPC_DESCRIPTOR_OPTIONAL_STRING)
		{
			return TypeDescriptor::Optional;
		}
		if (name == KPPC_DESCRIPTOR_REQUIRED_STRING)
		{
			return TypeDescriptor::Required;
		}
		if (name == KPPC_DESCRIPTOR_RECOMMENDED_STRING)
		{
			return TypeDescriptor::Recommended;
		}
		if (name == KPPC_DESCRIPTOR_COULD_BE_USABLE_STRING)
		{
			return TypeDescriptor::CouldBeUsable;
		}
		if (name == KPPC_DESCRIPTOR_NOT_USABLE_STRING)
		{
			return TypeDescriptor::NotUsable;
		}
		return default;
	}
	wxString ComponentsSection::TypeDescriptorToString(TypeDescriptor type)
	{
		switch (type)
		{
			case TypeDescriptor::Optional:
			{
				return KPPC_DESCRIPTOR_OPTIONAL_STRING;
			}
			case TypeDescriptor::Required:
			{
				return KPPC_DESCRIPTOR_REQUIRED_STRING;
			}
			case TypeDescriptor::Recommended:
			{
				return KPPC_DESCRIPTOR_RECOMMENDED_STRING;
			}
			case TypeDescriptor::CouldBeUsable:
			{
				return KPPC_DESCRIPTOR_COULD_BE_USABLE_STRING;
			}
			case TypeDescriptor::NotUsable:
			{
				return KPPC_DESCRIPTOR_NOT_USABLE_STRING;
			}
		};
		return wxEmptyString;
	}
	wxString ComponentsSection::TypeDescriptorToTranslation(TypeDescriptor type)
	{
		return KTr(KxString::Format("PackageCreator.PageComponents.TypeDescriptor.%1", ComponentsSection::TypeDescriptorToString(type)));
	}
	
	SelectionMode ComponentsSection::StringToSelectionMode(const wxString& name)
	{
		if (name == KPPC_SELECT_ANY_STRING)
		{
			return SelectionMode::Any;
		}
		if (name == KPPC_SELECT_ALL_STRING)
		{
			return SelectionMode::All;
		}
		if (name == KPPC_SELECT_EXACTLY_ONE_STRING)
		{
			return SelectionMode::ExactlyOne;
		}
		if (name == KPPC_SELECT_AT_LEAST_ONE_STRING)
		{
			return SelectionMode::AtLeastOne;
		}
		if (name == KPPC_SELECT_AT_MOST_ONE_STRING)
		{
			return SelectionMode::AtMostOne;
		}
		return ms_DefaultSelectionMode;
	}
	wxString ComponentsSection::SelectionModeToString(SelectionMode type)
	{
		switch (type)
		{
			case SelectionMode::Any:
			{
				return KPPC_SELECT_ANY_STRING;
			}
			case SelectionMode::All:
			{
				return KPPC_SELECT_ALL_STRING;
			}
			case SelectionMode::ExactlyOne:
			{
				return KPPC_SELECT_EXACTLY_ONE_STRING;
			}
			case SelectionMode::AtLeastOne:
			{
				return KPPC_SELECT_AT_LEAST_ONE_STRING;
			}
			case SelectionMode::AtMostOne:
			{
				return KPPC_SELECT_AT_MOST_ONE_STRING;
			}
		};
		return wxEmptyString;
	}
	wxString ComponentsSection::SelectionModeToTranslation(SelectionMode type)
	{
		return KTr(KxString::Format("PackageCreator.PageComponents.SelectionMode.%1", SelectionModeToString(type)));
	}
	
	KxStringVector ComponentsSection::GetFlagsAttributes(FlagAttribute index) const
	{
		KxStringVector outList;
	
		// Add flags for requirements sets
		if (index == FlagAttribute::Name)
		{
			for (const auto& group: GetProject().GetRequirements().GetGroups())
			{
				outList.push_back(group->GetFlagName());
			}
		}
		
		// Some standard values
		if (index == FlagAttribute::Value)
		{
			outList.push_back("true");
			outList.push_back("false");
		}
	
		// All others
		for (const auto& step: m_Steps)
		{
			for (const auto& group: step->GetGroups())
			{
				for (const auto& entry: group->GetEntries())
				{
					for (const FlagItem& flagEntry: entry->GetConditionalFlags().GetFlags())
					{
						outList.push_back(index == FlagAttribute::Name ? flagEntry.GetName() : flagEntry.GetValue());
					}
				}
			}
		}
	
		// Remove duplicates
		outList.erase(std::unique(outList.begin(), outList.end()), outList.end());
		return outList;
	}
	
	ComponentsSection::ComponentsSection(ModPackageProject& project)
		:ProjectSection(project)
	{
	}
	ComponentsSection::~ComponentsSection()
	{
	}
	
	KxStringVector ComponentsSection::GetFlagsNames() const
	{
		return GetFlagsAttributes(FlagAttribute::Name);
	}
	KxStringVector ComponentsSection::GetFlagsValues() const
	{
		return GetFlagsAttributes(FlagAttribute::Value);
	}
}
