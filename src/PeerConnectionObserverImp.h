// An implementation for the PeerConnectionObserver class
// required as a callback mechanism to the signaling thread
#ifndef PEER_CONNECTION_OBSERVER_IMP_H_
#define PEER_CONNECTION_OBSERVER_IMP_H_

#include <iostream>

#include "api/peer_connection_interface.h"

class PeerConnectionObserverImp: webrtc::PeerConnectionObserver {
public:
    void set_on_ice_candidate(std::function<void(const webrtc::IceCandidateInterface*)> callback){
        ice_candidate_callback = callback;
    }

    // Called when the signaling state changes
    virtual void OnSignalingChange(
        webrtc::PeerConnectionInterface::SignalingState new_state) override;

    // Called any time the IceGatheringState changes.
    virtual void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state) override;

    // A new ICE candidate has been gathered.
    virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;

    // Triggered when a remote peer opens a data channel.
    virtual void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;

private:
    std::function<void(rtc::scoped_refptr<webrtc::DataChannelInterface>)> data_channel_callback;
    std::function<void(const webrtc::IceCandidateInterface*)> ice_candidate_callback;
};

#endif