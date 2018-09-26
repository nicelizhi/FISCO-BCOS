#include "MinerPrecompiled.h"
#include "libstorage/DBFactoryPrecompiled.h"
#include "libstorage/EntriesPrecompiled.h"
#include <libdevcore/easylog.h>
#include <libethcore/ABI.h>

using namespace dev;
using namespace dev::precompiled;

bytes MinerPrecompiled::call(PrecompiledContext::Ptr context, bytesConstRef param)
{
    LOG(TRACE) << "this: " << this << " call MinerPrecompiled:" << toHex(param);

    //解析出函数名
    uint32_t func = getParamFunc(param);
    bytesConstRef data = getParamData(param);

    LOG(DEBUG) << "func:" << std::hex << func;

    dev::eth::ContractABI abi;
    bytes out;
    switch (func)
    {
    case 0xb0c8f9dc:
    {  // add(string)
        std::string nodeID;
        abi.abiOut(data, nodeID);
        if (nodeID.size() != 128u)
        {
            LOG(DEBUG) << "NodeID length error. " << nodeID;
            break;
        }
        storage::DB::Ptr db = openTable(context, "_sys_miners_");
        if (db.get())
        {
            LOG(DEBUG) << "MinerPrecompiled add miner nodeID : " << nodeID;
            auto condition = db->newCondition();
            condition->EQ(MINER_KEY_NODEID, nodeID);
            auto entries = db->select(MINER_TYPE_MINER, condition);
            auto entry = db->newEntry();
            entry->setField(MINER_PRIME_KEY, MINER_TYPE_MINER);
            if (entries->size() == 0u)
            {
                entries = db->select(MINER_TYPE_OBSERVER, condition);
                if (entries->size() == 0u)
                {
                    entry->setField(MINER_KEY_NODEID, nodeID);
                    entry->setField(MINER_KEY_ENABLENUM, (context->blockInfo().number + 1).str());
                    db->insert(MINER_TYPE_MINER, entry);
                    break;
                }
                db->update(MINER_TYPE_OBSERVER, entry, condition);
                break;
            }
            db->update(MINER_TYPE_MINER, entry, condition);
        }
        break;
    }
    case 0x80599e4b:
    {  // remove(string)
        std::string nodeID;
        abi.abiOut(data, nodeID);
        if (nodeID.size() != 128u)
        {
            LOG(DEBUG) << "NodeID length error. " << nodeID;
            break;
        }
        storage::DB::Ptr db = openTable(context, "_sys_miners_");
        if (db.get())
        {
            auto condition = db->newCondition();
            condition->EQ(MINER_KEY_NODEID, nodeID);
            auto entries = db->select(MINER_TYPE_MINER, condition);
            if (entries->size() == 0u)
                break;
            auto entry = db->newEntry();
            entry->setField(MINER_PRIME_KEY, MINER_TYPE_OBSERVER);
            db->update(MINER_TYPE_MINER, entry, condition);
            LOG(DEBUG) << "MinerPrecompiled remove miner nodeID : " << nodeID;
        }
        break;
    }
    default:
    {
        break;
    }
    }

    return out;
}