#pragma once
#include "stdafx.h"
#include "Application/IManager.h"
#include "Application/IModule.h"
#include "IProgramEntry.h"
#include <KxFramework/KxMenu.h>
#include <KxFramework/KxSingleton.h>
#include <KxFramework/KxProcess.h>
class KMainWindow;
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
		friend class KMainWindow;

		public:
			enum class BitmapVariant
			{
				Small,
				Large
			};

		private:
			void OnAddMainMenuItems(KxMenu& menu);

			std::unique_ptr<KxProcess> DoCreateProcess(const IProgramEntry& entry) const;
			int DoRunProcess(std::unique_ptr<KxProcess> process) const;
			bool DoCheckEntry(const IProgramEntry& entry) const;

		protected:
			void LoadProgramsFromXML(IProgramEntry::Vector& programs, const KxXMLNode& rootNode);

		public:
			IProgramManager();

		public:
			virtual const IProgramEntry::Vector& GetProgramList() const = 0;
			virtual IProgramEntry::Vector& GetProgramList() = 0;

			virtual std::unique_ptr<IProgramEntry> NewProgram() = 0;
			virtual void RemoveProgram(IProgramEntry& programEntry) = 0;
			IProgramEntry& EmplaceProgram()
			{
				return *GetProgramList().emplace_back(NewProgram());
			}
			IProgramEntry& EmplaceProgram(std::unique_ptr<IProgramEntry> programEntry)
			{
				return *GetProgramList().emplace_back(std::move(programEntry));
			}

			// Loads default programs for this instance
			virtual void LoadDefaultPrograms() = 0;

			// Created process can be run with either 'KxPROCESS_WAIT_SYNC' or 'KxPROCESS_WAIT_ASYNC' flag.
			// Or use 'RunProcess' to run it with default parameters.
			std::unique_ptr<KxProcess> CreateProcess(const IProgramEntry& entry) const;
			int RunProcess(std::unique_ptr<KxProcess> process) const;

			// Check entry paths and perform 'CreateProcess -> RunProcess' sequence on it.
			int RunEntry(const IProgramEntry& entry) const;

		public:
			wxBitmap LoadProgramIcon(const IProgramEntry& entry, BitmapVariant bitmapVariant) const;
			void LoadProgramIcons(IProgramEntry& entry) const;
			bool CheckProgramIcons(const IProgramEntry& entry) const;
	};
}
