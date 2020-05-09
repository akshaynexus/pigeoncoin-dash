#include <list>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>
#include "key.h"

/* simple HD chain data model (Legacy to be compatabile with old wallet.dat) */
class CHDChainLegacy
{
public:
    uint32_t nExternalChainCounter;
    uint32_t nInternalChainCounter;
    CKeyID masterKeyID; //!< master key hash160

    static const int VERSION_HD_BASE        = 1;
    static const int VERSION_HD_CHAIN_SPLIT = 2;
    static const int CURRENT_VERSION        = VERSION_HD_CHAIN_SPLIT;
    int nVersion;

    CHDChainLegacy() { SetNull(); }
    ADD_SERIALIZE_METHODS;
    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action)
    {
        READWRITE(this->nVersion);
        READWRITE(nExternalChainCounter);
        READWRITE(masterKeyID);
        if (this->nVersion >= VERSION_HD_CHAIN_SPLIT)
            READWRITE(nInternalChainCounter);
    }

    void SetNull()
    {
        nVersion = CHDChainLegacy::CURRENT_VERSION;
        nExternalChainCounter = 0;
        nInternalChainCounter = 0;
        masterKeyID.SetNull();
    }
};