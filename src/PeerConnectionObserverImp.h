// An implementation for the PeerConnectionObserver class
// required as a callback mechanism to the signaling thread
#ifndef PEER_CONNECTION_OBSERVER_IMP_H_
#define PEER_CONNECTION_OBSERVER_IMP_H_

#include <iostream>

#include "api/peer_connection_interface.h"
#include "PeerConnectionCommon.h"
class PeerConnectionObserverImp: public webrtc::PeerConnectionObserver {
public:


    PeerConnectionObserverImp(std::shared_ptr<RedBack::Connection<StreamEvents>> conn)
    : PeerConnectionObserver()
    {
        _connection = conn;
    }

    void set_on_ice_candidate(std::function<void(const webrtc::IceCandidateInterface*)> callback){
        ice_candidate_callback_ = callback;
    }

    void set_on_data_channel(std::function<void(rtc::scoped_refptr<webrtc::DataChannelInterface>)> callback){
        data_channel_callback_ = callback;
    }
	
	// We won't implement this method
	virtual void OnRenegotiationNeeded() override;

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

    // Triggered when a new track is added
    void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;
    
    virtual ~PeerConnectionObserverImp() = default;
    
private:

    std::shared_ptr<RedBack::Connection<StreamEvents>> _connection;

    std::function<void(rtc::scoped_refptr<webrtc::DataChannelInterface>)> data_channel_callback_;
    std::function<void(const webrtc::IceCandidateInterface*)> ice_candidate_callback_;
};

#endif
