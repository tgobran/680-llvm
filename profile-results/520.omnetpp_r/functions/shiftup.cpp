void cMessageHeap::shiftup(int from)
{
    // restores heap structure (in a sub-heap)
    int i,j;
    cMessage *temp;

    i = from;
    while ((j=i<<1) <= n)
    {
        if (j<n && (*h[j] > *h[j+1]))   //direction
            j++;
        if (*h[i] > *h[j])  //is change necessary?
        {
            temp=h[j];
            (h[j]=h[i])->heapindex=j;
            (h[i]=temp)->heapindex=i;
            i=j;
        }
        else
            break;
    }
}
