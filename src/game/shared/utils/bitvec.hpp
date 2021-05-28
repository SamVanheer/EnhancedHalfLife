//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once

#include <cassert>
#include <cstdint>
#include <cstring>

class CBitVecAccessor
{
public:
	CBitVecAccessor(std::uint32_t* pDWords, int iBit);

	void		operator=(int val);
	operator std::uint32_t();

private:
	std::uint32_t* m_pDWords;
	int				m_iBit;
};


// CBitVec allows you to store a list of bits and do operations on them like they were 
// an atomic type.
template<int NUM_BITS>
class CBitVec
{
public:

	CBitVec();

	// Set all values to the specified value (0 or 1..)
	void			Init(int val = 0);

	// Access the bits like an array.
	CBitVecAccessor	operator[](int i);

	// Operations on other bit vectors.
	CBitVec& operator=(CBitVec<NUM_BITS> const& other);
	bool			operator==(CBitVec<NUM_BITS> const& other);
	bool			operator!=(CBitVec<NUM_BITS> const& other);

	// Get underlying dword representations of the bits.
	int				GetNumDWords();
	std::uint32_t	GetDWord(int i);
	void			SetDWord(int i, std::uint32_t val);

	int				GetNumBits();

private:

	enum { NUM_DWORDS = NUM_BITS / 32 + !!(NUM_BITS & 31) };
	std::uint32_t	m_DWords[NUM_DWORDS];
};



// ------------------------------------------------------------------------ //
// CBitVecAccessor inlines.
// ------------------------------------------------------------------------ //

inline CBitVecAccessor::CBitVecAccessor(std::uint32_t* pDWords, int iBit)
{
	m_pDWords = pDWords;
	m_iBit = iBit;
}


inline void CBitVecAccessor::operator=(int val)
{
	if (val)
		m_pDWords[m_iBit >> 5] |= (1 << (m_iBit & 31));
	else
		m_pDWords[m_iBit >> 5] &= ~(std::uint32_t)(1 << (m_iBit & 31));
}

inline CBitVecAccessor::operator std::uint32_t()
{
	return m_pDWords[m_iBit >> 5] & (1 << (m_iBit & 31));
}



// ------------------------------------------------------------------------ //
// CBitVec inlines.
// ------------------------------------------------------------------------ //

template<int NUM_BITS>
inline int CBitVec<NUM_BITS>::GetNumBits()
{
	return NUM_BITS;
}


template<int NUM_BITS>
inline CBitVec<NUM_BITS>::CBitVec()
{
	for (int i = 0; i < NUM_DWORDS; i++)
		m_DWords[i] = 0;
}


template<int NUM_BITS>
inline void CBitVec<NUM_BITS>::Init(int val)
{
	for (int i = 0; i < GetNumBits(); i++)
	{
		(*this)[i] = val;
	}
}


template<int NUM_BITS>
inline CBitVec<NUM_BITS>& CBitVec<NUM_BITS>::operator=(CBitVec<NUM_BITS> const& other)
{
	memcpy(m_DWords, other.m_DWords, sizeof(m_DWords));
	return *this;
}


template<int NUM_BITS>
inline CBitVecAccessor CBitVec<NUM_BITS>::operator[](int i)
{
	assert(i >= 0 && i < GetNumBits());
	return CBitVecAccessor(m_DWords, i);
}


template<int NUM_BITS>
inline bool CBitVec<NUM_BITS>::operator==(CBitVec<NUM_BITS> const& other)
{
	for (int i = 0; i < NUM_DWORDS; i++)
		if (m_DWords[i] != other.m_DWords[i])
			return false;

	return true;
}


template<int NUM_BITS>
inline bool CBitVec<NUM_BITS>::operator!=(CBitVec<NUM_BITS> const& other)
{
	return !(*this == other);
}


template<int NUM_BITS>
inline int CBitVec<NUM_BITS>::GetNumDWords()
{
	return NUM_DWORDS;
}

template<int NUM_BITS>
inline std::uint32_t CBitVec<NUM_BITS>::GetDWord(int i)
{
	assert(i >= 0 && i < NUM_DWORDS);
	return m_DWords[i];
}


template<int NUM_BITS>
inline void CBitVec<NUM_BITS>::SetDWord(int i, std::uint32_t val)
{
	assert(i >= 0 && i < NUM_DWORDS);
	m_DWords[i] = val;
}