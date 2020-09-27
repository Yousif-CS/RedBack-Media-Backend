// A builder class that builds a WebRTC peer connection

#include "EventSocket.h"
#include "CustomWebSocket.h"
#include "api/peer_connection_interface.h"

template<typename T>
class PeerConnectionBuilder: std::enable_shared_from_this<PeerConnectionBuilder>{

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
        return shared_from_this();
    }

private:
    
    //setup event listeners and establish the connection
    void configure();
    T t_;
};
