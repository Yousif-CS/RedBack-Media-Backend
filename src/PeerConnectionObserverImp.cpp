// Implementation file for the concrete class of PeerConnectionObserver

#include <iostream>
#include "PeerConnectionObserverImp.h"
#include "PeerConnectionCommon.h"

#include "eventsocketcpp/RedBackMessage.h"

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
 
void PeerConnectionObserverImp::OnRenegotiationNeeded()
{
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

        std::cout << "Ice candidate gathered!" << std::endl;

        //a callback to execute when an ice candidate is gathered
        ice_candidate_callback_(candidate);
}

// Triggered when a remote peer opens a data channel.
void PeerConnectionObserverImp::OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel){

        std::cout << "Data Channel open!" << std::endl;

    //deal with the data channel
    data_channel_callback_(data_channel);

}

void PeerConnectionObserverImp::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver)
{
    // Track ready. At this point do nothing really. Later feature: Notify connected clients that
    // A track is ready to be viewed
}
