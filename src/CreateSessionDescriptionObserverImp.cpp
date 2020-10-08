// Implementation for the CreateSessionDescriptionObserverImp class

#include <iostream>

#include "CreateSessionDescriptionObserverImp.h"
#include "SetSessionDescriptionObserverImp.h"
#include "jsoncpp/json/value.h"

void CreateSessionDescriptionObserverImp::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{   

    std::cout << "Offer created!" << std::endl;
    
    //set the local description of the peer connection
    peer_connection_->SetLocalDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(), desc);

    //serialize and send the created offer
    std::string sdp;
    desc->ToString(&sdp);
    Json::Value answer;
    answer["type"] = "offer";
    answer["sdp"] = sdp;
    Json::FastWriter writer;
    std::cout << writer.write(answer) << std::endl;
    signaling_channel_.emit_event("sdp_offer", writer.write(answer));
}

void CreateSessionDescriptionObserverImp::OnFailure(webrtc::RTCError error){
    std::cerr << "Could Not Create Session Description: " << error.message() << std::endl;
}
