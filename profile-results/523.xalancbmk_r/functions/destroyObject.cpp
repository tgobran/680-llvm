	/*
	 * Destroy the object, and free the block for re-use.
	 *
	 * @param theObject the address of the object.
	 * @return true if the object was deleted, false if not.
	 */
	bool
	destroyObject(ObjectType*	theObject)
	{
		bool bResult = false;

		assert ( theObject != 0 );

		if ( this->m_blocks.empty() )
			return bResult;

		iterator iTerator = this->m_blocks.begin();

		iterator iEnd = this->m_blocks.end();

		// first , run over unfull blocks ( that consentrated from the head )
		while( iTerator != iEnd 
					&& (*iTerator)->blockAvailable() )
		{
			if ((*iTerator)->ownsBlock(theObject) == true)
			{
				(*iTerator)->destroyObject(theObject);
				
				// move the block we have just deleted to the head of the list
				if (iTerator != this->m_blocks.begin())
				{
					// move the block to the beginning
					ReusableArenaBlockType* block = *iTerator;

					assert(block != 0);
					
					this->m_blocks.erase(iTerator);

					this->m_blocks.push_front(block);
				}

				if (m_destroyBlocks)
				{
					destroyBlock();
				}

				bResult = true;

				break;
			}

			++iTerator;
		}

		reverse_iterator rIterator = this->m_blocks.rbegin();

		reverse_iterator rEnd = this->m_blocks.rend();

		// if the block hasn't been found from the head , start with full blocks ( from the taile)
		while ( !bResult && rIterator != rEnd )
		{
			if ((*rIterator)->ownsBlock(theObject))
			{
				(*rIterator)->destroyObject(theObject);

				if (rIterator != this->m_blocks.rbegin())
				{
					// move the block to the beginning
					ReusableArenaBlockType* block = *iTerator;

					assert(block != 0);
					
					this->m_blocks.erase(iTerator);

					this->m_blocks.push_front(block);

				}

				if (m_destroyBlocks)
				{
					destroyBlock();
				}

				bResult = true;

				break; 
			}

			if ( *rIterator == *iTerator)
			{
				break;
			}
			else
			{
				++rIterator;
			}
		}

		return bResult;

		assert ( bResult );
	}