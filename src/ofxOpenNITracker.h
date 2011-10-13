#pragma once

#include <map>
#include <set>
#include <XnTypes.h>
#include <XnCppWrapper.h>
#include "ofConstants.h"
#include "ofxOpenNIUser.h"
#include "ofMesh.h"

class ofxOpenNI;

class ofxOpenNITracker{
public:
	ofxOpenNITracker();

	bool setup(ofxOpenNI & openNI);

	void update();
	void draw();

	void drawUser(int nUserNum);

	void setUseMaskPixels(bool b);
	void setUsePointClouds(bool b);

	void setSmoothing(float smooth);
	float getSmoothing();

	int	getNumberOfTrackedUsers();
	ofxOpenNIUser&	getTrackedUser(int nUserNum);

	float getWidth();
	float getHeight();


	xn::UserGenerator&	getXnUserGenerator();

	static string LOG_NAME;
private:
	void updatePointClouds(ofxOpenNIUser & user);
	void updateUserPixels(ofxOpenNIUser & user);

	bool needsPoseForCalibration();
	void startPoseDetection(XnUserID nID);
	void stopPoseDetection(XnUserID nID);
	void requestCalibration(XnUserID nID);
	void startTracking(XnUserID nID);


	static void XN_CALLBACK_TYPE User_NewUser(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie);
	static void XN_CALLBACK_TYPE User_LostUser(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie);
	static void XN_CALLBACK_TYPE UserPose_PoseDetected(xn::PoseDetectionCapability& rCapability, const XnChar* strPose, XnUserID nID, void* pCookie);
	static void XN_CALLBACK_TYPE UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nID, void* pCookie);
	static void XN_CALLBACK_TYPE UserCalibration_CalibrationEnd(xn::SkeletonCapability& rCapability, XnUserID nID, XnCalibrationStatus bSuccess, void* pCookie);

	ofxOpenNI * openNI;

	xn::UserGenerator user_generator;

	bool needs_pose;
	XnChar	calibration_pose[20];

	bool usePointClouds,useMaskPixels;

	set<XnUserID> prev_tracked_users;
	map<XnUserID,ofxOpenNIUser> tracked_users;
	vector<XnUserID> tracked_users_index;

	int width, height;

	float smoothing_factor;
};
