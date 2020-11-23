#pragma once
#include "ReplicationCommand.h"

// TODO(you): World state replication lab session
class ReplicationManagerClient
{
public:
    void read(const InputMemoryStream& packet);
};