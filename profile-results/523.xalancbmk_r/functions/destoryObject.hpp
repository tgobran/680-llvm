    /*
     * Destroy the object, and return the block to the free list.
     * The behavior is undefined if the object pointed to is not
     * owned by the block.
     *
     * @param theObject the address of the object.
     */
    void
    destroyObject(ObjectType*   theObject)
    {
        assert(theObject != 0);

        // check if any uncommited block is there, add it to the list
        if ( this->m_firstFreeBlock != this->m_nextFreeBlock )
        {
            // Return it to the pool of free blocks
            void* const     p = this->m_objectBlock + this->m_firstFreeBlock;

            new (p) NextBlock(this->m_nextFreeBlock);

            this->m_nextFreeBlock = this->m_firstFreeBlock;
        }

        assert(ownsObject(theObject) == true);
        assert(shouldDestroyBlock(theObject));

        XalanDestroy(*theObject);

        new (theObject) NextBlock(this->m_firstFreeBlock);

        m_firstFreeBlock =
            this->m_nextFreeBlock =
            size_type(theObject - this->m_objectBlock);

        assert (this->m_firstFreeBlock <= this->m_blockSize);

        --this->m_objectCount;
    }