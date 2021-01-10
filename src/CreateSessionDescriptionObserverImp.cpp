// Implementation for the CreateSessionDescriptionObserverImp class

#include <iostream>

#include "PeerConnectionHelpers.h"
#include "SetSessionDescriptionObserverImp.h"
#include "jsoncpp/json/value.h"
#include "jsoncpp/json/writer.h"

void CreateSessionDescriptionObserverImp::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{   

    std::cout << "Session Description Created!" << std::endl;
    
    //set the local description of the peer connection
    _peerConnection->SetLocalDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(_peerConnection), desc);

    //serialize and send the created session description
    std::string sdp;
    desc->ToString(&sdp);
    Json::Value sess;
    sess["sdp"] = sdp;

    Json::FastWriter writer;

    RedBack::Message<StreamEvents> msg;
    
    if (desc->GetType() == webrtc::SdpType::kOffer){
        sess["type"] = "offer";    
        msg.setID(StreamEvents::Offer);
    }
    else {
        sess["type"] = "answer";
        msg.setID(StreamEvents::Answer);

    }
    msg << writer.write(sess);

    _peerConnection->GetConnection()->send(msg);
}

void CreateSessionDescriptionObserverImp::OnFailure(webrtc::RTCError error){
    std::cerr << "Could Not Create Session Description: " << error.message() << std::endl;
}
