#pragma once 

using ElementId = int;
using TimeOffset = unsigned;
using Time = unsigned;

enum class ReceiverType {
    WORKER,
    STOREHOUSE
};

enum class NodeType{
    RAMP,
    WORKER,
    STOREHOUSE
};