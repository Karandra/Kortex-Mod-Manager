#pragma once
#include "stdafx.h"
#include "Utility/WithBitmap.h"
#include <KxFramework/KxXML.h>

namespace Kortex
{
	class IProgramItem
	{
		public:
			using Vector = std::vector<std::unique_ptr<IProgramItem>>;
			using RefVector = std::vector<IProgramItem*>;

		public:
			virtual ~IProgramItem() = default;

		public:
			virtual bool IsOK() const = 0;
			virtual void Load(const KxXMLNode& node) = 0;
			virtual void Save(KxXMLNode& node) const = 0;

			virtual bool RequiresVFS() const = 0;
			virtual bool CanRunNow() const = 0;
			virtual void OnRun() = 0;

			virtual bool ShouldShowInMainMenu() const = 0;
			virtual void ShowInMainMenu(bool value) = 0;

			virtual wxString RawGetName() const
			{
				return GetName();
			}
			virtual wxString GetName() const = 0;
			virtual void SetName(const wxString& value) = 0;
			
			virtual wxString RawGetIconPath() const
			{
				return GetIconPath();
			}
			virtual wxString GetIconPath() const = 0;
			virtual void SetIconPath(const wxString& value) = 0;
			
			virtual wxString RawGetExecutable() const
			{
				return GetExecutable();
			}
			virtual wxString GetExecutable() const = 0;
			virtual void SetExecutable(const wxString& value) = 0;

			virtual wxString RawGetArguments() const
			{
				return GetArguments();
			}
			virtual wxString GetArguments() const = 0;
			virtual void SetArguments(const wxString& value) = 0;

			virtual wxString RawGetWorkingDirectory() const
			{
				return GetWorkingDirectory();
			}
			virtual wxString GetWorkingDirectory() const = 0;
			virtual void SetWorkingDirectory(const wxString& value) = 0;

			virtual Utility::WithBitmap& GetSmallBitmap() = 0;
			const Utility::WithBitmap& GetSmallBitmap() const
			{
				return const_cast<IProgramItem&>(*this).GetSmallBitmap();
			}

			virtual Utility::WithBitmap& GetLargeBitmap() = 0;
			const Utility::WithBitmap& GetLargeBitmap() const
			{
				return const_cast<IProgramItem&>(*this).GetLargeBitmap();
			}
	};
}
