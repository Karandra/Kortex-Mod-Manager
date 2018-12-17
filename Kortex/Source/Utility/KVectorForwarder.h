#pragma once
#include "stdafx.h"

template<class VectorT> class KVectorForwarder
{
	public:
		using value_type = typename VectorT::value_type;
		using allocator_type = typename VectorT::allocator_type;
		using size_type = typename VectorT::size_type;
		using difference_type = typename VectorT::difference_type;

		using reference = typename VectorT::reference;
		using const_reference = typename VectorT::const_reference;
		using pointer = typename VectorT::pointer;
		using const_pointer = typename VectorT::const_pointer;

		using iterator = typename VectorT::iterator;
		using const_iterator = typename VectorT::const_iterator;
		using reverse_iterator = typename VectorT::reverse_iterator;
		using const_reverse_iterator = typename VectorT::const_reverse_iterator;

	private:
		VectorT* m_Vector = nullptr;

	public:
		KVectorForwarder(VectorT& value)
			:m_Vector(&value)
		{
		}

	public:
		bool empty() const
		{
			return m_Vector->empty();
		}
		size_type size() const
		{
			return m_Vector->size();
		}

		const value_type& operator[](size_t i) const
		{
			return *m_Vector[i];
		}
		value_type& operator[](size_t i)
		{
			return *m_Vector[i];
		}

		const value_type& front() const
		{
			return m_Vector->front();
		}
		value_type& front()
		{
			return m_Vector->front();
		}
		const value_type& back() const
		{
			return m_Vector->back();
		}
		value_type& back()
		{
			return m_Vector->back();
		}

		iterator begin()
		{
			return m_Vector->begin();
		}
		iterator end()
		{
			return m_Vector->begin();
		}
		const_iterator begin() const
		{
			return m_Vector->begin();
		}
		const_iterator end() const
		{
			return m_Vector->begin();
		}
};
