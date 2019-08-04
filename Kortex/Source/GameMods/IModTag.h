#pragma once
#include "stdafx.h"
#include <KxFramework/KxSingleton.h>

namespace Kortex
{
	class IModTag: public RTTI::IInterface<IModTag>
	{
		public:
			using Vector = std::vector<std::unique_ptr<IModTag>>;
			using RefVector = std::vector<IModTag*>;
			using CRefVector = std::vector<const IModTag*>;

		public:
			static std::optional<wxString> GetTranslatedNameByID(const wxString& id);

		public:
			virtual bool IsOK() const = 0;
			virtual std::unique_ptr<IModTag> Clone() const = 0;
			
			bool IsDefaultTag() const;

			virtual bool IsExpanded() const = 0;
			virtual void SetExpanded(bool isExpanded) = 0;
			void ToggleExpanded()
			{
				SetExpanded(!IsExpanded());
			}
			void Expand()
			{
				SetExpanded(true);
			}
			void Collapse()
			{
				SetExpanded(false);
			}

			virtual wxString GetID() const = 0;
			virtual void SetID(const wxString& value) = 0;

			virtual wxString GetName() const = 0;
			virtual void SetName(const wxString& label) = 0;

			virtual KxColor GetColor() const  = 0;
			virtual void SetColor(const KxColor& color) = 0;
			void ResetColor()
			{
				SetColor(KxColor());
			}
			bool HasColor() const
			{
				return GetColor().IsOk();
			}
	};
}

namespace Kortex
{
	class INexusModTag: public RTTI::IInterface<INexusModTag>
	{
		public:
			enum
			{
				InvalidNexusID = -1
			};

		public:
			virtual int GetNexusID() const = 0;
			virtual void SetNexusID(int value) = 0;
			bool HasNexusID() const
			{
				return GetNexusID() > 0;
			}
	};
}
