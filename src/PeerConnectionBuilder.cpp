#include <iostream>

#include <memory>
#include <string>
#include <utility>
#include <tuple>

#include "boost/asio.hpp"

#include "api/create_peerconnection_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/ssl_adapter.h"

#include "PeerConnectionBuilder.h"
#include "PeerConnectionObserverImp.h"
#include "CreateSessionDescriptionObserverImp.h"
#include "SetSessionDescriptionObserverImp.h"
#include "EventSocket.h"
#include "VideoTrackSourceImp.h"

namespace ip = boost::asio::ip;

template<typename T>
void PeerConnectionBuilder<T>::configure(){


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
        std::cout << "Received sdp answer: " << answer << std::endl;
#endif //REDBACK_DEBUG

        // An object to hold the error 
        webrtc::SdpParseError error;
        // Create a remote session description from the answer and set it 
        std::unique_ptr<webrtc::SessionDescriptionInterface> 
                session_description(webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, answer, &error));
        
        if (error.description != ""){
            std::cerr << "Error creating answer: " << error.description << std::endl;
            exit(EXIT_FAILURE);
        }

        this->get_peer_connection()->SetRemoteDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(),
                                session_description.release());
    });

    // Creating the peer connection stuff
    this->create_peer_connection();
    
    if (want_audio_stream())
        this->add_audio_track();
    
    if (want_video_stream())
        this->add_video_track();
    
    if (want_data_channel())
        this->add_data_channel();

    this->create_offer();
}

template<typename T>
rtc::scoped_refptr<webrtc::PeerConnectionInterface> PeerConnectionBuilder<T>::get_peer_connection(){
    return peer_connection_;
}


template<typename T>
void PeerConnectionBuilder<T>::create_peer_connection(){
    std::unique_ptr<PeerConnectionObserverImp> observer = std::make_unique<PeerConnectionObserverImp>();
    
    // Serialize and send the ice_candidate if it is gathered
    observer->set_on_ice_candidate([this](const webrtc::IceCandidateInterface* ice_candidate){

        std::string payload;
        
        ice_candidate->ToString(&payload);

        Json::Value candidate;
        Json::FastWriter writer;
        candidate["candidate"] = payload;
        candidate["sdpMid"] = ice_candidate->sdp_mid();
        candidate["sdpMLineIndex"] = ice_candidate->sdp_mline_index();

        this->get_signaling_channel().emit_event("icecandidate", writer.write(candidate));
    });

    // When the client opens a data channel, we save it
    observer->set_on_data_channel([this](rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel){
        data_channel_ = data_channel.get();
    });

    webrtc::PeerConnectionInterface::IceServer iceServer;
    
    iceServer.urls.push_back("stun:stun.stunprotocol.org");
    iceServer.urls.push_back("turn:numb.viagenie.ca");
    iceServer.username = "webrtc@live.com";
    iceServer.password = "muazkh";
    
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    config.enable_dtls_srtp = true;


    config.servers.push_back(iceServer);

    create_threads();

    // Create the peer connection factory
    // We do not need to provide the nullptr values as they are 
    // implicitly created in the API
    pcf_ = webrtc::CreatePeerConnectionFactory(
        network_thread_.get() /*network thread*/, worker_thread_.get() /*worker thread*/, signaling_thread_.get()/*signaling thread*/, nullptr /*default adm*/,
        webrtc::CreateBuiltinAudioEncoderFactory(), webrtc::CreateBuiltinAudioDecoderFactory(),
        webrtc::CreateBuiltinVideoEncoderFactory(), webrtc::CreateBuiltinVideoDecoderFactory(),
        nullptr /*audio mixer*/, nullptr /*audio_processing*/);
    
    peer_connection_ = pcf_->CreatePeerConnection(config, nullptr, nullptr, observer.release());

}

template<typename T>
void PeerConnectionBuilder<T>::add_video_track(){
    static auto videoSrc = new rtc::RefCountedObject<VideoTrackSourceImp>();
    static auto videoTrack = pcf_->CreateVideoTrack("video_label", videoSrc);

    auto ret = peer_connection_->AddTrack(videoTrack, {"stream_id"});
    assert(ret.ok());
}

template<typename T>
void PeerConnectionBuilder<T>::create_threads(){

    rtc::InitializeSSL();
    
    network_thread_ = rtc::Thread::CreateWithSocketServer();
    network_thread_->SetName("pc_network_thread", nullptr);
    network_thread_->Start();

    worker_thread_ = rtc::Thread::Create();
    worker_thread_->SetName("pc_worker_thread", nullptr);
    worker_thread_->Start();

    signaling_thread_ = rtc::Thread::Create();
    signaling_thread_->SetName("pc_signaling_thread", nullptr);
    signaling_thread_->Start();

}

template<typename T>
void PeerConnectionBuilder<T>::create_offer(){
        this->get_peer_connection()->CreateOffer(new rtc::RefCountedObject<CreateSessionDescriptionObserverImp>(this->get_peer_connection(), this->get_signaling_channel()),
                webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
}

template<typename T>
void PeerConnectionBuilder<T>::add_audio_track(){
        
        static auto audioSrc = pcf_->CreateAudioSource(cricket::AudioOptions());
        static auto audioTrack = pcf_->CreateAudioTrack("audio_track", audioSrc);
        
        auto ret = this->get_peer_connection()->AddTrack(audioTrack, {"audio_id"});        
        
        assert(ret.ok());
}

template<typename T>
void PeerConnectionBuilder<T>::add_data_channel(){
    // TODO
}

template class PeerConnectionBuilder<RedBack::EventSocket<RedBack::WebSocket<ip::tcp::socket>>>;