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
			virtual bool IsSystemTag() const = 0;

			virtual wxString GetID() const = 0;
			virtual void SetID(const wxString& value) = 0;

			virtual wxString GetName() const = 0;
			virtual void SetName(const wxString& label) = 0;

			virtual KxColor GetColor() const  = 0;
			virtual void SetColor(const KxColor& color) = 0;
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
