#pragma once
#include "stdafx.h"

class KUPtrVectorUtil
{
	private:
		enum class MoveMode
		{
			Before,
			After
		};
		template<class T> using ElementsVector = std::vector<std::unique_ptr<T>>;
		template<class T> using ElementsRefVector = std::vector<T*>;

		template<class T> static bool MoveElements(ElementsVector<T>& elementsVector,
										   const ElementsRefVector<T>& entriesToMove,
										   const T& anchor,
										   MoveMode moveMode
		)
		{
			auto FindAnchor = [&anchor](const auto& entry)
			{
				return entry.get() == &anchor;
			};

			// Check if anchor is not one of moved elements
			if (std::find(entriesToMove.begin(), entriesToMove.end(), &anchor) != entriesToMove.end())
			{
				return false;
			}

			// Remove all moved elements from existing place
			elementsVector.erase(std::remove_if(elementsVector.begin(), elementsVector.end(), [&entriesToMove](auto& entry)
			{
				// Release unique_ptr's and remove them
				// Pointer to it is present in 'entriesToMove' vector
				if (std::find(entriesToMove.begin(), entriesToMove.end(), entry.get()) != entriesToMove.end())
				{
					entry.release();
					return true;
				}
				return false;
			}), elementsVector.end());

			// Find anchor position
			auto anchorIt = std::find_if(elementsVector.begin(), elementsVector.end(), FindAnchor);
			if (anchorIt != elementsVector.end())
			{
				switch (moveMode)
				{
					case MoveMode::Before:
					{
						elementsVector.insert(anchorIt, entriesToMove.begin(), entriesToMove.end());
						break;
					}
					default:
					{
						size_t index = 1;
						for (auto it = entriesToMove.begin(); it != entriesToMove.end(); ++it)
						{
							elementsVector.emplace(anchorIt + index, *it);
							index++;
						}
						break;
					}
				};
				return true;
			}
			return false;
		}

	public:
		template<class T> static bool MoveBefore(ElementsVector<T>& elementsVector, const ElementsRefVector<T>& entriesToMove, const T& anchor)
		{
			return MoveElements<T>(elementsVector, entriesToMove, anchor, MoveMode::Before);
		}
		template<class T> static bool MoveAfter(ElementsVector<T>& elementsVector, const ElementsRefVector<T>& entriesToMove, const T& anchor)
		{
			return MoveElements<T>(elementsVector, entriesToMove, anchor, MoveMode::After);
		}
};
