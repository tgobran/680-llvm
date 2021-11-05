inline unsigned int XMLString::stringLen(const XMLCh* const src)
{
    if (src == 0 || *src == 0)
    {
        return 0;
   }
    else
   {
        const XMLCh* pszTmp = src + 1;

        while (*pszTmp)
            ++pszTmp;

        return (unsigned int)(pszTmp - src);
    }
}