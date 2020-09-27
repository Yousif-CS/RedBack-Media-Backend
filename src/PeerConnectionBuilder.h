// A builder class that builds a WebRTC peer connection
#include <memory>

#include "api/peer_connection_interface.h"

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
    std::shared_ptr<webrtc::PeerConnectionInterface> get_peer_connection();

    std::shared_ptr<PeerConnectionBuilder<T>> getPtr(){
        return this->shared_from_this();
    }

private:
    
    //setup event listeners and establish the connection
    void configure();

    rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
    T t_;
};
