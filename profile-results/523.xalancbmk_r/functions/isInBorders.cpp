	/*
	 * Determine if this block is located between beginning of the array
	 * and the "rightBorder" array member (not included)
	 * @param theObject the address of the object
	 *		  rightBorder the right 
	 * @return true if we own the object block, false if not.
	 */
	bool
	isInBorders(
            const ObjectType*	theObject,
            size_type           rightBoundary) const
	{
		if ( rightBoundary > m_blockSize )
		{
			rightBoundary = m_blockSize;
		}

        // Use less<>, since it's guaranteed to do pointer
		// comparisons correctly...
		XALAN_STD_QUALIFIER less<const ObjectType*>		functor;

		if (functor(theObject, m_objectBlock) == false &&
			functor(theObject, m_objectBlock + rightBoundary) == true)
		{
			return true;
		}
		else
		{
			return false;
		}
	}