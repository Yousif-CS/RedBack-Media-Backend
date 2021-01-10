#ifndef CLIENT_H_
#define CLIENT_H_

#include <iostream>
#include <string>
#include <functional>
#include <chrono>
#include <thread>

#include "PeerConnectionCommon.h"
#include "PeerConnectionHelpers.h"
#include "SetSessionDescriptionObserverImp.h"

#include "eventsocketcpp/client/EventClientInterface.h"

#include "jsoncpp/json/value.h"
#include "jsoncpp/json/reader.h"

class Client: public RedBack::Client::EventClientInterface<StreamEvents> 
{
public:

    using EventClientInterface<StreamEvents>::EventClientInterface;
    
    // We want to start setting up event handlers then start the peer connection process
    void OnConnect()
    {
        OnEvent(StreamEvents::Answer, [this](RedBack::Message<StreamEvents> msg){
            // Store that offer and send response
            std::string answer; msg >> answer;

            // JSON parse it
            Json::Value root;
            Json::Reader reader;

            // mangled offer
            if (!reader.parse(answer.c_str(), root))
            {
                std::cerr << "Error parsing answer." << std::endl;
                disconnect();
                return;           
            }

            webrtc::SdpParseError error;

			// Create SessionDescription
			std::unique_ptr<webrtc::SessionDescriptionInterface>
				sess(webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, root["sdp"].asString(), &error));
			
			// Check for errors in parsing
			if (error.description != ""){
            	std::cerr << "Error creating answer: " << error.description << std::endl;
				disconnect();
				return;
			}
            
            // Store it as a remote answer. The callback of SetSessionDescriptionObserverImp will 
            // not do anything else as both the remote and local descriptions will have been set.
            _peerConnection->SetRemoteDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(_peerConnection), sess.release());
            
        });

        OnEvent(StreamEvents::StreamRequestAck, [this](RedBack::Message<StreamEvents> msg){

            // We setup stuff here and send an offer
            // Setup the peer connection
            _peerConnection = std::make_shared<RedBack::WebRTC::PeerConnection>
                                (_peerConnectionFactory.GetPeerConnectionFactory(), GetConnection());

            // Add the current video feed
            RedBack::VideoTrackSourceImp * source = RedBack::WebRTC::PeerConnection::GetCurrentCam();
            rtc::scoped_refptr<webrtc::VideoTrackInterface> track = 
                                    _peerConnectionFactory.GetPeerConnectionFactory()->CreateVideoTrack("stream", source);

            _peerConnection->AddTrack(track);

            // Create and send the offer 
            _peerConnection->CreateSessionDescription(webrtc::SdpType::kOffer);

        });

        OnEvent(StreamEvents::IceCandidate, [this](RedBack::Message<StreamEvents> msg) {
            // Getting an Ice candidate from the other end, so we need to set it here.
			Json::Value root;
			Json::Reader reader;

			std::string candidate; msg >> candidate;

			if (!reader.parse(candidate.c_str(), root))
			{
				std::cerr << "Error Parsing Ice Candidate" << std::endl;
                disconnect();
				return;
			}

			// Create the ice candidate from the candidate string
	        webrtc::IceCandidateInterface * ice_candidate = 
                webrtc::CreateIceCandidate(root["id"].asString(), root["label"].asInt(), root["candidate"].asString(), nullptr);

			// Add ice candidate to peer connection and error check
        	if (!_peerConnection->AddIceCandidate(ice_candidate))
			{
            	std::cerr << "Could not add ice candidate to peer connection" << std::endl;
                disconnect();
				return;
        	}

        });
        
        // Emit a stream event to server 
        RedBack::Message<StreamEvents> msg;
        msg.setID(StreamEvents::Stream);
        send(msg);
        
    };

    void OnDisconnect()
    {
        // Since this is running on the car, we wait 5 seconds to try and reconnect 
        std::chrono::seconds duration(5);
        std::this_thread::sleep_for(duration);
        connect(host, port);
    }

private:
    RedBack::WebRTC::PeerConnectionFactory _peerConnectionFactory;
    std::shared_ptr<RedBack::WebRTC::PeerConnection> _peerConnection; 
};
#endif