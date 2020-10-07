// Implementation for the CreateSessionDescriptionObserverImp class

#include <iostream>

#include "CreateSessionDescriptionObserverImp.h"
#include "SetSessionDescriptionObserverImp.h"

void CreateSessionDescriptionObserverImp::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{   

    std::cout << "Answer created!" << std::endl;
    
    //set the local description of the peer connection
    peer_connection_->SetLocalDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(), desc);

    //serialize and send the created offer
    std::string sdp;
    desc->ToString(&sdp);
    signaling_channel_.emit_event("sdp_answer", sdp);
}

void CreateSessionDescriptionObserverImp::OnFailure(webrtc::RTCError error){
    std::cerr << "Could Not Create Session Description: " << error.message() << std::endl;
}
