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

		template<class T> static bool MoveElements(ElementsVector<T>& allItems,
										   const ElementsRefVector<T>& toMove,
										   const T& anchor,
										   MoveMode moveMode
		)
		{
			auto FindAnchor = [&anchor](const auto& entry)
			{
				return entry.get() == &anchor;
			};

			// Check if anchor is not one of moved elements
			if (std::find(toMove.begin(), toMove.end(), &anchor) != toMove.end())
			{
				return false;
			}

			// Remove all moved elements from existing place
			allItems.erase(std::remove_if(allItems.begin(), allItems.end(), [&toMove](auto& entry)
			{
				// Release unique_ptr's and remove them
				// Pointer to it is present in 'entriesToMove' vector
				if (std::find(toMove.begin(), toMove.end(), entry.get()) != toMove.end())
				{
					entry.release();
					return true;
				}
				return false;
			}), allItems.end());

			// Find anchor position
			auto anchorIt = std::find_if(allItems.begin(), allItems.end(), FindAnchor);
			if (anchorIt != allItems.end())
			{
				switch (moveMode)
				{
					case MoveMode::Before:
					{
						allItems.insert(anchorIt, toMove.begin(), toMove.end());
						break;
					}
					default:
					{
						size_t index = 1;
						for (auto it = toMove.begin(); it != toMove.end(); ++it)
						{
							allItems.emplace(anchorIt + index, *it);
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
		template<class T> static bool MoveBefore(ElementsVector<T>& allItems, const ElementsRefVector<T>& toMove, const T& anchor)
		{
			return MoveElements<T>(allItems, toMove, anchor, MoveMode::Before);
		}
		template<class T> static bool MoveAfter(ElementsVector<T>& allItems, const ElementsRefVector<T>& toMove, const T& anchor)
		{
			return MoveElements<T>(allItems, toMove, anchor, MoveMode::After);
		}
};
