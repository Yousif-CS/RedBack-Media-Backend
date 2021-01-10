// A callback class when a session description is set correctly
#ifndef SET_SESSION_DESCRIPTION_OBSERVER_IMP_H_
#define SET_SESSION_DESCRIPTION_OBSERVER_IMP_H_

#include <iostream>

#include "PeerConnectionHelpers.h"

#include "api/peer_connection_interface.h"

class SetSessionDescriptionObserverImp: public webrtc::SetSessionDescriptionObserver {

public:

    // Only provide the peerConnection if we need to do a follow up operation like sending an answer back.
    SetSessionDescriptionObserverImp(std::shared_ptr<RedBack::WebRTC::PeerConnection>  peerConnection)
    :SetSessionDescriptionObserver()
    {
        _peerConnection = peerConnection;
    }

    void OnSuccess() override {

        const webrtc::SessionDescriptionInterface* remoteDescp = _peerConnection->RemoteDescription();
        const webrtc::SessionDescriptionInterface* localDescp = _peerConnection->LocalDescription();
        
        // If we already have both descriptions set, that's it
        if (remoteDescp && localDescp)
            return;

        // No need to check if the remote description is not set, because we cannot do much 
        // until the other peer sends it

        // If the local description is not set, we go ahead and create one.
        // We first need to know which one to create: an offer or an answer
        if (!localDescp)
        {
            webrtc::SdpType tocreate{webrtc::SdpType::kOffer};

            if (remoteDescp->GetType() == webrtc::SdpType::kOffer)
                tocreate = webrtc::SdpType::kAnswer;

            _peerConnection->CreateSessionDescription(tocreate);
        }
    }

    void OnFailure(webrtc::RTCError error) override {
        std::cerr << "Error Setting Description: " << error.message() << std::endl;
    }
private:
    std::shared_ptr<RedBack::WebRTC::PeerConnection> _peerConnection;
};

#endif