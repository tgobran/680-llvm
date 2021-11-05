void MACRelayUnitBase::printAddressTable()
{
    AddressTable::iterator iter;
    EV << "Address Table (" << addresstable.size() << " entries):\n";
    for (iter = addresstable.begin(); iter!=addresstable.end(); ++iter)
    {
        EV << "  " << iter->first << " --> port" << iter->second.portno <<
              (iter->second.insertionTime+agingTime <= simTime() ? " (aged)" : "") << endl;
    }
}