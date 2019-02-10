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
			wxString m_InputFormat;
			wxString m_OutputFormat;
			SourceFormatValue m_SourceFormat;
			TypeDetectorValue m_TypeDetector;
			EditableBehaviorValue m_EditableBehavior;
			int m_Precision = -1;

		public:
			ItemOptions() = default;
			ItemOptions(const KxXMLNode& node, const DataType& dataType = {})
			{
				Load(node, dataType);
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
			
			EditableBehaviorValue GetEditableBehavior() const
			{
				return m_EditableBehavior;
			}

			bool HasInputFormat() const
			{
				return !m_InputFormat.IsEmpty();
			}
			wxString GetInputFormat() const
			{
				return HasInputFormat() ? m_InputFormat : wxS("%1");
			}

			bool HasOutputFormat() const
			{
				return !m_OutputFormat.IsEmpty();
			}
			wxString GetOutputFormat() const
			{
				return HasOutputFormat() ? m_OutputFormat : wxS("%1");
			}

			bool HasPrecision() const
			{
				return m_Precision >= 0 && m_Precision <= 8;
			}
			int GetPrecision() const
			{
				return m_Precision;
			}

			void Load(const KxXMLNode& node, const DataType& dataType = {});
			void CopyIfNotSpecified(const ItemOptions& other, const DataType& dataType = {});
	};
}
