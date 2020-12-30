#include "Networks.h"
#include "DeliveryManager.h"


Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
    Delivery del;
    packet << nextOutgoingSequenceNumber;

    del.sequenceNumber = nextOutgoingSequenceNumber;
    del.dispatchTime = Time.time;
    del.delegate = new PacketLost();

    ++nextOutgoingSequenceNumber;

    pendingDeliveries.push_back(del);

    return &del;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
    int sequenceNumber;
    packet >> sequenceNumber;

    for (auto i = pendingSequenceNumbers.begin(); i != pendingSequenceNumbers.end(); ++i)
    {
        if ((*i) == sequenceNumber)
        {
            return false;
        }
    }

    if (pendingSequenceNumbers.size() > 0 && sequenceNumber < pendingSequenceNumbers.back() + 1)
    {
        return false;
    }

    pendingSequenceNumbers.push_back(sequenceNumber);
    return true;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
    if (pendingSequenceNumbers.size() > 0)
    {
        return true;
    }

    return false;
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
    packet << (int)pendingSequenceNumbers.size();

    for (auto iter = pendingSequenceNumbers.begin(); iter != pendingSequenceNumbers.end(); ++iter)
    {
        packet << (*iter);
    }

    pendingSequenceNumbers.clear();
}

void DeliveryManager::processAckdSequenceNumbers(const InputMemoryStream& packet)
{
    int size;
    packet >> size;

    for (int i = 0; i < size; ++i)
    {
        int ackSequenceNumber;
        packet >> ackSequenceNumber;
        for (auto iter = pendingDeliveries.begin(); iter != pendingDeliveries.end();)
        {
            if ((*iter).sequenceNumber == ackSequenceNumber)
            {
                if ((*iter).delegate != nullptr)
                {
                    (*iter).delegate->onDeliverySuccess(this, (*iter).sequenceNumber);
                }

                iter = pendingDeliveries.erase(iter);
            }
            else
            {
                ++iter;
            }
        }
    }
}

void DeliveryManager::processTimedoutPackets()
{
    for (auto iter = pendingDeliveries.begin(); iter != pendingDeliveries.end();)
    {
        if ((*iter).dispatchTime + timedOut < Time.time)
        {
            if ((*iter).delegate != nullptr)
            {
                (*iter).delegate->onDeliveryFailure(this, (*iter).sequenceNumber);
            }

            iter = pendingDeliveries.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

void DeliveryManager::clear()
{
    pendingDeliveries.clear();
    pendingSequenceNumbers.clear();
}

void DeliveryDelegate::onDeliverySuccess(DeliveryManager* deliveryManager, int sequenceNumber)
{

}

void DeliveryDelegate::onDeliveryFailure(DeliveryManager* deliveryManager, int sequenceNumber)
{
}

void PacketLost::onDeliverySuccess(DeliveryManager* deliveryManager, int sequenceNumber)
{
    //LOG("Packet %i arrived!", sequenceNumber);
}

void PacketLost::onDeliveryFailure(DeliveryManager* deliveryManager, int sequenceNumber)
{
    //LOG("Packet %i was lost :c", sequenceNumber);
}
