void EtherMAC::printState()
{
#define CASE(x) case x: EV << #x; break
    EV << "transmitState: ";
    switch (transmitState) {
        CASE(TX_IDLE_STATE);
        CASE(WAIT_IFG_STATE);
        CASE(TRANSMITTING_STATE);
        CASE(JAMMING_STATE);
        CASE(BACKOFF_STATE);
        CASE(PAUSE_STATE);
    }
    EV << ",  receiveState: ";
    switch (receiveState) {
        CASE(RX_IDLE_STATE);
        CASE(RECEIVING_STATE);
        CASE(RX_COLLISION_STATE);
    }
    EV << ",  backoffs: " << backoffs;
    EV << ",  numConcurrentTransmissions: " << numConcurrentTransmissions;
    EV << ",  queueLength: " << txQueue.length() << endl;
#undef CASE
}