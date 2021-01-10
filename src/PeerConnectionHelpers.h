// A namespace of functions and classes required to setup peer connections


#ifndef REDBACK_PEER_CONNECTION_HELPERS_H
#define REDBACK_PEER_CONNECTION_HELPERS_H

// forward declare the classes
namespace RedBack {
    namespace WebRTC {
        class PeerConnection;
        class PeerConnectionFactory;
    };
};

#include "PeerConnectionCommon.h"
#include "PeerConnectionObserverImp.h"
#include "VideoTrackSourceImp.h"

#include "api/peer_connection_interface.h"
#include "api/media_stream_interface.h"
#include "api/create_peerconnection_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "rtc_base/rtc_certificate_generator.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/thread.h"

#include "jsoncpp/json/value.h"
#include "jsoncpp/json/writer.h"


// Forward declare server to be passed down to PeerConnectionObserver;
class Server;

struct CreateSessionDescriptionObserverImp: public webrtc::CreateSessionDescriptionObserver {

    CreateSessionDescriptionObserverImp(std::shared_ptr<RedBack::WebRTC::PeerConnection> pc)

    :_peerConnection(pc)
    {}
    
    virtual void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;
    virtual void OnFailure(webrtc::RTCError error) override;
    
    const std::shared_ptr<RedBack::WebRTC::PeerConnection> _peerConnection;
};

namespace RedBack
{
    namespace WebRTC
    {

        /**
         * A wrapper for a peer connection factory that handles the creation of threads
        */
        class PeerConnectionFactory: public std::enable_shared_from_this<PeerConnectionFactory> {
        public:

            PeerConnectionFactory()
            {
                initFactory();
            }

            // Delete copy constructor and assignment because we only need one factory
            // per applications
            PeerConnectionFactory(const PeerConnectionFactory&) = delete;
            PeerConnectionFactory& operator=(PeerConnectionFactory&) = delete;

            rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> GetPeerConnectionFactory()
            {
                return _peerConnectionFactory;
            }

        private:

            void initFactory();
            // We have to setup a peer connection factory
            // to create peer connections. This factory uses 4 threads to perform
            // asynchronously. 
            rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _peerConnectionFactory;
            
            // The threads used in the factory
            std::unique_ptr<rtc::Thread> _networkThread;
            std::unique_ptr<rtc::Thread> _signalingThread;
            std::unique_ptr<rtc::Thread> _workerThread;
            
        };

        /**
         * A PeerConnection Class that encapsulates the original PeerConnection
         * class and provides additional functionality
        */
        class PeerConnection: public std::enable_shared_from_this<PeerConnection>{
        public:
            /**
             * Constructor which creates a peer connection base object, 
             * and requires a connection to send and receive bootstrapping information on
            */
            PeerConnection(const rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf, 
                           std::shared_ptr<RedBack::Connection<StreamEvents>> conn);
            
            // Delete the copy constructor
            PeerConnection(const PeerConnection&) = delete;

            /**
             * Wrappers
            */
            void SetLocalDescription(webrtc::SetSessionDescriptionObserver * observer, webrtc::SessionDescriptionInterface* desc);

            void SetRemoteDescription(webrtc::SetSessionDescriptionObserver * observer, webrtc::SessionDescriptionInterface* desc);
            
            const webrtc::SessionDescriptionInterface* RemoteDescription() const;

            const webrtc::SessionDescriptionInterface* LocalDescription() const;

            /**
             * Wrapper to create an offer in the underlying base peer connection.
             * Upon completion, the CreateSessionDescriptionObserver object callbacks will be invoked
            */
            void CreateSessionDescription(webrtc::SdpType type);

            /**
             * Add a track to this current peer connection.
             * The track can be a webcam stream from this device or any other track from another stream
             * for that matter
            */
            void AddTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track);
            
            /**
             * Add an audio track to the current peer connection.
             * This audio track can be a stream from this device or any other track 
            */
            void AddAudioTrack(rtc::scoped_refptr<webrtc::AudioTrackInterface> track);

            /**
             * Wrapper that forwards adding an ice candidate
            */
            bool AddIceCandidate(const webrtc::IceCandidateInterface * candidate);
            
            /**
             * Wrapper to create an answer.
             * Upon completion, CreateSessionDescriptionObserver callbacks will be called 
            */
            std::shared_ptr<Connection<StreamEvents>> GetConnection();

            // Get the track of the current peer connection
            // Returns null if there is not track
            rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> GetTrack();
            
            // This'll allow us to share the current webcam stream coming from our device,
            // Otherwise, the program crashes if we were to create a duplicate one.
            static RedBack::VideoTrackSourceImp * GetCurrentCam();
        private:
            
            // Called by PeerConnectionObserver once an ice candidate is found 
            void OnIceCandidate(const webrtc::IceCandidateInterface* ice_candidate);

            std::unique_ptr<PeerConnectionObserverImp> _observer = nullptr;
            rtc::scoped_refptr<webrtc::PeerConnectionInterface> _peerConnection = nullptr;
            std::shared_ptr<RedBack::Connection<StreamEvents>> _connection = nullptr;
        };
        // This is the end of the PeerConnection class

    };
};

#endif
