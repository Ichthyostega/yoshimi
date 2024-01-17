/*
    GuiDataExchange.cpp - threadsafe and asynchronous data exchange into the GUI

    Copyright 2024,  Ichthyostega

    This file is part of yoshimi, which is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the License,
    or (at your option) any later version.

    yoshimi is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License (version 2
    or later) for more details.

    You should have received a copy of the GNU General Public License
    along with yoshimi.  If not, see <http://www.gnu.org/licenses/>.

*/


#include "Interface/GuiDataExchange.h"
#include "Misc/DataBlockBuff.h"
//#include "Misc/FormatFuncs.h"

//#include <functional>
#include <atomic>
//#include <string>
//#include <array>




namespace {
    std::atomic_size_t dataExchangeID{1};

    const size_t SIZ = 512; /////////////////////TODO find a way to derive sizes
    const size_t CAP = 64;
}

/**
 * Generate a new unique ID on each invocation, to be used as _Identity._
 * This allows to keep track of different connections and update receivers.
 */
size_t GuiDataExchange::generateUniqueID()
{          // Note : returning previous value before increment
    return dataExchangeID.fetch_add(+1, std::memory_order_relaxed);
}


/**
 * »PImpl« to maintain the block storage and manage the actual data exchange.
 */
class GuiDataExchange::DataManager
{
public:
    using Storage = DataBlockBuff<GuiDataExchange::RoutingTag, CAP, SIZ>;
    Storage storage;

    DataManager()
        : storage{}
        { }
};



// destructor needs the definition of ProtocolManager
GuiDataExchange::~GuiDataExchange() { }

/**
 * Create a protocol/mediator for data connection Core -> GUI
 * @param how_to_publish a function allowing to push a CommandBlock
 *        into some communication channel
 */
GuiDataExchange::GuiDataExchange(PublishFun how_to_publish)
    : publish{std::move(how_to_publish)}
    , manager{std::make_unique<DataManager>()}
    { }


/**
 * Open new storage slot by re-using the oldest storage buffer;
 * @param tag connection-ID to mark the new buffer, so it's contents
 *        can later be published to the correct receivers by dispatchUpdates()
 * @note using information encoded into the tag to ensure the buffer is suitable
 *        to hold a copy of the data to be published
 */
size_t GuiDataExchange::claimBuffer(RoutingTag const& tag)
{
    return manager->storage.claimNextBuffer(tag);
}

void* GuiDataExchange::getRawStorageBuff(size_t idx)
{
    return manager->storage.accessRawStorage(idx);
}

/**
 * This function is called automatically whenever a Subscription (=data receiver) is created.
 * The Subscription is associated with the RoutingTag and gets a callback for detaching on destruction
 */
GuiDataExchange::DetachHook GuiDataExchange::attachReceiver(RoutingTag const& tag, Subscription& client)
{
    throw std::logic_error("unimplemented");
}


void GuiDataExchange::publishSlot(size_t idx)
{
    throw std::logic_error("unimplemented");
}

void GuiDataExchange::dispatchUpdates(CommandBlock const& notification)
{
    throw std::logic_error("unimplemented");
}

