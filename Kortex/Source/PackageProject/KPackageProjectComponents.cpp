#include "stdafx.h"
#include "KPackageProjectComponents.h"
#include "KPackageProject.h"
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

namespace Kortex::PackageDesigner
{
	wxString KPPCFlagEntry::GetDeletedFlagPrefix()
	{
		return wxS("DELETED_");
	}
	
	KPPCFlagEntry::KPPCFlagEntry(const wxString& value, const wxString& name)
		:KLabeledValue(value, name)
	{
	}
	KPPCFlagEntry::~KPPCFlagEntry()
	{
	}
}

namespace Kortex::PackageDesigner
{
	KPPCEntry::KPPCEntry()
		:m_TypeDescriptorDefault(KPackageProjectComponents::ms_DefaultTypeDescriptor)
	{
	}
	KPPCEntry::~KPPCEntry()
	{
	}
}

namespace Kortex::PackageDesigner
{
	KPPCGroup::KPPCGroup()
		:m_SelectionMode(KPackageProjectComponents::ms_DefaultSelectionMode)
	{
	}
	KPPCGroup::~KPPCGroup()
	{
	}
}

namespace Kortex::PackageDesigner
{
	KPPCStep::KPPCStep()
	{
	}
	KPPCStep::~KPPCStep()
	{
	}
}

namespace Kortex::PackageDesigner
{
	KPPCConditionalStep::KPPCConditionalStep()
	{
	}
	KPPCConditionalStep::~KPPCConditionalStep()
	{
	}
}

namespace Kortex::PackageDesigner
{
	KPPCTypeDescriptor KPackageProjectComponents::StringToTypeDescriptor(const wxString& name, KPPCTypeDescriptor default)
	{
		if (name == KPPC_DESCRIPTOR_OPTIONAL_STRING)
		{
			return KPPC_DESCRIPTOR_OPTIONAL;
		}
		if (name == KPPC_DESCRIPTOR_REQUIRED_STRING)
		{
			return KPPC_DESCRIPTOR_REQUIRED;
		}
		if (name == KPPC_DESCRIPTOR_RECOMMENDED_STRING)
		{
			return KPPC_DESCRIPTOR_RECOMMENDED;
		}
		if (name == KPPC_DESCRIPTOR_COULD_BE_USABLE_STRING)
		{
			return KPPC_DESCRIPTOR_COULD_BE_USABLE;
		}
		if (name == KPPC_DESCRIPTOR_NOT_USABLE_STRING)
		{
			return KPPC_DESCRIPTOR_NOT_USABLE;
		}
		return default;
	}
	wxString KPackageProjectComponents::TypeDescriptorToString(KPPCTypeDescriptor type)
	{
		switch (type)
		{
			case KPPC_DESCRIPTOR_OPTIONAL:
			{
				return KPPC_DESCRIPTOR_OPTIONAL_STRING;
			}
			case KPPC_DESCRIPTOR_REQUIRED:
			{
				return KPPC_DESCRIPTOR_REQUIRED_STRING;
			}
			case KPPC_DESCRIPTOR_RECOMMENDED:
			{
				return KPPC_DESCRIPTOR_RECOMMENDED_STRING;
			}
			case KPPC_DESCRIPTOR_COULD_BE_USABLE:
			{
				return KPPC_DESCRIPTOR_COULD_BE_USABLE_STRING;
			}
			case KPPC_DESCRIPTOR_NOT_USABLE:
			{
				return KPPC_DESCRIPTOR_NOT_USABLE_STRING;
			}
		};
		return wxEmptyString;
	}
	wxString KPackageProjectComponents::TypeDescriptorToTranslation(KPPCTypeDescriptor type)
	{
		return KTr(KxString::Format("PackageCreator.PageComponents.TypeDescriptor.%1", KPackageProjectComponents::TypeDescriptorToString(type)));
	}
	
	KPPCSelectionMode KPackageProjectComponents::StringToSelectionMode(const wxString& name)
	{
		if (name == KPPC_SELECT_ANY_STRING)
		{
			return KPPC_SELECT_ANY;
		}
		if (name == KPPC_SELECT_ALL_STRING)
		{
			return KPPC_SELECT_ALL;
		}
		if (name == KPPC_SELECT_EXACTLY_ONE_STRING)
		{
			return KPPC_SELECT_EXACTLY_ONE;
		}
		if (name == KPPC_SELECT_AT_LEAST_ONE_STRING)
		{
			return KPPC_SELECT_AT_LEAST_ONE;
		}
		if (name == KPPC_SELECT_AT_MOST_ONE_STRING)
		{
			return KPPC_SELECT_AT_MOST_ONE;
		}
		return ms_DefaultSelectionMode;
	}
	wxString KPackageProjectComponents::SelectionModeToString(KPPCSelectionMode type)
	{
		switch (type)
		{
			case KPPC_SELECT_ANY:
			{
				return KPPC_SELECT_ANY_STRING;
			}
			case KPPC_SELECT_ALL:
			{
				return KPPC_SELECT_ALL_STRING;
			}
			case KPPC_SELECT_EXACTLY_ONE:
			{
				return KPPC_SELECT_EXACTLY_ONE_STRING;
			}
			case KPPC_SELECT_AT_LEAST_ONE:
			{
				return KPPC_SELECT_AT_LEAST_ONE_STRING;
			}
			case KPPC_SELECT_AT_MOST_ONE:
			{
				return KPPC_SELECT_AT_MOST_ONE_STRING;
			}
		};
		return wxEmptyString;
	}
	wxString KPackageProjectComponents::SelectionModeToTranslation(KPPCSelectionMode type)
	{
		return KTr(KxString::Format("PackageCreator.PageComponents.SelectionMode.%1", SelectionModeToString(type)));
	}
	
	KxStringVector KPackageProjectComponents::GetFlagsAttributes(FlagAttribute index) const
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
					for (const KPPCFlagEntry& flagEntry: entry->GetConditionalFlags().GetFlags())
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
	
	KPackageProjectComponents::KPackageProjectComponents(KPackageProject& project)
		:KPackageProjectPart(project)
	{
	}
	KPackageProjectComponents::~KPackageProjectComponents()
	{
	}
	
	KxStringVector KPackageProjectComponents::GetFlagsNames() const
	{
		return GetFlagsAttributes(FlagAttribute::Name);
	}
	KxStringVector KPackageProjectComponents::GetFlagsValues() const
	{
		return GetFlagsAttributes(FlagAttribute::Value);
	}
}
