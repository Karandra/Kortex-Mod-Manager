#include "stdafx.h"
#include "KPackageProjectComponents.h"
#include "KPackageProject.h"
#include "KApp.h"
#include "KAux.h"

KPPCFlagEntry::KPPCFlagEntry(const wxString& value, const wxString& name, KPPOperator operatorType)
	:KLabeledValue(value, name), m_Operator(operatorType)
{
}
KPPCFlagEntry::~KPPCFlagEntry()
{
}

//////////////////////////////////////////////////////////////////////////
KPPCEntry::KPPCEntry()
	:m_TypeDescriptorDefault(KPackageProjectComponents::ms_DefaultTypeDescriptor)
{
}
KPPCEntry::~KPPCEntry()
{
}

//////////////////////////////////////////////////////////////////////////
KPPCGroup::KPPCGroup()
	:m_SelectionMode(KPackageProjectComponents::ms_DefaultSelectionMode)
{
}
KPPCGroup::~KPPCGroup()
{
}

//////////////////////////////////////////////////////////////////////////
KPPCStep::KPPCStep()
{
}
KPPCStep::~KPPCStep()
{
}

//////////////////////////////////////////////////////////////////////////
KPPCConditionalStep::KPPCConditionalStep()
{
}
KPPCConditionalStep::~KPPCConditionalStep()
{
}

//////////////////////////////////////////////////////////////////////////
#define KPPC_DESCRIPTOR_OPTIONAL_STRING					"Optional"
#define KPPC_DESCRIPTOR_REQUIRED_STRING					"Required"
#define KPPC_DESCRIPTOR_RECOMMENDED_STRING				"Recommended"
#define KPPC_DESCRIPTOR_COULD_BE_USABLE_STRING			"CouldBeUsable"
#define KPPC_DESCRIPTOR_NOT_USABLE_STRING				"NotUsable"

#define KPPC_SELECT_ANY_STRING							"Any"
#define KPPC_SELECT_EXACTLY_ONE_STRING					"ExactlyOne"
#define KPPC_SELECT_AT_LEAST_ONE_STRING					"AtLeastOne"
#define KPPC_SELECT_AT_MOST_ONE_STRING					"AtMostOne"
#define KPPC_SELECT_ALL_STRING							"All"

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
	return T(wxString::Format("PackageCreator.PageComponents.TypeDescriptor.%s", KPackageProjectComponents::TypeDescriptorToString(type)));
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
	return T(wxString::Format("PackageCreator.PageComponents.SelectionMode.%s", SelectionModeToString(type)));
}

KxStringVector KPackageProjectComponents::GetFlagsAttributes(FlagAttribute index) const
{
	KxStringVector outList;

	// Add flags for requirements sets
	if (index == Name)
	{
		for (const auto& pSet: GetProject().GetRequirements().GetGroups())
		{
			outList.push_back(pSet->GetFlagName());
		}
	}
	
	// Some std values
	if (index == Value)
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
				for (const KPPCFlagEntry& flagEntry: entry->GetAssignedFlags())
				{
					outList.push_back(index == Name ? flagEntry.GetName() : flagEntry.GetValue());
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
	return GetFlagsAttributes(Name);
}
KxStringVector KPackageProjectComponents::GetFlagsValues() const
{
	return GetFlagsAttributes(Value);
}
