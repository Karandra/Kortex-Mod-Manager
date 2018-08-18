#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"
#include "KLabeledValue.h"
#include "KWithIDName.h"

enum KPPCSelectionMode
{
	KPPC_SELECT_ANY,
	KPPC_SELECT_EXACTLY_ONE,
	KPPC_SELECT_AT_LEAST_ONE, /* One or more */
	KPPC_SELECT_AT_MOST_ONE, /* One or nothing */
	KPPC_SELECT_ALL, /* All entries must be selected (pretty useless) */
};
enum KPPCTypeDescriptor
{
	KPPC_DESCRIPTOR_INVALID = -1,

	KPPC_DESCRIPTOR_OPTIONAL,
	KPPC_DESCRIPTOR_REQUIRED,
	KPPC_DESCRIPTOR_RECOMMENDED,
	KPPC_DESCRIPTOR_COULD_BE_USABLE,
	KPPC_DESCRIPTOR_NOT_USABLE,
};

//////////////////////////////////////////////////////////////////////////
class KPPCFlagEntry: public KLabeledValue
{
	public:
		static wxString GetDeletedFlagPrefix()
		{
			return "DELETED_";
		}

	private:
		KPPOperator m_Operator;

	private:
		bool HasLabel() const = delete;
		const wxString& GetLabelRaw() const = delete;
		const wxString& GetLabel() const = delete;
		void SetLabel(const wxString& label) = delete;

	public:
		KPPCFlagEntry(const wxString& value, const wxString& name = wxEmptyString, KPPOperator operatorType = KPP_OPERATOR_AND);
		virtual ~KPPCFlagEntry();

	public:
		KPPOperator GetOperator() const
		{
			return m_Operator;
		}
		void SetOperator(KPPOperator operatorType)
		{
			m_Operator = operatorType;
		}

		const wxString& GetValue() const
		{
			return KLabeledValue::GetValue();
		}
		void SetValue(const wxString& value)
		{
			KLabeledValue::SetValue(value);
		}

		const wxString& GetName() const
		{
			return KLabeledValue::GetLabelRaw();
		}
		wxString GetDeletedName() const
		{
			return GetDeletedFlagPrefix() + GetName();
		}
		void SetName(const wxString& value)
		{
			KLabeledValue::SetLabel(value);
		}
};
typedef std::vector<KPPCFlagEntry> KPPCFlagEntryArray;

//////////////////////////////////////////////////////////////////////////
class KPPCEntry: public KWithName
{
	private:
		wxString m_Image;
		wxString m_Description;
		KxStringVector m_FileData;
		KxStringVector m_Requirements;
		KPPCTypeDescriptor m_TypeDescriptorDefault;
		KPPCTypeDescriptor m_TypeDescriptorConditional = KPPC_DESCRIPTOR_INVALID;
		KPPCTypeDescriptor m_TypeDescriptorCurrent = KPPC_DESCRIPTOR_INVALID;
		KPPCFlagEntryArray m_TypeDescriptorConditions;
		KPPCFlagEntryArray m_AssignedFlags;

	public:
		KPPCEntry();
		~KPPCEntry();

	public:
		const wxString& GetImage() const
		{
			return m_Image;
		}
		void SetImage(const wxString& value)
		{
			m_Image = value;
		}

		const wxString& GetDescription() const
		{
			return m_Description;
		}
		void SetDescription(const wxString& value)
		{
			m_Description = value;
		}

		KxStringVector& GetFileData()
		{
			return m_FileData;
		}
		const KxStringVector& GetFileData() const
		{
			return m_FileData;
		}

		KxStringVector& GetRequirements()
		{
			return m_Requirements;
		}
		const KxStringVector& GetRequirements() const
		{
			return m_Requirements;
		}

		KPPCTypeDescriptor GetTDDefaultValue() const
		{
			return m_TypeDescriptorDefault;
		}
		void SetTDDefaultValue(KPPCTypeDescriptor type)
		{
			m_TypeDescriptorDefault = type;
		}
		
		KPPCTypeDescriptor GetTDConditionalValue() const
		{
			return m_TypeDescriptorConditional;
		}
		void SetTDConditionalValue(KPPCTypeDescriptor type)
		{
			m_TypeDescriptorConditional = type;
		}
		
		KPPCTypeDescriptor GetTDCurrentValue() const
		{
			return m_TypeDescriptorCurrent != KPPC_DESCRIPTOR_INVALID ? m_TypeDescriptorCurrent : GetTDDefaultValue();
		}
		void SetTDCurrentValue(KPPCTypeDescriptor type)
		{
			m_TypeDescriptorCurrent = type;
		}

		KPPCFlagEntryArray& GetTDConditions()
		{
			return m_TypeDescriptorConditions;
		}
		const KPPCFlagEntryArray& GetTDConditions() const
		{
			return m_TypeDescriptorConditions;
		}

		KPPCFlagEntryArray& GetAssignedFlags()
		{
			return m_AssignedFlags;
		}
		const KPPCFlagEntryArray& GetAssignedFlags() const
		{
			return m_AssignedFlags;
		}
};
typedef std::vector<std::unique_ptr<KPPCEntry>> KPPCEntryArray;
typedef std::vector<KPPCEntry*> KPPCEntryRefArray;

//////////////////////////////////////////////////////////////////////////
class KPPCGroup: public KWithName
{
	private:
		KPPCSelectionMode m_SelectionMode;
		KPPCEntryArray m_Entries;

	public:
		KPPCGroup();
		~KPPCGroup();

	public:
		KPPCSelectionMode GetSelectionMode() const
		{
			return m_SelectionMode;
		}
		void SetSelectionMode(KPPCSelectionMode type)
		{
			m_SelectionMode = type;
		}

		KPPCEntryArray& GetEntries()
		{
			return m_Entries;
		}
		const KPPCEntryArray& GetEntries() const
		{
			return m_Entries;
		}
};
typedef std::vector<std::unique_ptr<KPPCGroup>> KPPCGroupArray;

//////////////////////////////////////////////////////////////////////////
class KPPCStep: public KWithName
{
	private:
		KPPCFlagEntryArray m_Conditions;
		KPPCGroupArray m_Entries;

	public:
		KPPCStep();
		~KPPCStep();

	public:
		KPPCFlagEntryArray& GetConditions()
		{
			return m_Conditions;
		}
		const KPPCFlagEntryArray& GetConditions() const
		{
			return m_Conditions;
		}

		KPPCGroupArray& GetGroups()
		{
			return m_Entries;
		}
		const KPPCGroupArray& GetGroups() const
		{
			return m_Entries;
		}		
};
typedef std::vector<std::unique_ptr<KPPCStep>> KPPCStepArray;

//////////////////////////////////////////////////////////////////////////
class KPPCConditionalStep
{
	private:
		KPPCFlagEntryArray m_Conditions;
		KxStringVector m_Entries;

	public:
		KPPCConditionalStep();
		~KPPCConditionalStep();

	public:
		KPPCFlagEntryArray& GetConditions()
		{
			return m_Conditions;
		}
		const KPPCFlagEntryArray& GetConditions() const
		{
			return m_Conditions;
		}

		KxStringVector& GetEntries()
		{
			return m_Entries;
		}
		const KxStringVector& GetEntries() const
		{
			return m_Entries;
		}
};
typedef std::vector<std::unique_ptr<KPPCConditionalStep>> KPPCConditionalStepArray;

//////////////////////////////////////////////////////////////////////////
class KPackageProjectComponents: public KPackageProjectPart
{
	public:
		static const KPPOperator ms_DefaultFlagsOperator = KPP_OPERATOR_AND;
		static const KPPCSelectionMode ms_DefaultSelectionMode = KPPC_SELECT_ANY;
		static const KPPCTypeDescriptor ms_DefaultTypeDescriptor = KPPC_DESCRIPTOR_OPTIONAL;

	public:
		static KPPCTypeDescriptor StringToTypeDescriptor(const wxString& name, KPPCTypeDescriptor default = ms_DefaultTypeDescriptor);
		static wxString TypeDescriptorToString(KPPCTypeDescriptor type);
		static wxString TypeDescriptorToTranslation(KPPCTypeDescriptor type);

		static KPPCSelectionMode StringToSelectionMode(const wxString& name);
		static wxString SelectionModeToString(KPPCSelectionMode type);
		static wxString SelectionModeToTranslation(KPPCSelectionMode type);

	private:
		enum FlagAttribute
		{
			Name,
			Value,
		};

	private:
		KxStringVector m_RequiredFileData;
		KPPCStepArray m_Steps;
		KPPCConditionalStepArray m_ConditionalSteps;

	private:
		KxStringVector GetFlagsAttributes(FlagAttribute index) const;

	public:
		KPackageProjectComponents(KPackageProject& project);
		virtual ~KPackageProjectComponents();

	public:
		KxStringVector& GetRequiredFileData()
		{
			return m_RequiredFileData;
		}
		const KxStringVector& GetRequiredFileData() const
		{
			return m_RequiredFileData;
		}

		KPPCStepArray& GetSteps()
		{
			return m_Steps;
		}
		const KPPCStepArray& GetSteps() const
		{
			return m_Steps;
		}

		KPPCConditionalStepArray& GetConditionalSteps()
		{
			return m_ConditionalSteps;
		}
		const KPPCConditionalStepArray& GetConditionalSteps() const
		{
			return m_ConditionalSteps;
		}

		KxStringVector GetFlagsNames() const;
		KxStringVector GetFlagsValues() const;
};
