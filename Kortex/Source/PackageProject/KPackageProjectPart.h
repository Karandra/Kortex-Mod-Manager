#pragma once
#include "stdafx.h"
#include "KPackageProjectDefs.h"

namespace Kortex
{
	class KPackageProject;
}

namespace Kortex::PackageProject
{
	class KPackageProjectPart
	{
		private:
			KPackageProject& m_Project;
	
		public:
			KPackageProjectPart(KPackageProject& project)
				:m_Project(project)
			{
			}
			virtual ~KPackageProjectPart() = default;
	
		public:
			KPackageProject& GetProject()
			{
				return m_Project;
			}
			const KPackageProject& GetProject() const
			{
				return m_Project;
			}
	};
}
