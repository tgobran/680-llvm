void cMessageHeap::insert(cMessage *event)
{
    int i,j;

    event->insertordr = insertcntr++;

    if (++n > size)
    {
        size *= 2;
        cMessage **hnew = new cMessage *[size+1];
        for (i=1; i<=n-1; i++)
             hnew[i]=h[i];
        delete [] h;
        h = hnew;
    }

    take(event);

    for (j=n; j>1; j=i)
    {
        i = j>>1;
        if (*h[i] <= *event)   //direction
            break;

        (h[j]=h[i])->heapindex=j;
    }
    (h[j]=event)->heapindex=j;
}