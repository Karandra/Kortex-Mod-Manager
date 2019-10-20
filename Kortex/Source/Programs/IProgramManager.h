#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Application/IModule.h"
#include "IProgramItem.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxProcess.h>
class KxXMLNode;
class KxMenu;

namespace Kortex
{
	namespace ProgramManager::Internal
	{
		extern const SimpleManagerInfo TypeInfo;
	}

	class IProgramManager:
		public ManagerWithTypeInfo<IManager, ProgramManager::Internal::TypeInfo>,
		public KxSingletonPtr<IProgramManager>
	{
		public:
			enum class BitmapVariant
			{
				Small,
				Large
			};

		private:
			void OnAddMainMenuItems(KxMenu& menu);

			std::unique_ptr<KxProcess> DoCreateProcess(const IProgramItem& entry) const;
			int DoRunProcess(std::unique_ptr<KxProcess> process) const;
			bool DoCheckEntry(const IProgramItem& entry) const;

		protected:
			void LoadProgramsFromXML(IProgramItem::Vector& programs, const KxXMLNode& rootNode);

		public:
			IProgramManager();

		public:
			virtual const IProgramItem::Vector& GetProgramList() const = 0;
			virtual IProgramItem::Vector& GetProgramList() = 0;

			virtual std::unique_ptr<IProgramItem> NewProgram() = 0;
			virtual void RemoveProgram(IProgramItem& programEntry) = 0;
			IProgramItem& EmplaceProgram()
			{
				return *GetProgramList().emplace_back(NewProgram());
			}
			IProgramItem& EmplaceProgram(std::unique_ptr<IProgramItem> programEntry)
			{
				return *GetProgramList().emplace_back(std::move(programEntry));
			}

			// Loads default programs for this instance
			virtual void LoadDefaultPrograms() = 0;

			// Created process can be run with either 'KxPROCESS_WAIT_SYNC' or 'KxPROCESS_WAIT_ASYNC' flag.
			// Or use 'RunProcess' to run it with default parameters.
			std::unique_ptr<KxProcess> CreateProcess(const IProgramItem& entry) const;
			int RunProcess(std::unique_ptr<KxProcess> process) const;

			// Check entry paths and perform 'CreateProcess -> RunProcess' sequence on it.
			int RunEntry(const IProgramItem& entry) const;

		public:
			wxBitmap LoadProgramIcon(const IProgramItem& entry, BitmapVariant bitmapVariant) const;
			void LoadProgramIcons(IProgramItem& entry) const;
			bool CheckProgramIcons(const IProgramItem& entry) const;
	};
}
