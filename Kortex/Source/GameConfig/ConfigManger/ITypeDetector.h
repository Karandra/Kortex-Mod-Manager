#pragma once
#include "stdafx.h"
#include "Common.h"
#include "DataType.h"
class KxXMLNode;

namespace Kortex::GameConfig
{
	class ITypeDetector
	{
		public:
			virtual ~ITypeDetector() = default;

		public:
			virtual TypeID GetType(const wxString& valueName, const wxString& valueData) const = 0;
	};
}

namespace Kortex::GameConfig
{
	class HungarianNotationTypeDetector: public ITypeDetector
	{
		private:
			std::unordered_map<wxString, TypeID> m_TypeMap;

		public:
			HungarianNotationTypeDetector(const KxXMLNode& rootNode);

		public:
			TypeID GetType(const wxString& valueName, const wxString& valueData) const override;
	};
}

namespace Kortex::GameConfig
{
	class DataAnalysisTypeDetector: public ITypeDetector
	{
		public:
			TypeID GetType(const wxString& valueName, const wxString& valueData) const override;
	};
}
