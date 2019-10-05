#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"
#include "Utility/KLabeledValue.h"
#include "Utility/KWithIDName.h"

namespace Kortex::PackageDesigner
{
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
}

namespace Kortex::PackageDesigner
{
	class KPPCFlagEntry: public KLabeledValue, public wxObject
	{
		public:
			using Vector = std::vector<KPPCFlagEntry>;
	
		public:
			static wxString GetDeletedFlagPrefix();
	
		private:
			bool HasLabel() const = delete;
			const wxString& GetLabelRaw() const = delete;
			const wxString& GetLabel() const = delete;
			void SetLabel(const wxString& label) = delete;
	
		public:
			KPPCFlagEntry(const wxString& value, const wxString& name = wxEmptyString);
			virtual ~KPPCFlagEntry();
	
		public:
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
}

namespace Kortex::PackageDesigner
{
	class KPPCCondition: public wxObject
	{
		public:
			using Vector = std::vector<KPPCCondition>;
	
		private:
			KPPCFlagEntry::Vector m_Flags;
			KPPOperator m_Operator;
	
		public:
			bool HasFlags() const
			{
				return !m_Flags.empty();
			}
			KPPCFlagEntry::Vector& GetFlags()
			{
				return m_Flags;
			}
			const KPPCFlagEntry::Vector& GetFlags() const
			{
				return m_Flags;
			}
	
			KPPOperator GetOperator() const
			{
				return m_Operator;
			}
			void SetOperator(KPPOperator value)
			{
				m_Operator = value;
			}
	};
}

namespace Kortex::PackageDesigner
{
	class KPPCConditionGroup
	{
		private:
			KPPCCondition::Vector m_Conditions;
			KPPOperator m_Operator;
	
		public:
			bool HasConditions() const
			{
				return !m_Conditions.empty();
			}
			KPPCCondition::Vector& GetConditions()
			{
				return m_Conditions;
			}
			const KPPCCondition::Vector& GetConditions() const
			{
				return m_Conditions;
			}
			KPPCCondition& GetOrCreateFirstCondition()
			{
				if (m_Conditions.empty())
				{
					return m_Conditions.emplace_back();
				}
				else
				{
					return m_Conditions.front();
				}
			}
	
			KPPOperator GetOperator() const
			{
				return m_Operator;
			}
			void SetOperator(KPPOperator value)
			{
				m_Operator = value;
			}
	};
}

namespace Kortex::PackageDesigner
{
	class KPPCEntry: public KWithName
	{
		public:
			using Vector = std::vector<std::unique_ptr<KPPCEntry>>;
			using RefVector = std::vector<KPPCEntry*>;
	
		private:
			wxString m_Image;
			wxString m_Description;
			KxStringVector m_FileData;
			KxStringVector m_Requirements;
			KPPCTypeDescriptor m_TypeDescriptorDefault;
			KPPCTypeDescriptor m_TypeDescriptorConditional = KPPC_DESCRIPTOR_INVALID;
			KPPCTypeDescriptor m_TypeDescriptorCurrent = KPPC_DESCRIPTOR_INVALID;
			KPPCConditionGroup m_TypeDescriptorConditions;
			KPPCCondition m_ConditionalFlags;
	
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
	
			KPPCConditionGroup& GetTDConditionGroup()
			{
				return m_TypeDescriptorConditions;
			}
			const KPPCConditionGroup& GetTDConditionGroup() const
			{
				return m_TypeDescriptorConditions;
			}
	
			KPPCCondition& GetConditionalFlags()
			{
				return m_ConditionalFlags;
			}
			const KPPCCondition& GetConditionalFlags() const
			{
				return m_ConditionalFlags;
			}
	};
}

namespace Kortex::PackageDesigner
{
	class KPPCGroup: public KWithName
	{
		public:
			using Vector = std::vector<std::unique_ptr<KPPCGroup>>;
	
		private:
			KPPCSelectionMode m_SelectionMode;
			KPPCEntry::Vector m_Entries;
	
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
	
			KPPCEntry::Vector& GetEntries()
			{
				return m_Entries;
			}
			const KPPCEntry::Vector& GetEntries() const
			{
				return m_Entries;
			}
	};
}

namespace Kortex::PackageDesigner
{
	class KPPCStep: public KWithName
	{
		public:
			using Vector = std::vector<std::unique_ptr<KPPCStep>>;
	
		private:
			KPPCConditionGroup m_Conditions;
			KPPCGroup::Vector m_Entries;
	
		public:
			KPPCStep();
			~KPPCStep();
	
		public:
			KPPCConditionGroup& GetConditionGroup()
			{
				return m_Conditions;
			}
			const KPPCConditionGroup& GetConditionGroup() const
			{
				return m_Conditions;
			}
	
			KPPCGroup::Vector& GetGroups()
			{
				return m_Entries;
			}
			const KPPCGroup::Vector& GetGroups() const
			{
				return m_Entries;
			}		
	};
}

namespace Kortex::PackageDesigner
{
	class KPPCConditionalStep
	{
		public:
			using Vector = std::vector<std::unique_ptr<KPPCConditionalStep>>;
	
		private:
			KPPCConditionGroup m_Conditions;
			KxStringVector m_Entries;
	
		public:
			KPPCConditionalStep();
			~KPPCConditionalStep();
	
		public:
			KPPCConditionGroup& GetConditionGroup()
			{
				return m_Conditions;
			}
			const KPPCConditionGroup& GetConditionGroup() const
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
}

namespace Kortex::PackageDesigner
{
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
			enum class FlagAttribute
			{
				Name,
				Value,
			};
	
		private:
			KxStringVector m_RequiredFileData;
			KPPCStep::Vector m_Steps;
			KPPCConditionalStep::Vector m_ConditionalSteps;
	
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
	
			KPPCStep::Vector& GetSteps()
			{
				return m_Steps;
			}
			const KPPCStep::Vector& GetSteps() const
			{
				return m_Steps;
			}
	
			KPPCConditionalStep::Vector& GetConditionalSteps()
			{
				return m_ConditionalSteps;
			}
			const KPPCConditionalStep::Vector& GetConditionalSteps() const
			{
				return m_ConditionalSteps;
			}
	
			KxStringVector GetFlagsNames() const;
			KxStringVector GetFlagsValues() const;
	};
}
