bool cGate::deliver(cMessage *msg, simtime_t t)
{
    if (nextgatep==NULL)
    {
        getOwnerModule()->arrived(msg, this, t);
        return true;
    }
    else
    {
        if (channelp)
        {
            if (!channelp->initialized())
                throw cRuntimeError(channelp, "Channel not initialized (did you forget to invoke "
                                              "callInitialize() for a dynamically created channel or "
                                              "a dynamically created compound module that contains it?)");
            return channelp->deliver(msg, t);
        }
        else
        {
            EVCB.messageSendHop(msg, this);
            return nextgatep->deliver(msg, t);
        }
    }
}