// Implementation file for the concrete class of PeerConnectionObserver

#include <iostream>
#include "PeerConnectionObserverImp.h"

void PeerConnectionObserverImp::OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState new_state){

#ifdef REDBACK_DEBUG
        std::cout<< "Signaling State Changed To: ";

        switch(new_state){
            case webrtc::PeerConnectionInterface::SignalingState::kHaveLocalOffer:
                std::cout << "HaveLocalOffer";
                break;
            case webrtc::PeerConnectionInterface::SignalingState::kHaveLocalPrAnswer:
                std::cout << "HaveLocalPrAnswer";
                break;
            case webrtc::PeerConnectionInterface::SignalingState::kHaveRemoteOffer:
                std::cout << "HaveRemoteOffer";
                break;
            case webrtc::PeerConnectionInterface::SignalingState::kHaveRemotePrAnswer:
                std::cout << "HaveRemotePrAnswer";
                break;
            case webrtc::PeerConnectionInterface::SignalingState::kClosed:
                std::cout << "Closed";
                break;
            case webrtc::PeerConnectionInterface::SignalingState::kStable:
                std::cout << "Stable";
        }

        std::cout << std::endl;
#endif

}
    
// Called any time the IceGatheringState changes.
void PeerConnectionObserverImp::OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state){

#ifdef REDBACK_DEBUG
        std::cout<< "Ice Gathering State Changed To: ";

        switch(new_state){
            case webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringNew:
                std::cout << "New";
                break;
            case webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringGathering:
                std::cout << "Gathering";
                break;
            case webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete:
                std::cout << "Complete";
        }

        std::cout << std::endl;

#endif //REDBACK_DEBUG

}

// A new ICE candidate has been gathered.
void PeerConnectionObserverImp::OnIceCandidate(const webrtc::IceCandidateInterface* candidate){

#ifdef REDBACK_DEBUG
        std::cout << "Ice candidate gathered!" << std::endl;
#endif //REDBACK_DEBUG

        //a callback to execute when an ice candidate is gathered
        ice_candidate_callback(candidate);
}

// Triggered when a remote peer opens a data channel.
void PeerConnectionObserverImp::OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel){

#ifdef REDBACK_DEBUG
        std::cout << "Data Channel open!" << std::endl;
#endif //REDBACK_DEBUG

    //deal with the data channel
    data_channel_callback(data_channel);

}