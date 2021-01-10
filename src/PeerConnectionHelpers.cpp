#include "PeerConnectionHelpers.h"
#include "PeerConnectionObserverImp.h"
#include "server.h"

namespace RedBack
{
    namespace WebRTC
    {
        void PeerConnectionFactory::initFactory()
        {
            // Initialize ssl and threads
            rtc::InitializeSSL();

            _networkThread = rtc::Thread::CreateWithSocketServer();
            _networkThread->SetName("pc_network_thread", nullptr);
            _networkThread->Start();
            
            _workerThread = rtc::Thread::Create();
            _workerThread->SetName("pc_worker_thread", nullptr);
            _workerThread->Start();

            _signalingThread = rtc::Thread::Create();
            _signalingThread->SetName("pc_signaling_thread", nullptr);
            _signalingThread->Start();

            // Create the factory
            _peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
                _networkThread.get(), _workerThread.get(), 
                _signalingThread.get(), nullptr /* default adm */,
                webrtc::CreateBuiltinAudioEncoderFactory(), 
                webrtc::CreateBuiltinAudioDecoderFactory(),
                webrtc::CreateBuiltinVideoEncoderFactory(),
                webrtc::CreateBuiltinVideoDecoderFactory(),
                nullptr /* audio mixer */, nullptr /* audio processing */
            );
        }

        
        /**
         * A PeerConnection Class that encapsulates the original PeerConnection
         * class and provides additional functionality
        */

        /**
         * Constructor which creates a peer connection base object, 
         * and requires a connection to send and receive bootstrapping information on
        */
        PeerConnection::PeerConnection(const rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf, 
                                       std::shared_ptr<RedBack::Connection<StreamEvents>> conn)
        : _connection(conn)
        {
            // create an observer and use it to create a peer connection
            _observer = std::make_unique<PeerConnectionObserverImp>(conn);

            _observer->set_on_ice_candidate([this](auto icecandidate) {
                //TODO: Make sure we are allowed to use `auto` like that
                this->OnIceCandidate(icecandidate);
            });
            
            // Setup ice server discovery and configurations
            webrtc::PeerConnectionInterface::IceServer iceServer;

            iceServer.urls.push_back("stun:stun.stunprotocol.org");
            iceServer.urls.push_back("turn:numb.viagenie.ca");
            iceServer.username = "webrtc@live.com";
            iceServer.password = "muazkh";
            
            webrtc::PeerConnectionInterface::RTCConfiguration config;
            config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
            config.enable_dtls_srtp = true;
            config.servers.push_back(iceServer);

            _peerConnection = pcf->CreatePeerConnection(config, nullptr, nullptr, _observer.get());
        }

        /**
         * Wrappers
        */
        void PeerConnection::SetLocalDescription(webrtc::SetSessionDescriptionObserver * observer, webrtc::SessionDescriptionInterface* desc)
        {
            _peerConnection->SetLocalDescription(observer, desc);
        }

        void PeerConnection::SetRemoteDescription(webrtc::SetSessionDescriptionObserver * observer, webrtc::SessionDescriptionInterface* desc)
        {
            _peerConnection->SetRemoteDescription(observer, desc);
        }
        
        const webrtc::SessionDescriptionInterface * PeerConnection::RemoteDescription() const
        {
            return _peerConnection->remote_description();
        }

        const webrtc::SessionDescriptionInterface * PeerConnection::LocalDescription() const
        {
            return _peerConnection->local_description();
        }


        /**
         * Wrapper to create an offer in the underlying base peer connection.
         * Upon completion, the CreateSessionDescriptionObserver object callbacks will be invoked
        */
        void PeerConnection::CreateSessionDescription(webrtc::SdpType type)
        {
            if (type == webrtc::SdpType::kAnswer)
                _peerConnection->CreateAnswer(new rtc::RefCountedObject<CreateSessionDescriptionObserverImp>(shared_from_this()),
                                            webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
            else if (type == webrtc::SdpType::kOffer)
                    _peerConnection->CreateOffer(new rtc::RefCountedObject<CreateSessionDescriptionObserverImp>(shared_from_this()),
                                            webrtc::PeerConnectionInterface::RTCOfferAnswerOptions());
        }

        /**
         * Add a track to this current peer connection.
         * The track can be a webcam stream from this device or any other track from another stream
         * for that matter
        */
        void PeerConnection::AddTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track)
        {
            auto ret = _peerConnection->AddTrack(track, {"stream_id"});
            
            // Throw an exception so that maybe we get rid of this object
            if (!ret.ok())
            {
                throw std::runtime_error("Could not add track");
            }
        }
        
        /**
         * Add an audio track to the current peer connection.
         * This audio track can be a stream from this device or any other track 
        */
        void PeerConnection::AddAudioTrack(rtc::scoped_refptr<webrtc::AudioTrackInterface> track)
        {
            auto ret = _peerConnection->AddTrack(track, {"audio_id"});

            // Throw an exception if it fails
            if (!ret.ok())
            {
                throw std::runtime_error("Could not add audio track");
            }
        }

        /**
         * Wrapper that forwards adding an ice candidate
        */
        bool PeerConnection::AddIceCandidate(const webrtc::IceCandidateInterface * candidate)
        {
            return _peerConnection->AddIceCandidate(candidate);
        }
        
        /**
         * Wrapper to create an answer.
         * Upon completion, CreateSessionDescriptionObserver callbacks will be called 
        */
        std::shared_ptr<Connection<StreamEvents>> PeerConnection::GetConnection()
        {
            return _connection->shared_from_this();
        }

        // Get the track of the current peer connection
        // Returns null if there is not track
        rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> PeerConnection::GetTrack()
        {
            // For now we assume there is only one track
            if (_peerConnection->GetTransceivers().size() > 0)
                return _peerConnection->GetTransceivers()[0]->receiver()->track();
            return nullptr;
        }

        // This'll allow us to share the current webcam stream coming from our device,
        // Otherwise, the program crashes if we were to create a duplicate one.
        RedBack::VideoTrackSourceImp * PeerConnection::GetCurrentCam()
        {
            static auto videoSrc = new rtc::RefCountedObject<RedBack::VideoTrackSourceImp>();
            return videoSrc;
        }

        // Called by PeerConnectionObserver once an ice candidate is found 
        void PeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* ice_candidate)
        {
            // // Only when we have both local and remote descriptions do we set it
            // if (!LocalDescription() || !RemoteDescription())
            //     return;
            

            // // set the ice candidate on the local peer connection
            // if (!AddIceCandidate(ice_candidate))
            // {
            //     std::cerr << "Error Adding Ice Candidate To The Peer Connection." << std::endl;
            //     return;
            // }

            // Prepare the ice candidate to be transmitted to the other end
            std::string icecandidateStr;
            
            ice_candidate->ToString(&icecandidateStr);

            Json::Value candidate;
            Json::FastWriter writer;
            candidate["candidate"] = icecandidateStr;
            candidate["sdpMid"] = ice_candidate->sdp_mid();
            candidate["sdpMLineIndex"] = ice_candidate->sdp_mline_index();

            // Make a message to send 
            RedBack::Message<StreamEvents> msg;
            msg.setID(StreamEvents::IceCandidate);
            msg << writer.write(candidate);


            _connection->send(msg);
        }
        // This is the end of the PeerConnection class
    };
};
