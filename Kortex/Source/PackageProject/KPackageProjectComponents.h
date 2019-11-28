#pragma once
#include "stdafx.h"
#include "KPackageProjectPart.h"
#include "Utility/KLabeledValue.h"
#include "Utility/KWithIDName.h"

namespace Kortex::PackageProject
{
	class FlagItem: public KLabeledValue, public wxObject
	{
		public:
			using Vector = std::vector<FlagItem>;
	
		public:
			static wxString GetDeletedFlagPrefix();
	
		private:
			bool HasLabel() const = delete;
			const wxString& GetLabelRaw() const = delete;
			const wxString& GetLabel() const = delete;
			void SetLabel(const wxString& label) = delete;
	
		public:
			FlagItem(const wxString& value, const wxString& name = wxEmptyString);
			virtual ~FlagItem();
	
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

namespace Kortex::PackageProject
{
	class Condition: public wxObject
	{
		public:
			using Vector = std::vector<Condition>;
	
		private:
			FlagItem::Vector m_Flags;
			Operator m_Operator;
	
		public:
			bool HasFlags() const
			{
				return !m_Flags.empty();
			}
			FlagItem::Vector& GetFlags()
			{
				return m_Flags;
			}
			const FlagItem::Vector& GetFlags() const
			{
				return m_Flags;
			}
	
			Operator GetOperator() const
			{
				return m_Operator;
			}
			void SetOperator(Operator value)
			{
				m_Operator = value;
			}
	};
}

namespace Kortex::PackageProject
{
	class ConditionGroup
	{
		private:
			Condition::Vector m_Conditions;
			Operator m_Operator;
	
		public:
			bool HasConditions() const
			{
				return !m_Conditions.empty();
			}
			Condition::Vector& GetConditions()
			{
				return m_Conditions;
			}
			const Condition::Vector& GetConditions() const
			{
				return m_Conditions;
			}
			Condition& GetOrCreateFirstCondition()
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
	
			Operator GetOperator() const
			{
				return m_Operator;
			}
			void SetOperator(Operator value)
			{
				m_Operator = value;
			}
	};
}

namespace Kortex::PackageProject
{
	class ComponentItem: public KWithName
	{
		public:
			using Vector = std::vector<std::unique_ptr<ComponentItem>>;
			using RefVector = std::vector<ComponentItem*>;
	
		private:
			wxString m_Image;
			wxString m_Description;
			KxStringVector m_FileData;
			KxStringVector m_Requirements;
			TypeDescriptor m_TypeDescriptorDefault;
			TypeDescriptor m_TypeDescriptorConditional = KPPC_DESCRIPTOR_INVALID;
			TypeDescriptor m_TypeDescriptorCurrent = KPPC_DESCRIPTOR_INVALID;
			ConditionGroup m_TypeDescriptorConditions;
			Condition m_ConditionalFlags;
	
		public:
			ComponentItem();
			~ComponentItem();
	
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
	
			TypeDescriptor GetTDDefaultValue() const
			{
				return m_TypeDescriptorDefault;
			}
			void SetTDDefaultValue(TypeDescriptor type)
			{
				m_TypeDescriptorDefault = type;
			}
			
			TypeDescriptor GetTDConditionalValue() const
			{
				return m_TypeDescriptorConditional;
			}
			void SetTDConditionalValue(TypeDescriptor type)
			{
				m_TypeDescriptorConditional = type;
			}
			
			TypeDescriptor GetTDCurrentValue() const
			{
				return m_TypeDescriptorCurrent != KPPC_DESCRIPTOR_INVALID ? m_TypeDescriptorCurrent : GetTDDefaultValue();
			}
			void SetTDCurrentValue(TypeDescriptor type)
			{
				m_TypeDescriptorCurrent = type;
			}
	
			ConditionGroup& GetTDConditionGroup()
			{
				return m_TypeDescriptorConditions;
			}
			const ConditionGroup& GetTDConditionGroup() const
			{
				return m_TypeDescriptorConditions;
			}
	
			Condition& GetConditionalFlags()
			{
				return m_ConditionalFlags;
			}
			const Condition& GetConditionalFlags() const
			{
				return m_ConditionalFlags;
			}
	};
}

namespace Kortex::PackageProject
{
	class ComponentGroup: public KWithName
	{
		public:
			using Vector = std::vector<std::unique_ptr<ComponentGroup>>;
	
		private:
			SelectionMode m_SelectionMode;
			ComponentItem::Vector m_Entries;
	
		public:
			ComponentGroup();
			~ComponentGroup();
	
		public:
			SelectionMode GetSelectionMode() const
			{
				return m_SelectionMode;
			}
			void SetSelectionMode(SelectionMode type)
			{
				m_SelectionMode = type;
			}
	
			ComponentItem::Vector& GetEntries()
			{
				return m_Entries;
			}
			const ComponentItem::Vector& GetEntries() const
			{
				return m_Entries;
			}
	};
}

namespace Kortex::PackageProject
{
	class ComponentStep: public KWithName
	{
		public:
			using Vector = std::vector<std::unique_ptr<ComponentStep>>;
	
		private:
			ConditionGroup m_Conditions;
			ComponentGroup::Vector m_Entries;
	
		public:
			ComponentStep();
			~ComponentStep();
	
		public:
			ConditionGroup& GetConditionGroup()
			{
				return m_Conditions;
			}
			const ConditionGroup& GetConditionGroup() const
			{
				return m_Conditions;
			}
	
			ComponentGroup::Vector& GetGroups()
			{
				return m_Entries;
			}
			const ComponentGroup::Vector& GetGroups() const
			{
				return m_Entries;
			}		
	};
}

namespace Kortex::PackageProject
{
	class ConditionalComponentStep
	{
		public:
			using Vector = std::vector<std::unique_ptr<ConditionalComponentStep>>;
	
		private:
			ConditionGroup m_Conditions;
			KxStringVector m_Entries;
	
		public:
			ConditionalComponentStep();
			~ConditionalComponentStep();
			
		public:
			ConditionGroup& GetConditionGroup()
			{
				return m_Conditions;
			}
			const ConditionGroup& GetConditionGroup() const
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

namespace Kortex::PackageProject
{
	class ComponentsSection: public ProjectSection
	{
		public:
			static const Operator ms_DefaultFlagsOperator = KPP_OPERATOR_AND;
			static const SelectionMode ms_DefaultSelectionMode = KPPC_SELECT_ANY;
			static const TypeDescriptor ms_DefaultTypeDescriptor = KPPC_DESCRIPTOR_OPTIONAL;
	
		public:
			static TypeDescriptor StringToTypeDescriptor(const wxString& name, TypeDescriptor default = ms_DefaultTypeDescriptor);
			static wxString TypeDescriptorToString(TypeDescriptor type);
			static wxString TypeDescriptorToTranslation(TypeDescriptor type);
	
			static SelectionMode StringToSelectionMode(const wxString& name);
			static wxString SelectionModeToString(SelectionMode type);
			static wxString SelectionModeToTranslation(SelectionMode type);
	
		private:
			enum class FlagAttribute
			{
				Name,
				Value,
			};
	
		private:
			KxStringVector m_RequiredFileData;
			ComponentStep::Vector m_Steps;
			ConditionalComponentStep::Vector m_ConditionalSteps;
	
		private:
			KxStringVector GetFlagsAttributes(FlagAttribute index) const;
	
		public:
			ComponentsSection(ModPackageProject& project);
			~ComponentsSection();
	
		public:
			KxStringVector& GetRequiredFileData()
			{
				return m_RequiredFileData;
			}
			const KxStringVector& GetRequiredFileData() const
			{
				return m_RequiredFileData;
			}
			
			ComponentStep::Vector& GetSteps()
			{
				return m_Steps;
			}
			const ComponentStep::Vector& GetSteps() const
			{
				return m_Steps;
			}
			
			ConditionalComponentStep::Vector& GetConditionalSteps()
			{
				return m_ConditionalSteps;
			}
			const ConditionalComponentStep::Vector& GetConditionalSteps() const
			{
				return m_ConditionalSteps;
			}
			
			KxStringVector GetFlagsNames() const;
			KxStringVector GetFlagsValues() const;
	};
}
