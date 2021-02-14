//A video source sink that contains a video capturer (from a webcam)

#ifndef REDBACK_VIDEO_TRACK_SOURCE_IMP_H
#define REDBACK_VIDEO_TRACK_SOURCE_IMP_H

#include "api/peer_connection_interface.h"
#include "api/video/video_sink_interface.h"
#include "api/video/video_source_interface.h"
#include "api/media_stream_interface.h"
#include "pc/video_track_source.h"

#include "VideoCapturer.h"
namespace RedBack {

		struct VideoTrackSourceImp: public webrtc::VideoTrackSource
		{
			VideoCapturer _videoCapturer;
			
			VideoTrackSourceImp()
			:webrtc::VideoTrackSource(false)
			{
			}
			
			rtc::VideoSourceInterface<webrtc::VideoFrame> * source() override {
				return &_videoCapturer;
			}
		};
};
#endif
