#pragma once
#include "ReplicationCommand.h"
#include <map>

// TODO(you): World state replication lab session
class ReplicationManagerServer
{
public:
    void create(uint32 networkId);
    void update(uint32 networkId);
    void destroy(uint32 networkId);
    void play_audio(uint32 networkId);

    void write(OutputMemoryStream& packet);
public:
    std::map<uint32, ReplicationAction> actions;
};