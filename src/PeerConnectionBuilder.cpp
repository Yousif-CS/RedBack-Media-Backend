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

    webrtc::PeerConnectionInterface::IceServer iceServer = webrtc::PeerConnectionInterface::IceServer();
    
    iceServer.urls.push_back("stun:stun.l.google.com:19302");
    
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    
    config.servers.push_back(iceServer);
    
    auto pcf = webrtc::CreatePeerConnectionFactory(
        nullptr /*network thread*/, nullptr /*worker thread*/, nullptr/*signaling thread*/, nullptr /*default adm*/,
        webrtc::CreateBuiltinAudioEncoderFactory(), webrtc::CreateBuiltinAudioDecoderFactory(),
        webrtc::CreateBuiltinVideoEncoderFactory(), webrtc::CreateBuiltinVideoDecoderFactory(),
        nullptr /*audio mixer*/, nullptr /*audio_processing*/);

    peer_connection_ = pcf->CreatePeerConnection(config, nullptr, nullptr, observer.get());

}

template class PeerConnectionBuilder<RedBack::EventSocket<RedBack::WebSocket<ip::tcp::socket>>>;