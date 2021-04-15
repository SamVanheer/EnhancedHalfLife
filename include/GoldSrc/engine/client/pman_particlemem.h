#pragma once

#include <vector>

class CCoreTriangleEffect;

constexpr int TRIANGLE_FPS = 30;

struct visibleparticles_t
{
	CCoreTriangleEffect *pVisibleParticle;
};

/**
*	@brief Memory block record.
*/
class MemoryBlock
{
private:
	char *m_pData;
	//bool m_bBlockIsInUse;
public:
	MemoryBlock(long lBlockSize)
	: next(nullptr), prev(nullptr)
	  //m_bBlockIsInUse(false) // Initialize block to 'free' state.
	{
		// Allocate memory here.
		m_pData = new char[lBlockSize];
	}

	virtual ~MemoryBlock()
	{
		// Free memory.
		delete[] m_pData;
	}
  
	inline char *Memory(void) { return m_pData; }

	MemoryBlock * next;
	MemoryBlock * prev;
};

class MemList
{
public:
	MemList() : m_pHead(nullptr) {}

	~MemList() { Reset(); }

	void Push(MemoryBlock * newItem)
	{
		if(!m_pHead)
		{
			m_pHead = newItem;
			newItem->next = nullptr;
			newItem->prev = nullptr;
			return;
		}

		MemoryBlock * temp = m_pHead;
		m_pHead = newItem;
		m_pHead->next = temp;
		m_pHead->prev = nullptr;
		
		temp->prev = m_pHead;
	}
	
	
	MemoryBlock * Front( void )
	{
		return(m_pHead);
	}

	MemoryBlock * Pop( void )
	{
		if(!m_pHead)
			return nullptr;

		MemoryBlock * temp = m_pHead;

		m_pHead = m_pHead->next;

		if(m_pHead)
			m_pHead->prev = nullptr;

		temp->next = nullptr;
		temp->prev = nullptr;

		return(temp);
	}

	void Delete( MemoryBlock * pItem)
	{

		if(m_pHead == pItem)
		{
			MemoryBlock * temp = m_pHead;

			m_pHead = m_pHead->next;
			if(m_pHead)
				m_pHead->prev = nullptr;

			temp->next = nullptr;
			temp->prev = nullptr;
			return;
		}

		MemoryBlock * prev = pItem->prev;
		MemoryBlock * next = pItem->next;

		if(prev)
			prev->next = next;
		
		if(next)
			next->prev = prev;

		pItem->next = nullptr;
		pItem->prev = nullptr;
	}

	void Reset( void )
	{
		while(m_pHead)
			Delete(m_pHead);
	}

private:
	MemoryBlock * m_pHead;
};


// Some helpful typedefs.
typedef std::vector<MemoryBlock *> VectorOfMemoryBlocks;
typedef VectorOfMemoryBlocks::iterator MemoryBlockIterator;

/**
*	@brief Mini memory manager - singleton.
*/
class CMiniMem
{
private:
	// Main memory pool.  Array is fine, but vectors are
	//  easier. :)
	static VectorOfMemoryBlocks m_vecMemoryPool;
	// Size of memory blocks in pool.
	static long m_lMemoryBlockSize;
	static long m_lMaxBlocks;
	static long m_lMemoryPoolSize;
	static CMiniMem *_instance;

	int m_iTotalParticles;
	int m_iParticlesDrawn;

protected:
	// private constructor and destructor.
	CMiniMem(long lMemoryPoolSize, long lMaxBlockSize);
	virtual ~CMiniMem();

	// ------------ Memory pool manager calls.
	// Find a free block and mark it as "in use".  Return nullptr
	//  if no free blocks found.
	char *AllocateFreeBlock(void);
public:

	// Return a pointer to usable block of memory.
	char *newBlock(void);

	// Mark a target memory item as no longer "in use".
	void deleteBlock(MemoryBlock *p);

	// Return the remaining capacity of the memory pool as a percent.
	long PercentUsed(void);

	void ProcessAll( void ); //Processes all

	void Reset( void ); //clears memory, setting all particles to not used.

	static int ApplyForce( Vector vOrigin, Vector vDirection, float flRadius, float flStrength );

	static CMiniMem *Instance(void);
	static long MaxBlockSize(void);

	bool CheckSize( int iSize );

	int GetTotalParticles( void ) { return m_iTotalParticles;	}
	int GetDrawnParticles( void ) { return m_iParticlesDrawn;	}
	void IncreaseParticlesDrawn( void ){ m_iParticlesDrawn++;	}
	
	void Shutdown( void );
	
	visibleparticles_t *m_pVisibleParticles;
};
