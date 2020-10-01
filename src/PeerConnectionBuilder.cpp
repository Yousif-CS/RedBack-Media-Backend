#include <iostream>

#include <memory>
#include <string>

#include "boost/asio.hpp"

#include "api/create_peerconnection_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"


#include "PeerConnectionBuilder.h"
#include "PeerConnectionObserverImp.h"
#include "CreateSessionDescriptionObserverImp.h"
#include "SetSessionDescriptionObserverImp.h"
#include "EventSocket.h"

namespace ip = boost::asio::ip;

template<typename T>
void PeerConnectionBuilder<T>::configure(){

    std::unique_ptr<PeerConnectionObserverImp> observer = std::make_unique<PeerConnectionObserverImp>();
    
    // Serialize and send the ice_candidate if it is gathered
    observer->set_on_ice_candidate([this](const webrtc::IceCandidateInterface* ice_candidate){

        std::string payload;
        
        ice_candidate->ToString(&payload);
        t_.emit_event("icecandidate", payload);
    });

    // When the client opens a data channel, we save it
    observer->set_on_data_channel([this](rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel){
        data_channel_ = data_channel.get();
    });

    webrtc::PeerConnectionInterface::IceServer iceServer;
    
    iceServer.urls.push_back("stun:stun.l.google.com:19302");
    
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    
    config.servers.push_back(iceServer);
    
    // Create the peer connection factory
    // We do not need to provide the nullptr values as they are 
    // implicitly created in the API
    auto pcf = webrtc::CreatePeerConnectionFactory(
        nullptr /*network thread*/, nullptr /*worker thread*/, nullptr/*signaling thread*/, nullptr /*default adm*/,
        webrtc::CreateBuiltinAudioEncoderFactory(), webrtc::CreateBuiltinAudioDecoderFactory(),
        webrtc::CreateBuiltinVideoEncoderFactory(), webrtc::CreateBuiltinVideoDecoderFactory(),
        nullptr /*audio mixer*/, nullptr /*audio_processing*/);


    peer_connection_ = pcf->CreatePeerConnection(config, nullptr, nullptr, observer.release());
    
    auto media_stream_track = pcf->CreateVideoTrack("video capture", );


    // Add the ice candidate to the peer connection
    t_.on_event("icecandidate", [this](std::string candidate) {

#ifdef REDBACK_DEBUG
        std::cout << "Received ice candidate:" << candidate << std::endl;
#endif //REDBACK_DEBUG

        Json::Value root;
        Json::Reader reader;

        if (!reader.parse(candidate.c_str(), root)){
            throw std::runtime_error("Could not parse ice candidate!");
        }
    
        // Create the ice candidate from the candidate string
        webrtc::IceCandidateInterface * ice_candidate = 
                webrtc::CreateIceCandidate(root["id"].asString(), root["label"].asInt(), root["candidate"].asString(), nullptr);

        // Add ice candidate to peer connection and error check
        if (!this->peer_connection_->AddIceCandidate(ice_candidate)){
            throw std::runtime_error("Could not add ice candidate to peer connection!");
        }
    });

    t_.on_event("sdp_answer", [this](std::string answer){

#ifdef REDBACK_DEBUG
        std::cout << "Received sdp offer: " << answer << std::endl;
#endif //REDBACK_DEBUG
        
        //Create a remote session description from the answer and set it 
        std::unique_ptr<webrtc::SessionDescriptionInterface> session_description
            = webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, answer);
        
        this->peer_connection_->SetRemoteDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(),
                                                     session_description.release());
    });

}
template<typename T>
rtc::scoped_refptr<webrtc::PeerConnectionInterface> PeerConnectionBuilder<T>::get_peer_connection(){

    while(peer_connection_->peer_connection_state() != webrtc::PeerConnectionInterface::PeerConnectionState::kConnected){
        // keep waiting until the peer connection is ready
    }
    return peer_connection_;
}

template class PeerConnectionBuilder<RedBack::EventSocket<RedBack::WebSocket<ip::tcp::socket>>>;