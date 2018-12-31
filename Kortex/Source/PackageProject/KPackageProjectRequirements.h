#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"
#include "Utility/KWithIDName.h"
#include <KxFramework/KxVersion.h>

enum KPPRObjectFunction
{
	KPPR_OBJFUNC_INVALID = -1,
	KPPR_OBJFUNC_NONE = 0,

	KPPR_OBJFUNC_MOD_ACTIVE,
	KPPR_OBJFUNC_MOD_INACTIVE,
	KPPR_OBJFUNC_FILE_EXIST,
	KPPR_OBJFUNC_FILE_NOT_EXIST,
	KPPR_OBJFUNC_PLUGIN_ACTIVE,
	KPPR_OBJFUNC_PLUGIN_INACTIVE,
};
enum KPPRTypeDescriptor
{
	KPPR_TYPE_USER = 0,
	KPPR_TYPE_SYSTEM = 1,
	KPPR_TYPE_AUTO = 2,
};

//////////////////////////////////////////////////////////////////////////
class KPPRRequirementEntry: public KWithIDName
{
	public:
		using Vector = std::vector<std::unique_ptr<KPPRRequirementEntry>>;
		using RefVector = std::vector<KPPRRequirementEntry*>;

	private:
		wxString m_Object;
		KxVersion m_RequiredVersion;

		mutable bool m_CurrentVersionChecked = false;
		mutable KxVersion m_CurrentVersion;
		
		KPPOperator m_RequiredVersionFunction;
		KPPRObjectFunction m_ObjectFunction;

		mutable bool m_ObjectFunctionResultChecked = false;
		mutable KPPReqState m_ObjectFunctionResult = KPPReqState::Unknown;

		wxString m_Description;
		wxString m_BinaryVersionKind = "FileVersion";
		
		bool m_OverallStatusCalculated = false;
		bool m_OverallStatus = false;

		KPPRTypeDescriptor m_TypeDescriptor = KPPR_TYPE_AUTO;
		wxString m_Category;
		KxStringVector m_Dependencies;

	public:
		KPPRRequirementEntry(KPPRTypeDescriptor typeDescriptor = KPPR_TYPE_AUTO);
		~KPPRRequirementEntry();

	public:
		const wxString& GetObject() const
		{
			return m_Object;
		}
		void SetObject(const wxString& value)
		{
			m_Object = value;
		}
		
		const KxVersion& GetRequiredVersion() const
		{
			return m_RequiredVersion;
		}
		KPPOperator GetRVFunction() const
		{
			return m_RequiredVersionFunction;
		}
		void SetRequiredVersion(const wxString& value)
		{
			m_RequiredVersion = value;
		}
		void SetRVFunction(KPPOperator operatorType)
		{
			m_RequiredVersionFunction = operatorType;
		}
		
		const KxVersion& GetCurrentVersion() const;
		void SetCurrentVersion(const wxString& value)
		{
			m_CurrentVersion = value;
		}
		void ResetCurrentVersion();
		bool CheckVersion() const;

		KPPRObjectFunction GetObjectFunction() const
		{
			return m_ObjectFunction;
		}
		void SetObjectFunction(KPPRObjectFunction state)
		{
			m_ObjectFunction = state;
		}
		KPPReqState GetObjectFunctionResult() const;
		void ResetObjectFunctionResult();
		
		const wxString& GetDescription() const
		{
			return m_Description;
		}
		void SetDescription(const wxString& value)
		{
			m_Description = value;
		}

		const wxString& GetBinaryVersionKind() const
		{
			return m_BinaryVersionKind;
		}
		void SetBinaryVersionKind(const wxString& value)
		{
			m_BinaryVersionKind = value;
		}

		bool IsStd() const;
		bool IsSystem() const;
		bool IsUserEditable() const;

		KPPRTypeDescriptor GetTypeDescriptor() const
		{
			return m_TypeDescriptor;
		}
		void SetTypeDescriptorUnchecked(KPPRTypeDescriptor type)
		{
			m_TypeDescriptor = type;
		}
		void TrySetTypeDescriptor(KPPRTypeDescriptor type);
		bool ConformToTypeDescriptor();

		const wxString& GetCategory() const
		{
			return m_Category;
		}
		void SetCategory(const wxString& value)
		{
			m_Category = value;
		}
		
		KxStringVector& GetDependencies()
		{
			return m_Dependencies;
		}
		const KxStringVector& GetDependencies() const
		{
			return m_Dependencies;
		}

		bool GetOverallStatus() const
		{
			return m_OverallStatus;
		}
		bool CalcOverallStatus();
};

//////////////////////////////////////////////////////////////////////////
class KPPRRequirementsGroup
{
	public:
		using Vector = std::vector<std::unique_ptr<KPPRRequirementsGroup>>;
		using RefVector = std::vector<KPPRRequirementsGroup*>;

	public:
		static wxString GetFlagNamePrefix();
		static wxString GetFlagName(const wxString& id);

	private:
		wxString m_ID;
		KPPOperator m_Operator;
		KPPRRequirementEntry::Vector m_Entries;

		bool m_GroupStatus = false;
		bool m_GroupStatusCalculated = false;

	public:
		KPPRRequirementsGroup();
		~KPPRRequirementsGroup();

	public:
		const wxString& GetID() const
		{
			return m_ID;
		}
		void SetID(const wxString& id)
		{
			m_ID = id;
		}
		wxString GetFlagName() const
		{
			return GetFlagName(GetID());
		}

		KPPOperator GetOperator() const
		{
			return m_Operator;
		}
		void SetOperator(KPPOperator value)
		{
			m_Operator = value;
		}

		KPPRRequirementEntry::Vector& GetEntries()
		{
			return m_Entries;
		}
		const KPPRRequirementEntry::Vector& GetEntries() const
		{
			return m_Entries;
		}

		KPPRRequirementEntry* FindEntry(const wxString& id) const;
		bool HasEntryWithID(const wxString& id) const
		{
			return FindEntry(id) != nullptr;
		}

		bool CalcGroupStatus();
		bool GetGroupStatus() const
		{
			return m_GroupStatus;
		}
};

//////////////////////////////////////////////////////////////////////////
class KPackageProjectRequirements: public KPackageProjectPart
{
	public:
		static const KPPOperator ms_DefaultGroupOperator = KPP_OPERATOR_AND;
		static const KPPOperator ms_DefaultVersionOperator = KPP_OPERATOR_GTEQ;
		static const KPPRObjectFunction ms_DefaultObjectFunction = KPPR_OBJFUNC_NONE;
		static const KPPRTypeDescriptor ms_DefaultTypeDescriptor = KPPR_TYPE_AUTO;

	public:
		static KPPRObjectFunction StringToObjectFunction(const wxString& name);
		static wxString ObjectFunctionToString(KPPRObjectFunction state);

		static KPPRTypeDescriptor StringToTypeDescriptor(const wxString& name);
		static wxString TypeDescriptorToString(KPPRTypeDescriptor type);

		static bool CompareVersions(KPPOperator operatorType, const KxVersion& current, const KxVersion& required);

	private:
		KPPRRequirementsGroup::Vector m_Groups;
		KxStringVector m_DefaultGroup;

	public:
		KPackageProjectRequirements(KPackageProject& project);
		virtual ~KPackageProjectRequirements();

	public:
		KPPRRequirementsGroup::Vector& GetGroups()
		{
			return m_Groups;
		}
		const KPPRRequirementsGroup::Vector& GetGroups() const
		{
			return m_Groups;
		}

		bool IsDefaultGroupEmpty() const
		{
			return m_DefaultGroup.empty();
		}
		const KxStringVector& GetDefaultGroup() const
		{
			return m_DefaultGroup;
		}
		KxStringVector& GetDefaultGroup()
		{
			return m_DefaultGroup;
		}
		bool IsDefaultGroupContains(const wxString& groupID) const;

		KPPRRequirementsGroup* FindGroupWithID(const wxString& id) const;
		bool HasSetWithID(const wxString& id) const
		{
			return FindGroupWithID(id) != nullptr;
		}

		KxStringVector GetFlagNames() const;
		bool CalcOverallStatus(const KxStringVector& groups) const;
};
