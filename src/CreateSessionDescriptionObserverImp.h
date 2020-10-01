// A callback class that upon creating an sdp offer, serializes it and sends it 

#ifndef CREATE_SESSION_DESCRIPTION_OBSERVER_IMP_H_
#define CREATE_SESSION_DESCRIPTION_OBSERVER_IMP_H_

#include <memory>

#include "api/peer_connection_interface.h"
#include "boost/asio.hpp"
#include "boost/beast/websocket.hpp"

#include "EventSocket.h"
#include "CustomWebSocket.h"

namespace ip = boost::asio::ip;

class CreateSessionDescriptionObserverImp: public webrtc::CreateSessionDescriptionObserver {

public:
    CreateSessionDescriptionObserverImp(rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection,
        RedBack::EventSocket<RedBack::WebSocket<ip::tcp::socket>>& signaling_channel)
        :peer_connection_(peer_connection), signaling_channel_(signaling_channel)
        {
        }
    
    void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;
    void OnFailure(webrtc::RTCError error) override;

private:
    
    RedBack::EventSocket<RedBack::WebSocket<ip::tcp::socket>>& signaling_channel_;
    const rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
};

#endif