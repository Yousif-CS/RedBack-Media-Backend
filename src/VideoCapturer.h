// A class used to capture the video from the webcam


#pragma once

#ifndef REDBACK_VIDEO_CAPTURER_H_
#define REDBACK_VIDEO_CAPTURER_H_

#include "media/base/video_broadcaster.h"
#include "media/base/video_adapter.h"
#include "modules/video_capture/video_capture_factory.h"
#include "modules/video_capture/device_info_impl.h"


namespace RedBack {
		class VideoCapturer: public rtc::VideoSourceInterface<webrtc::VideoFrame>, public rtc::VideoSinkInterface<webrtc::VideoFrame>
		{

		public:

			rtc::scoped_refptr<webrtc::VideoCaptureModule> video_capture_module;

			VideoCapturer(){
				std::unique_ptr<webrtc::VideoCaptureModule::DeviceInfo> device_info(
					webrtc::VideoCaptureFactory::CreateDeviceInfo());
				
				// Get the id of the webcam
				char name[2048];
				char uniqueID[2048];
				device_info->GetDeviceName(0, name, 2048, uniqueID, 2048, nullptr, 0);

				webrtc::VideoCaptureCapability capability;
				device_info->GetCapability(uniqueID, 0, capability);

				// set up camera and start capturing
				video_capture_module = webrtc::VideoCaptureFactory::Create(uniqueID);
				video_capture_module->RegisterCaptureDataCallback(this);
				video_capture_module->StartCapture(capability);
				video_capture_module->CaptureStarted();
			}

			// Adds a video sink to the current video track. Used to connect the track to the underlying
			// video engine.
			void AddOrUpdateSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink, const rtc::VideoSinkWants &wants) override {
				broadcaster_.AddOrUpdateSink(sink, wants);
				UpdateVideoAdapter();
			}


			void RemoveSink(rtc::VideoSinkInterface<webrtc::VideoFrame> *sink) override {
				broadcaster_.RemoveSink(sink);
				UpdateVideoAdapter();
			}

			~VideoCapturer() {
				video_capture_module->StopCapture();
				video_capture_module->DeRegisterCaptureDataCallback();
			}

		protected:
			void OnFrame(const webrtc::VideoFrame &frame) override {
				broadcaster_.OnFrame(frame);
			}

			rtc::VideoSinkWants GetSinkWants(){
				return broadcaster_.wants();
			}

		private:
			void UpdateVideoAdapter(){
				video_adapter_.OnSinkWants(broadcaster_.wants());
			}
			
			rtc::VideoBroadcaster broadcaster_;
			cricket::VideoAdapter video_adapter_;
		};






};
#endif
