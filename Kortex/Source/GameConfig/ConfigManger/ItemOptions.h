#pragma once
#include "stdafx.h"
#include "ITypeDetector.h"
#include "Common.h"
#include <KxFramework/KxIndexedEnum.h>
class KxXMLNode;

namespace Kortex::GameConfig
{
	class ItemOptions
	{
		private:
			SourceFormatValue m_SourceFormat;
			TypeDetectorIDValue m_TypeDetector;

		public:
			ItemOptions() = default;
			ItemOptions(const KxXMLNode& node)
			{
				Load(node);
			}

		public:
			SourceFormatValue GetSourceFormat() const
			{
				return m_SourceFormat;
			}
			TypeDetectorIDValue GetTypeDetection() const
			{
				return m_TypeDetector;
			}
			
			void Load(const KxXMLNode& node);
			void CopyIfNotSpecified(const ItemOptions& other);
	};
}
