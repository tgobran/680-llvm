bool ValueStore::contains(const FieldValueMap* const other) {

    if (fValueTuples) {

        unsigned int otherSize = other->size();
        unsigned int tupleSize = fValueTuples->size();

        for (unsigned int i=0; i<tupleSize; i++) {

            FieldValueMap* valueMap = fValueTuples->elementAt(i);

            if (otherSize == valueMap->size()) {

                bool matchFound = true;

                for (unsigned int j=0; j<otherSize; j++) {
                    if (!isDuplicateOf(valueMap->getDatatypeValidatorAt(j), valueMap->getValueAt(j),
                                       other->getDatatypeValidatorAt(j), other->getValueAt(j))) {
                        matchFound = false;
                        break;
                    }
                }

                if (matchFound) { // found it
                    return true;
                }
            }
        }
    }

    return false;
}