    void
    shrinkCount(size_type   theCount)
    {
        assert(m_size >= theCount);

        while (theCount > 0)
        {
            pop_back();

            --theCount;
        }
    }