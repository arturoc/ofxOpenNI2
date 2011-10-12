#pragma once

#include <XnCppWrapper.h>
#include <XnCodecIDs.h>

#include "ofConstants.h"
#include "ofxOpenNI.h"

typedef struct NodeCapturingData
{
	XnCodecID captureFormat;
	XnUInt32 nCapturedFrames;
	bool bRecording;
	xn::Generator* pGenerator;
} NodeCapturingData;

#define MAX_STRINGS 20

typedef struct
{
	int nValuesCount;
	XnCodecID pValues[MAX_STRINGS];
	string pIndexToName[MAX_STRINGS];
} NodeCodec;

class ofxOpenNICapture{

public:
	ofxOpenNICapture();
	bool setup(ofxOpenNI & context, string filename, XnCodecID depthFormat=XN_CODEC_16Z_EMB_TABLES, XnCodecID imageFormat=XN_CODEC_JPEG, XnCodecID irFormat=XN_CODEC_NULL, XnCodecID audioFormat=XN_CODEC_NULL);
	bool startCapture();
	void stopCapture();

	void update();

	static string LOG_NAME;

private:

	XnStatus captureFrame();

	// --------------------------------
	// Types
	// --------------------------------
	enum CapturingState	{
		NOT_CAPTURING,
		SHOULD_CAPTURE,
		CAPTURING,
	};

	enum CaptureNodeType
	{
		CAPTURE_DEPTH_NODE,
		CAPTURE_IMAGE_NODE,
		CAPTURE_IR_NODE,
		CAPTURE_AUDIO_NODE,
		CAPTURE_NODE_COUNT
	};

	NodeCapturingData nodes[CAPTURE_NODE_COUNT];
	xn::Recorder* pRecorder;
	string csFileName;
	XnUInt32 nStartOn; // time to start, in seconds
	bool bSkipFirstFrame;
	CapturingState State;
	XnUInt32 nCapturedFrameUniqueID;
	string csDisplayMessage;

	NodeCodec g_DepthFormat;
	NodeCodec g_ImageFormat;
	NodeCodec g_IRFormat;
	NodeCodec g_AudioFormat;

	ofxOpenNI * context;

};
