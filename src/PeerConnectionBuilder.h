// A builder class that builds a WebRTC peer connection
#include <memory>

#include "api/peer_connection_interface.h"
#include "rtc_base/thread.h"
#include "EventSocket.h"
#include "CustomWebSocket.h"


template<typename T>
class PeerConnectionBuilder: std::enable_shared_from_this<PeerConnectionBuilder<T>>{

public:

    PeerConnectionBuilder(T& t)
        :t_(t)
    {
        configure();
    }

    // Returns the newly created peer connection; if it is available
    // otherwise; blocks until it is ready
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> get_peer_connection();

    // Get the signaling channel
    T& get_signaling_channel() { return t_; };

    std::shared_ptr<PeerConnectionBuilder<T>> getPtr(){
        return this->shared_from_this();
    }

private:
    
    // Setup event listeners and establish the connection
    void configure();
    // The steps required in setting up a connection (optional step of adding video track)
    // Create the threads required for the various connection establishment and maintenance
    // operations.
    void create_threads();
    void create_peer_connection();
    void add_video_track();
    void create_offer();
    
    std::unique_ptr<rtc::Thread> network_thread_;
    std::unique_ptr<rtc::Thread> signaling_thread_;
    std::unique_ptr<rtc::Thread> worker_thread_;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> pcf_ = nullptr;
    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_ = nullptr;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_ = nullptr;
    T& t_;
};
