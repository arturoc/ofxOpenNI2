/*
 * ofxOpenNICapture.cpp
 *
 *  Created on: 11/10/2011
 *      Author: arturo
 */

#include "ofxOpenNICapture.h"

using namespace xn;

string ofxOpenNICapture::LOG_NAME = "ofxOpenNICapture";

#define START_CAPTURE_CHECK_RC(rc, what)												\
	if (nRetVal != XN_STATUS_OK)														\
	{																					\
		ofLogError() << "Failed to" << what << xnGetStatusString(rc);				\
		delete pRecorder;														\
		pRecorder = NULL;														\
		return false;																	\
	}


//----------------------------------------
ofxOpenNICapture::ofxOpenNICapture(){
	// Depth Formats
	int nIndex = 0;

	g_DepthFormat.pValues[nIndex] = XN_CODEC_16Z_EMB_TABLES;
	g_DepthFormat.pIndexToName[nIndex] = "PS Compression (16z ET)";
	nIndex++;

	g_DepthFormat.pValues[nIndex] = XN_CODEC_UNCOMPRESSED;
	g_DepthFormat.pIndexToName[nIndex] = "Uncompressed";
	nIndex++;

	g_DepthFormat.pValues[nIndex] = XN_CODEC_NULL;
	g_DepthFormat.pIndexToName[nIndex] = "Not Captured";
	nIndex++;

	g_DepthFormat.nValuesCount = nIndex;

	// Image Formats
	nIndex = 0;

	g_ImageFormat.pValues[nIndex] = XN_CODEC_JPEG;
	g_ImageFormat.pIndexToName[nIndex] = "JPEG";
	nIndex++;

	g_ImageFormat.pValues[nIndex] = XN_CODEC_UNCOMPRESSED;
	g_ImageFormat.pIndexToName[nIndex] = "Uncompressed";
	nIndex++;

	g_ImageFormat.pValues[nIndex] = XN_CODEC_NULL;
	g_ImageFormat.pIndexToName[nIndex] = "Not Captured";
	nIndex++;

	g_ImageFormat.nValuesCount = nIndex;

	// IR Formats
	nIndex = 0;

	g_IRFormat.pValues[nIndex] = XN_CODEC_UNCOMPRESSED;
	g_IRFormat.pIndexToName[nIndex] = "Uncompressed";
	nIndex++;

	g_IRFormat.pValues[nIndex] = XN_CODEC_NULL;
	g_IRFormat.pIndexToName[nIndex] = "Not Captured";
	nIndex++;

	g_IRFormat.nValuesCount = nIndex;

	// Audio Formats
	nIndex = 0;

	g_AudioFormat.pValues[nIndex] = XN_CODEC_UNCOMPRESSED;
	g_AudioFormat.pIndexToName[nIndex] = "Uncompressed";
	nIndex++;

	g_AudioFormat.pValues[nIndex] = XN_CODEC_NULL;
	g_AudioFormat.pIndexToName[nIndex] = "Not Captured";
	nIndex++;

	g_AudioFormat.nValuesCount = nIndex;

	// Init
	csFileName[0] = 0;
	State = NOT_CAPTURING;
	nCapturedFrameUniqueID = 0;
	csDisplayMessage[0] = '\0';
	bSkipFirstFrame = false;

	nodes[CAPTURE_DEPTH_NODE].captureFormat = XN_CODEC_16Z_EMB_TABLES;
	nodes[CAPTURE_IMAGE_NODE].captureFormat = XN_CODEC_JPEG;
	nodes[CAPTURE_IR_NODE].captureFormat = XN_CODEC_NULL;
	nodes[CAPTURE_AUDIO_NODE].captureFormat = XN_CODEC_NULL;

	pRecorder = NULL;
}

//----------------------------------------
bool ofxOpenNICapture::setup(ofxOpenNI & _context, string filename, XnCodecID depthFormat, XnCodecID imageFormat, XnCodecID irFormat, XnCodecID audioFormat){
	context = &_context;
	csFileName = ofToDataPath(filename);

	nodes[CAPTURE_DEPTH_NODE].captureFormat = depthFormat;
	nodes[CAPTURE_IMAGE_NODE].captureFormat = imageFormat;
	nodes[CAPTURE_IR_NODE].captureFormat = irFormat;
	nodes[CAPTURE_AUDIO_NODE].captureFormat = audioFormat;

	XnStatus nRetVal = XN_STATUS_OK;
	NodeInfoList recordersList;
	nRetVal = context->getXnContext().EnumerateProductionTrees(XN_NODE_TYPE_RECORDER, NULL, recordersList);
	START_CAPTURE_CHECK_RC(nRetVal, "Enumerate recorders");
	// take first
	NodeInfo chosen = *recordersList.Begin();

	pRecorder = new Recorder;
	nRetVal = context->getXnContext().CreateProductionTree(chosen, *pRecorder);
	START_CAPTURE_CHECK_RC(nRetVal, "Create recorder");

	nRetVal = pRecorder->SetDestination(XN_RECORD_MEDIUM_FILE, csFileName.c_str());
	START_CAPTURE_CHECK_RC(nRetVal, "Set output file");

	return true;
}

//----------------------------------------
bool ofxOpenNICapture::startCapture(){
	if (csFileName.empty())
	{
		ofLogError() << "Should call setup before startCapture";
		return false;
	}

	XnUInt64 nNow;
	xnOSGetTimeStamp(&nNow);
	nNow /= 1000;

	nStartOn = (XnUInt32)nNow;
	State = SHOULD_CAPTURE;

	return true;
}

//----------------------------------------
void ofxOpenNICapture::stopCapture(){
	if (pRecorder != NULL){
		pRecorder->Release();
		delete pRecorder;
		pRecorder = NULL;
	}
}

//----------------------------------------
XnStatus ofxOpenNICapture::captureFrame(){
	XnStatus nRetVal = XN_STATUS_OK;

	if (State == SHOULD_CAPTURE){
		XnUInt64 nNow;
		xnOSGetTimeStamp(&nNow);
		nNow /= 1000;

		// check if time has arrived
		if (nNow >= nStartOn)
		{
			// check if we need to discard first frame
			if (bSkipFirstFrame){
				bSkipFirstFrame = false;
			}else{
				// start recording
				for (int i = 0; i < CAPTURE_NODE_COUNT; ++i){
					nodes[i].nCapturedFrames = 0;
					nodes[i].bRecording = false;
				}
				State = CAPTURING;

				// add all captured nodes

				if (context->getDevice().IsValid()){
					nRetVal = pRecorder->AddNodeToRecording(context->getDevice(), XN_CODEC_UNCOMPRESSED);
					START_CAPTURE_CHECK_RC(nRetVal, "add device node");
					ofLogVerbose(LOG_NAME) << "will capture device";
				}

				if (context->getDepthGenerator().IsValid() && nodes[CAPTURE_DEPTH_NODE].captureFormat!=XN_CODEC_NULL){
					nRetVal = pRecorder->AddNodeToRecording(context->getDepthGenerator(), nodes[CAPTURE_DEPTH_NODE].captureFormat);
					START_CAPTURE_CHECK_RC(nRetVal, "add depth node");
					nodes[CAPTURE_DEPTH_NODE].bRecording = TRUE;
					nodes[CAPTURE_DEPTH_NODE].pGenerator = &context->getDepthGenerator();
					ofLogVerbose(LOG_NAME) << "will capture depth";
				}

				if (context->getImageGenerator().IsValid() && nodes[CAPTURE_IMAGE_NODE].captureFormat!=XN_CODEC_NULL){
					nRetVal = pRecorder->AddNodeToRecording(context->getImageGenerator(), nodes[CAPTURE_IMAGE_NODE].captureFormat);
					START_CAPTURE_CHECK_RC(nRetVal, "add image node");
					nodes[CAPTURE_IMAGE_NODE].bRecording = TRUE;
					nodes[CAPTURE_IMAGE_NODE].pGenerator = &context->getImageGenerator();
					ofLogVerbose(LOG_NAME) << "will capture rgb";
				}

				if (context->getIRGenerator().IsValid() && nodes[CAPTURE_IR_NODE].captureFormat!=XN_CODEC_NULL){
					nRetVal = pRecorder->AddNodeToRecording(context->getIRGenerator(), nodes[CAPTURE_IR_NODE].captureFormat);
					START_CAPTURE_CHECK_RC(nRetVal, "add IR stream");
					nodes[CAPTURE_IR_NODE].bRecording = TRUE;
					nodes[CAPTURE_IR_NODE].pGenerator = &context->getIRGenerator();
					ofLogVerbose(LOG_NAME) << "will capture IR";
				}
#if 0 // no audio by noe
				if (isAudioOn() && (nodes[CAPTURE_AUDIO_NODE].captureFormat != CODEC_DONT_CAPTURE))
				{
					nRetVal = pRecorder->AddNodeToRecording(*getAudioGenerator(), nodes[CAPTURE_AUDIO_NODE].captureFormat);
					START_CAPTURE_CHECK_RC(nRetVal, "add Audio stream");
					nodes[CAPTURE_AUDIO_NODE].bRecording = TRUE;
					nodes[CAPTURE_AUDIO_NODE].pGenerator = getAudioGenerator();
				}
#endif
			}
		}
	}

	if (State == CAPTURING){
		// There isn't a real need to call Record() here, as the WaitXUpdateAll() call already makes sure
		// recording is performed.
		nRetVal = pRecorder->Record();
		if (nRetVal != ((XnStatus)0)){
			ofLogError(LOG_NAME) << "Error capturing frame" << xnGetStatusString(nRetVal);
			return (nRetVal);
		}

		// count recorded frames
		for (int i = 0; i < CAPTURE_NODE_COUNT; ++i){
			if (nodes[i].bRecording && nodes[i].pGenerator->IsDataNew())
				nodes[i].nCapturedFrames++;
		}
	}
	return XN_STATUS_OK;
}

//----------------------------------------
void ofxOpenNICapture::update(){
	captureFrame();
}
