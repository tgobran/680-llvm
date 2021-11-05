inline bool XMLString::equals(   const char* const    str1
                               , const char* const    str2)
{
    const char* psz1 = str1;
    const char* psz2 = str2;

    if (psz1 == 0 || psz2 == 0) {
        if ((psz1 != 0 && *psz1) || (psz2 != 0 && *psz2))
            return false;
        else
            return true;
    }

    while (*psz1 == *psz2)
    {
        // If either has ended, then they both ended, so equal
        if (!*psz1)
            return true;

        // Move upwards for the next round
        psz1++;
        psz2++;
    }
    return false;
}