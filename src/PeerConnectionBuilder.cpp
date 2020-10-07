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

#include "PeerConnectionBuilder.h"
#include "PeerConnectionObserverImp.h"
#include "CreateSessionDescriptionObserverImp.h"
#include "SetSessionDescriptionObserverImp.h"
#include "EventSocket.h"
#include "VideoTrackSourceImp.h"

namespace ip = boost::asio::ip;

// A class that contains tasks (or in other words methods)
// to be invoked (posted) on the signaling thread;
template<typename ... T>
class QueuedTask: public webrtc::QueuedTask {

    using bind_type = decltype(std::bind(std::declval<std::function<bool(T...)>>(), std::declval<T>()...));
public:
    QueuedTask(std::function<bool(T...)> task, T... t)
    :bind_(task, std::forward<T>(t)...)
    {}
    
    bool Run() override {
        return bind_();
    }

private:
    bind_type bind_;
};

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

    t_.on_event("sdp_offer", [this](std::string offer){

#ifdef REDBACK_DEBUG
        std::cout << "Received sdp offer: " << offer << std::endl;
#endif //REDBACK_DEBUG
        
        // Creating the peer connection stuff
        this->create_peer_connection();
        
        // Add a video track to it (which currently is just the webcam on the laptop)
        //this->add_video_track();

        // An object to hold the error 
        webrtc::SdpParseError error;
        // Create a remote session description from the answer and set it 
        std::unique_ptr<webrtc::SessionDescriptionInterface> 
                session_description(webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, offer, &error));
        
        if (error.description != ""){
            std::cerr << "Error creating offer: " << error.description << std::endl;
            exit(EXIT_FAILURE);
        }
        // this->peer_connection_->signaling_thread()->Invoke(RTC_FROM_HERE,
        //     std::make_unique<QueuedTask<webrtc::SessionDescriptionInterface *>>
        //                 ([this](webrtc::SessionDescriptionInterface * desc)-> bool
        //                             {
        //                                 this->peer_connection_->SetRemoteDescription(
        //                                     new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(), desc);
        //                                 return true;
        //                             },
        //                             session_description.release()));
        this->get_peer_connection()->signaling_thread()->template Invoke<void>(RTC_FROM_HERE,
            [this, &session_description]()-> void {
                this->get_peer_connection()->SetRemoteDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(),
                                session_description.release());
            });
        // this->peer_connection_->SetRemoteDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(),
        //                                              session_description.release());
        this->add_video_track();
        //create an answer and send it 
        // this->peer_connection_->signaling_thread()->Invoke<bool>(RTC_FROM_HERE,
        //     std::make_unique<QueuedTask<>>([this]() -> bool{
        //         this->peer_connection_->CreateAnswer(new rtc::RefCountedObject<CreateSessionDescriptionObserverImp>(this->peer_connection_, this->t_),
        //                     webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
        //         return true;
        //     })
        // );
        this->get_peer_connection()->signaling_thread()->template Invoke<void>(RTC_FROM_HERE,
            [this]()-> void {
                this->get_peer_connection()->CreateAnswer(new rtc::RefCountedObject<CreateSessionDescriptionObserverImp>(this->get_peer_connection(), this->get_signaling_channel()),
                        webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
            });
        // this->peer_connection_->CreateAnswer(new rtc::RefCountedObject<CreateSessionDescriptionObserverImp>(this->peer_connection_, this->t_),
        //         webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());

    });
}

template<typename T>
rtc::scoped_refptr<webrtc::PeerConnectionInterface> PeerConnectionBuilder<T>::get_peer_connection(){
    // while(peer_connection_ != nullptr && peer_connection_->peer_connection_state() != webrtc::PeerConnectionInterface::PeerConnectionState::kConnected){
    //     // keep waiting until the peer connection is ready
    // }
    return peer_connection_;
}

template<typename T>
void PeerConnectionBuilder<T>::create_peer_connection(){
    std::unique_ptr<PeerConnectionObserverImp> observer = std::make_unique<PeerConnectionObserverImp>();
    
    // Serialize and send the ice_candidate if it is gathered
    observer->set_on_ice_candidate([this](const webrtc::IceCandidateInterface* ice_candidate){

        std::string payload;
        
        ice_candidate->ToString(&payload);
        this->get_signaling_channel().emit_event("icecandidate", payload);
    });

    // When the client opens a data channel, we save it
    observer->set_on_data_channel([this](rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel){
        data_channel_ = data_channel.get();
    });

    webrtc::PeerConnectionInterface::IceServer iceServer;
    
    iceServer.urls.push_back("stun:stun.l.google.com:19302");
    
    webrtc::PeerConnectionInterface::RTCConfiguration config;
    // config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    // config.enable_dtls_srtp = true;


    config.servers.push_back(iceServer);

    // Create the peer connection factory
    // We do not need to provide the nullptr values as they are 
    // implicitly created in the API
    pcf_ = webrtc::CreatePeerConnectionFactory(
        nullptr /*network thread*/, nullptr /*worker thread*/, nullptr/*signaling thread*/, nullptr /*default adm*/,
        webrtc::CreateBuiltinAudioEncoderFactory(), webrtc::CreateBuiltinAudioDecoderFactory(),
        webrtc::CreateBuiltinVideoEncoderFactory(), webrtc::CreateBuiltinVideoDecoderFactory(),
        nullptr /*audio mixer*/, nullptr /*audio_processing*/);


    peer_connection_ = pcf_->CreatePeerConnection(config, nullptr, nullptr, observer.release());

}

template<typename T>
void PeerConnectionBuilder<T>::add_video_track(){
    auto videoSrc = new rtc::RefCountedObject<VideoTrackSourceImp>();
    auto videoTrack = pcf_->CreateVideoTrack("camera test", videoSrc);

    auto ret = peer_connection_->AddTrack(videoTrack, {"web camera test"});
    assert(ret.ok());
}

template class PeerConnectionBuilder<RedBack::EventSocket<RedBack::WebSocket<ip::tcp::socket>>>;