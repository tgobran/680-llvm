bool
XalanDOMStringCache::release(XalanDOMString&	theString)
{
	XALAN_USING_STD(find)

	StringListType::iterator	i =
		find(m_busyList.begin(),
			 m_busyList.end(),
			 &theString);

	if (i == m_busyList.end())
	{
		return false;
	}
	else
	{
		if (m_availableList.size() > m_maximumSize)
		{
			m_allocator.destroy(theString);
		}
		else
		{
			theString.erase();

			m_availableList.push_back(*i);
		}

		m_busyList.erase(i);

		return true;
	}
}