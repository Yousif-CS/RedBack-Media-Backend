// A callback class when a session description is set correctly
#ifndef SET_SESSION_DESCRIPTION_OBSERVER_IMP_H_
#define SET_SESSION_DESCRIPTION_OBSERVER_IMP_H_

#include <iostream>
#include "api/peer_connection_interface.h"

class SetSessionDescriptionObserverImp: public webrtc::SetSessionDescriptionObserver {

public:
    void OnSuccess() override {
#ifdef REDBACK_DEBUG
        std::cout << "Session Description Set Correctly!" << std::endl;
#endif //REDBACK_DEBUB
    }

    void OnFailure(webrtc::RTCError error) override {
        std::cerr << "Error Setting Description: " << error.message() << std::endl;
    }
};

#endif