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
			TypeDetectorValue m_TypeDetector;

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
			TypeDetectorValue GetTypeDetection() const
			{
				return m_TypeDetector;
			}
			
			void Load(const KxXMLNode& node);
			void CopyIfNotSpecified(const ItemOptions& other);
	};
}
