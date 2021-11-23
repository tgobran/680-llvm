bool cIndexedFileOutputVectorManager::record(void *vectorhandle, simtime_t t, double value)
{
    sVector *vp = (sVector *)vectorhandle;

    if (!vp->enabled)
        return false;

    if (vp->intervals.contains(t))
    {
        if (!vp->initialized)
            initVector(vp);

        sBlock &currentBlock = vp->currentBlock;
        eventnumber_t eventNumber = simulation.getEventNumber();
        if (currentBlock.count == 0)
        {
            currentBlock.startTime = t;
            currentBlock.startEventNum = eventNumber;
        }
        currentBlock.endTime = t;
        currentBlock.endEventNum = eventNumber;
        currentBlock.count++;
        currentBlock.min = std::min(currentBlock.min, value);
        currentBlock.max = std::max(currentBlock.max, value);
        currentBlock.sum += value;
        currentBlock.sumSqr += value*value;

        vp->buffer.push_back(sSample(t, eventNumber, value));
        memoryUsed += sizeof(sSample);

        if (vp->maxBufferedSamples > 0 && (int)vp->buffer.size() >= vp->maxBufferedSamples)
            writeBlock(vp);
        else if (memoryUsed >= maxMemoryUsed)
            writeRecords();

        return true;
    }
    return false;
}