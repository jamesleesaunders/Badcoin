// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_PUREHEADER_H
#define BITCOIN_PRIMITIVES_PUREHEADER_H

#include <serialize.h>
#include <uint256.h>
#include <consensus/params.h>

/** Multi-Algo definitions used to encode algorithm in nVersion */

enum {
    ALGO_SHA256D  = 0,
    ALGO_SCRYPT   = 1,
    ALGO_GROESTL  = 2,
    ALGO_SKEIN    = 3,
    ALGO_YESCRYPT = 4,
    NUM_ALGOS_IMPL
};

const int NUM_ALGOS = 5;

enum
{
    // primary version
    BLOCK_VERSION_DEFAULT        = 4,

    // algo
    BLOCK_VERSION_ALGO           = (7 << 9),
    BLOCK_VERSION_SCRYPT         = (1 << 9),
    BLOCK_VERSION_GROESTL        = (2 << 9),
    BLOCK_VERSION_SKEIN          = (3 << 9),
    BLOCK_VERSION_YESCRYPT       = (4 << 9),
};

/** extract algo from nVersion */
int GetAlgo(int nVersion);

/**
 * A block header without auxpow information.  This "intermediate step"
 * in constructing the full header is useful, because it breaks the cyclic
 * dependency between auxpow (referencing a parent block header) and
 * the block header (referencing an auxpow).  The parent block header
 * does not have auxpow itself, so it is a pure header.
 */
class CPureBlockHeader
{
private:

    /* Modifiers to the version.  */
    static const int32_t VERSION_AUXPOW = (1 << 8);

    /** Bits above are reserved for the auxpow chain ID.  */
    static const int32_t VERSION_CHAIN_START = (1 << 16);

public:
    // header
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;

    CPureBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action) {
        READWRITE(this->nVersion);
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);
    }

    void SetNull()
    {
        nVersion = 0;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

    uint256 GetHash() const;

    uint256 GetPoWHash(int algo, const Consensus::Params& consensusParams) const;
    
    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }

    /* Below are methods to interpret the version with respect to
       auxpow data and chain ID.  This used to be in the CBlockVersion
       class, but was moved here when we switched back to nVersion being
       a pure int member as preparation to undoing the "abuse" and
       allowing BIP9 to work.  */

    /**
     * Extract the base version (without modifiers and chain ID).
     * @return The base version./
     */
    inline int32_t GetBaseVersion() const
    {
        return GetBaseVersion(nVersion);
    }
    static inline int32_t GetBaseVersion(int32_t ver)
    {
        //return ver % VERSION_AUXPOW;
        return (ver & 0x000000ff);
    }

    /**
     * Set the base version (apart from chain ID and auxpow flag) to
     * the one given.  This should only be called when auxpow is not yet
     * set, to initialise a block!
     * @param nBaseVersion The base version.
     * @param nChainId The auxpow chain ID.
     */
    void SetBaseVersion(int32_t nBaseVersion, int32_t nChainId);

    /**
     * Extract the chain ID.
     * @return The chain ID encoded in the version.
     */
    inline int32_t GetChainId() const
    {
        return nVersion / VERSION_CHAIN_START;
    }

    /**
     * Set the chain ID.  This is used for the test suite.
     * @param ch The chain ID to set.
     */
    inline void SetChainId(int32_t chainId)
    {
        nVersion %= VERSION_CHAIN_START;
        nVersion |= chainId * VERSION_CHAIN_START;
    }

    /**
     * Check if the auxpow flag is set in the version.
     * @return True iff this block version is marked as auxpow.
     */
    inline bool IsAuxpow() const
    {
        return nVersion & VERSION_AUXPOW;
    }

    /**
     * Set the auxpow flag.  This is used for testing.
     * @param auxpow Whether to mark auxpow as true.
     */
    inline void SetAuxpowVersion (bool auxpow)
    {
        if (auxpow)
            nVersion |= VERSION_AUXPOW;
        else
            nVersion &= ~VERSION_AUXPOW;
    }

    /**
     * Check whether this is a "legacy" block without chain ID.
     * @return True iff it is.
     */
    inline bool IsLegacy() const
    {
        return nVersion == 1;
    }
    
    /** Extract algo from blockheader */
    inline int GetAlgo() const
    {
        return ::GetAlgo(nVersion);
    }
    
    /** Encode the algorithm into nVersion */
    inline void SetAlgo(int algo)
    {
        switch(algo)
        {
            case ALGO_SHA256D:
                break;
            case ALGO_SCRYPT:
                nVersion |= BLOCK_VERSION_SCRYPT;
                break;
            case ALGO_GROESTL:
                nVersion |= BLOCK_VERSION_GROESTL;
                break;
            case ALGO_SKEIN:
                nVersion |= BLOCK_VERSION_SKEIN;
                break;
            case ALGO_YESCRYPT:
                nVersion |= BLOCK_VERSION_YESCRYPT;
                break;
            default:
                break;
        }
    }
};

#endif // BITCOIN_PRIMITIVES_PUREHEADER_H
