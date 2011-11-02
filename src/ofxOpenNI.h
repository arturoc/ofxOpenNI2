#pragma once

#include <XnCppWrapper.h>
#include <XnTypes.h>
#include "ofConstants.h"
#include "ofPixels.h"
#include "ofTexture.h"
#include "ofThread.h"

class ofxOpenNI: public ofThread{
public:
	ofxOpenNI();

	bool setupFromXML(string xml, bool threaded=true);
	bool setupFromRecording(string recording, bool threaded=true);

	bool isNewFrame();
	void update();
	void draw(int x, int y);
	void drawRGB(int x, int y);

	void setUseTexture(bool useTexture);

	ofPixels & getDepthPixels();
	ofShortPixels & getDepthRawPixels();
	ofPixels & getRGBPixels();
	
	ofTexture & getDepthTextureReference();
	ofTexture & getRGBTextureReference();

	ofMesh & getPointCloud();
	void setGeneratePCColors(bool generateColors);
	void setGeneratePCTexCoords(bool generateTexCoords);

	float getWidth();
	float getHeight();

	enum DepthColoring {
		COLORING_PSYCHEDELIC_SHADES = 0,
		COLORING_PSYCHEDELIC,
		COLORING_RAINBOW,
		COLORING_CYCLIC_RAINBOW,
		COLORING_BLUES,
		COLORING_GREY,
		COLORING_STATUS,
		COLORING_COUNT
	};

	void setDepthColoring(DepthColoring coloring);

	bool toggleCalibratedRGBDepth();
	bool enableCalibratedRGBDepth();
	bool disableCalibratedRGBDepth();

	ofPoint worldToProjective(const ofPoint & p);
	ofPoint worldToProjective(const XnVector3D & p);

	ofPoint projectiveToWorld(const ofPoint & p);
	ofPoint projectiveToWorld(const XnVector3D & p);

	ofPoint cameraToWorld(const ofVec2f& c);
	void cameraToWorld(const vector<ofVec2f>& c, vector<ofVec3f>& w);

	void addLicense(string sVendor, string sKey);

	xn::Context & getXnContext();
	xn::Device & getDevice();
	xn::DepthGenerator & getDepthGenerator();
	xn::ImageGenerator & getImageGenerator();
	xn::IRGenerator & getIRGenerator();
	xn::AudioGenerator & getAudioGenerator();
	xn::Player & getPlayer();

	xn::DepthMetaData & getDepthMetaData();
	xn::ImageMetaData & getImageMetaData();
	xn::IRMetaData & getIRMetaData();
	xn::AudioMetaData & getAudioMetaData();

	static string LOG_NAME;

protected:
	void threadedFunction();

private:
	void openCommon();
	void initConstants();
	void readFrame();
	void generateDepthPixels();
	void generateImagePixels();
	void allocateDepthBuffers();
	void allocateDepthRawBuffers();
	void allocateRGBBuffers();

	static void XN_CALLBACK_TYPE onErrorStateChanged(XnStatus errorState, void* pCookie);

	xn::Context g_Context;
	xn::ScriptNode g_scriptNode;

	struct DeviceParameter{
		int nValuesCount;
		unsigned int pValues[20];
		string pValueToName[20];
	};

	struct NodeCodec{
		int nValuesCount;
		XnCodecID pValues[20];
		string pIndexToName[20];
	};

	struct DeviceStringProperty{
		int nValuesCount;
		string pValues[20];
	};

	DeviceStringProperty g_PrimaryStream;
	DeviceParameter g_Registration;
	DeviceParameter g_Resolution;
	bool g_bIsDepthOn;
	bool g_bIsImageOn;
	bool g_bIsIROn;
	bool g_bIsAudioOn;
	bool g_bIsPlayerOn;
	bool g_bIsDepthRawOnOption;

	xn::Device g_Device;
	xn::DepthGenerator g_Depth;
	xn::ImageGenerator g_Image;
	xn::IRGenerator g_IR;
	xn::AudioGenerator g_Audio;
	xn::Player g_Player;

	xn::MockDepthGenerator mockDepth;

	xn::DepthMetaData g_DepthMD;
	xn::ImageMetaData g_ImageMD;
	xn::IRMetaData g_irMD;
	xn::AudioMetaData g_AudioMD;

	xn::ProductionNode* g_pPrimary;


	bool useTexture;
	bool bNewPixels;
	bool bNewFrame;
	bool threaded;


	// depth
	ofTexture depthTexture;
	ofPixels depthPixels[2];
	ofPixels * backDepthPixels, * currentDepthPixels;
	DepthColoring depth_coloring;
	float	max_depth;
	
	// depth raw
	ofShortPixels depthRawPixels[2];
	ofShortPixels * backDepthRawPixels, * currentDepthRawPixels;

	// rgb
	ofTexture rgbTexture;
	ofPixels rgbPixels[2];
	ofPixels * backRGBPixels, * currentRGBPixels;

	// point cloud
	ofMesh pointCloud;
	bool isPointCloudValid;
	bool bGeneratePCColors, bGeneratePCTexCoords;
};
