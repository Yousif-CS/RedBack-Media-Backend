// A Server that processes connection requests

#ifndef SERVER_H_
#define SERVER_H_

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <utility>

#include "PeerConnectionCommon.h"
#include "PeerConnectionHelpers.h"
#include "SetSessionDescriptionObserverImp.h"

#include "eventsocketcpp/server/EventServerInterface.h"

#include "jsoncpp/json/value.h"
#include "jsoncpp/json/reader.h"

class Server: public RedBack::Server::EventServerInterface<StreamEvents>, std::enable_shared_from_this<Server>{
public:
	
    using EventServerInterface<StreamEvents>::EventServerInterface;

    /**
     * process every connection request either to stream, or watch
    */
    virtual bool OnConnect(std::shared_ptr<RedBack::Connection<StreamEvents>> conn)
    {
        // Setup event handlers, this is the main bulk logic
        // of the peer connection establishment process
        
        // We check whether there are broadcasters to watch
        OnEvent(StreamEvents::Watch, conn, [this, conn](RedBack::Message<StreamEvents> msg){
            if (this->_streamAvailable)
            {
                // We start the procedure of establishing a connection
				// Providing it with the video stream of the streamer
				_watchers[conn->GetID()] = std::make_shared<RedBack::WebRTC::PeerConnection>
												(_peerConnectionFactory.GetPeerConnectionFactory(), conn);

				_watchers[conn->GetID()]->AddTrack(_streamer.second->GetTrack());
				_watchers[conn->GetID()]->CreateSessionDescription(webrtc::SdpType::kOffer);
		    }
			else {
				// We send a no stream available event
				RedBack::Message<StreamEvents> msg{};
				msg.setConfig(RedBack::Config::None);
				msg.setID(StreamEvents::NoStreamAvailable);
				conn->send(msg);
			}
        });

		OnEvent(StreamEvents::Offer, conn, [this, conn](RedBack::Message<StreamEvents> msg){
			// Parse the message
			webrtc::SdpParseError error;
			std::string offer; msg >> offer;

			// JSON Parse the offer
			Json::Value root;
			Json::Reader reader;

			// Error check
			if (!reader.parse(offer.c_str(), root))
			{
				std::cerr << "Error parsing offer sent by " << "[" << conn->GetID() << "]" << std::endl;
				removeConnection(conn);
				return; 
			}

			// Create SessionDescription
			std::unique_ptr<webrtc::SessionDescriptionInterface>
				sess(webrtc::CreateSessionDescription(webrtc::SdpType::kOffer, root["sdp"].asString(), &error));
			
			// Check for errors in parsing
			if (error.description != ""){
            	std::cerr << "Error creating answer: " << error.description << std::endl;
				removeConnection(conn);
				return;
			}

			// Figure out whose offer is this
			std::shared_ptr<RedBack::WebRTC::PeerConnection> curr;
			if (_streamer.first == conn->GetID())
				curr = _streamer.second;
			else
				curr = _watchers[conn->GetID()];
			
			// Set the remote description
			curr->SetRemoteDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(curr),
														   sess.release());
		});

		OnEvent(StreamEvents::Stream, conn, [this, conn](RedBack::Message<StreamEvents> msg){
			if (this->_streamAvailable)
			{
				// Just refuse the stream for now. Later updates we'll
				// allow multiple streams especially if we have multiple cars that stream
				RedBack::Message<StreamEvents> msg;
				msg.setID(StreamEvents::StreamAlreadyAvailable);
				conn->send(msg);

			}else 
			{
				// Here we perform the connection with the streamer (car client)
				_streamer.first = conn->GetID();
				_streamer.second = 
					std::make_shared<RedBack::WebRTC::PeerConnection>(_peerConnectionFactory.GetPeerConnectionFactory(), conn);

				// Send back an acknowledgement so they can send us an offer
				RedBack::Message<StreamEvents> msg;
				msg.setID(StreamEvents::StreamRequestAck);
				conn->send(msg);
				this->_streamAvailable = true;
			}

		});

		OnEvent(StreamEvents::Answer, conn, [this, conn](RedBack::Message<StreamEvents> msg){

			webrtc::SdpParseError error;
			std::string answer;
			msg >> answer;

			// JSON parse the answer
			Json::Value root;
			Json::Reader reader;

			// Error check
			if (!reader.parse(answer.c_str(), root))
			{
				std::cerr << "Error parsing answer sent by" << " [" << conn->GetID()<< "]" << std::endl;
				removeConnection(conn);
				return;
			}

			// Create SessionDescription
			std::unique_ptr<webrtc::SessionDescriptionInterface>
				sess(webrtc::CreateSessionDescription(webrtc::SdpType::kAnswer, root["sdp"].asString(), &error));
			
			// Check for errors in parsing
			if (error.description != ""){
            	std::cerr << "Error creating answer: " << error.description << std::endl;
				removeConnection(conn);
				return;
			}

			// Figure out whose answer is this
			std::shared_ptr<RedBack::WebRTC::PeerConnection> curr;
			if (_streamer.first == conn->GetID())
				curr = _streamer.second;
			else
				curr = _watchers[conn->GetID()];
			
			// Set the remote description
			curr->SetRemoteDescription(new rtc::RefCountedObject<SetSessionDescriptionObserverImp>(curr),
														   sess.release());
		});

		OnEvent(StreamEvents::IceCandidate, conn, [this, conn](RedBack::Message<StreamEvents> msg){
			// Getting an Ice candidate from the other end, so we need to set it here.
			Json::Value root;
			Json::Reader reader;

			std::string candidate; msg >> candidate;

			if (!reader.parse(candidate.c_str(), root))
			{
				std::cerr << "Error Parsing Ice Candidate for [" << conn->GetID() << "]" << std::endl;
				removeConnection(conn);
				return;
			}

			// Create the ice candidate from the candidate string
	        webrtc::IceCandidateInterface * ice_candidate = 
                webrtc::CreateIceCandidate(root["id"].asString(), root["label"].asInt(), root["candidate"].asString(), nullptr);


			// Figure out whose ice candidate is this
			std::shared_ptr<RedBack::WebRTC::PeerConnection> curr;
			if (_streamer.first == conn->GetID())
				curr = _streamer.second;
			else
				curr = _watchers[conn->GetID()];
			
			// Add ice candidate to peer connection and error check
        	if (!curr->AddIceCandidate(ice_candidate))
			{
            	std::cerr << "Could not add ice candidate to peer connection of [" << conn->GetID() << "]" << std::endl;
				removeConnection(conn);
				return;
        	}

		});

    	return true;
	}

	bool OnDisconnect(std::shared_ptr<RedBack::Connection<StreamEvents>> conn) override 
	{
		// Make sure we remove the associated webrtc connection
		removeConnection(conn);
	}

private:

	// Remove the connection, whether it is a streamer or watcher
	void removeConnection(std::shared_ptr<RedBack::Connection<StreamEvents>> conn)
	{
		// Close the connection 
		if (_streamer.first == conn->GetID())
		{
			removeStreamer(conn);
		}
		else
		{
			removeWatcher(conn);
		}
	}

	// If something fails, remove the streamer 
	void removeStreamer(std::shared_ptr<RedBack::Connection<StreamEvents>> conn)
	{
		_streamer.first = -1;
		_streamer.second.reset();
		_streamAvailable = false;
		conn->disconnect();
	}
	
	// If something fails, remove the watcher
	void removeWatcher(std::shared_ptr<RedBack::Connection<StreamEvents>> conn)
	{
		_watchers.erase(conn->GetID());
		conn->disconnect();
	}

    // This holds the possible streams from the different cars
    // For now, we can only stream from one car 
	RedBack::WebRTC::PeerConnectionFactory _peerConnectionFactory;

    std::pair<uint32_t, std::shared_ptr<RedBack::WebRTC::PeerConnection>> _streamer;

	std::map<uint32_t, std::shared_ptr<RedBack::WebRTC::PeerConnection>> _watchers;

    // To refuse watching requests if there are not streams available
    bool _streamAvailable = false;
};

#endif