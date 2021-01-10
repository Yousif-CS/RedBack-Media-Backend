#include "eventsocketcpp/RedBackConnection.h"
#include "eventsocketcpp/RedBackMessage.h"

#ifndef PEER_CONNECTION_COMMON_H
#define PEER_CONNECTION_COMMON_H

// The different events that are required to establish, stream and watch footage/audio
enum class StreamEvents {
    None, Offer, Answer, IceCandidate, Watch, Stream, StreamRequestAck, StreamAvailable, NoStreamAvailable, StreamAlreadyAvailable
};

#endif