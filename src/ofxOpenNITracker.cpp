/*
 * ofxOpenNITracker.cpp
 *
 *  Created on: 12/10/2011
 *      Author: arturo
 */

#include "ofxOpenNITracker.h"

#include "ofGraphics.h"

#include "ofxOpenNIUtils.h"
#include "ofxOpenNI.h"


string ofxOpenNITracker::LOG_NAME = "ofxOpenNITracker";

#define MAX_NUMBER_USERS 20

// CALLBACKS
// =============================================================================

//----------------------------------------
void XN_CALLBACK_TYPE ofxOpenNITracker::User_NewUser(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie){
	ofLogVerbose(LOG_NAME) << "New User" << nID;

	ofxOpenNITracker* tracker = static_cast<ofxOpenNITracker*>(pCookie);
	if(tracker->needsPoseForCalibration()) {
		tracker->startPoseDetection(nID);
	} else {
		tracker->requestCalibration(nID);
	}
}

//----------------------------------------
void XN_CALLBACK_TYPE ofxOpenNITracker::User_LostUser(xn::UserGenerator& rGenerator, XnUserID nID, void* pCookie){
	ofLogVerbose(LOG_NAME) << "Lost user" << nID;
	rGenerator.GetSkeletonCap().Reset(nID);

}

//----------------------------------------
void XN_CALLBACK_TYPE ofxOpenNITracker::UserPose_PoseDetected(xn::PoseDetectionCapability& rCapability, const XnChar* strPose, XnUserID nID, void* pCookie){
	ofxOpenNITracker* tracker = static_cast<ofxOpenNITracker*>(pCookie);
	ofLogVerbose(LOG_NAME) << "Pose" << strPose << "detected for user" << nID;
	tracker->requestCalibration(nID);
	tracker->stopPoseDetection(nID);
}


//----------------------------------------
void XN_CALLBACK_TYPE ofxOpenNITracker::UserCalibration_CalibrationStart(xn::SkeletonCapability& capability, XnUserID nID, void* pCookie){
	ofLogVerbose(LOG_NAME) << "Calibration started for user" << nID;
}


//----------------------------------------
void XN_CALLBACK_TYPE ofxOpenNITracker::UserCalibration_CalibrationEnd(xn::SkeletonCapability& rCapability, XnUserID nID, XnCalibrationStatus bSuccess, void* pCookie){
	ofxOpenNITracker* tracker = static_cast<ofxOpenNITracker*>(pCookie);
	if(bSuccess == XN_CALIBRATION_STATUS_OK) {
		ofLogVerbose(LOG_NAME) << "+++++++++++++++++++++++ Succesfully tracked user:" << nID;
		tracker->startTracking(nID);
	} else {
		if(tracker->needsPoseForCalibration()) {
			tracker->startPoseDetection(nID);
		} else {
			tracker->requestCalibration(nID);
		}
	}
}





//----------------------------------------
ofxOpenNITracker::ofxOpenNITracker(){
	openNI = NULL;
	smoothing_factor = 1;
	usePointClouds = false;
	useMaskPixels = false;
}

//----------------------------------------
bool ofxOpenNITracker::setup(ofxOpenNI & _openNI){
	openNI = &_openNI;

	if (!openNI->getDepthGenerator().IsValid()){
		ofLogError(LOG_NAME) << "no depth generator present";
		return false;
	}

	XnStatus result = XN_STATUS_OK;

	// get map_mode so we can setup width and height vars from depth gen size
	XnMapOutputMode map_mode;
	openNI->getDepthGenerator().GetMapOutputMode(map_mode);

	width = map_mode.nXRes;
	height = map_mode.nYRes;

	// set update mask pixels default to false
	//useMaskPixels = false;

	// setup mask pixels array TODO: clean this up on closing or dtor
	//including 0 as all users
	//for (int user = 0; user <= MAX_NUMBER_USERS; user++) {
	//	maskPixels[user] = new unsigned char[width * height];
	//}

	// set update cloud points default to false
	/*cloudPoints.resize(MAX_NUMBER_USERS);

	// setup cloud points array TODO: clean this up on closing or dtor
	// including 0 as all users
	for (int user = 0; user <= MAX_NUMBER_USERS; user++) {
		cloudPoints[user].getVertices().resize(width * height);
		cloudPoints[user].getColors().resize(width * height);
		cloudPoints[user].setMode(OF_PRIMITIVE_POINTS);
	}*/

	// if one doesn't exist then create user generator.
	result = user_generator.Create(openNI->getXnContext());
	SHOW_RC(result, "Create user generator");

	if (result != XN_STATUS_OK) return false;

	// register user callbacks
	XnCallbackHandle user_cb_handle;
	user_generator.RegisterUserCallbacks(
		 User_NewUser
		,User_LostUser
		,this
		,user_cb_handle
	);

	XnCallbackHandle calibration_cb_handle;
	user_generator.GetSkeletonCap().RegisterToCalibrationStart(
		 UserCalibration_CalibrationStart
		,this
		,calibration_cb_handle
	);

	user_generator.GetSkeletonCap().RegisterToCalibrationComplete(
			UserCalibration_CalibrationEnd
			,this
			,calibration_cb_handle
	);

	// check if we need to pose for calibration
	if(user_generator.GetSkeletonCap().NeedPoseForCalibration()) {

		needs_pose = true;

		if(!user_generator.IsCapabilitySupported(XN_CAPABILITY_POSE_DETECTION)) {
			ofLogError(LOG_NAME) << "Pose required, but not supported";
			return false;
		}

		XnCallbackHandle user_pose_cb_handle;

		user_generator.GetPoseDetectionCap().RegisterToPoseDetected(
			 UserPose_PoseDetected
			,this
			,user_pose_cb_handle
		);

		user_generator.GetSkeletonCap().GetCalibrationPose(calibration_pose);

	}

	user_generator.GetSkeletonCap().SetSkeletonProfile(XN_SKEL_PROFILE_ALL);

	// needs this to allow skeleton tracking when using pre-recorded .oni or nodes init'd by code (as opposed to xml)
	// as otherwise the image/depth nodes play but are not generating callbacks
	//if (context->isUsingRecording()) {
	result = openNI->getXnContext().StartGeneratingAll();
	SHOW_RC(result, "StartGenerating");
	if (result != XN_STATUS_OK) return false;

	return true;
}

//----------------------------------------
void ofxOpenNITracker::setUseMaskPixels(bool b){
	useMaskPixels = b;
}

//----------------------------------------
void ofxOpenNITracker::setUsePointClouds(bool b){
	usePointClouds = b;
}

//----------------------------------------
void ofxOpenNITracker::setSmoothing(float smooth){
	if (smooth > 0.0f && smooth < 1.0f) {
		smoothing_factor = smooth;
		if (user_generator.IsValid()) {
			user_generator.GetSkeletonCap().SetSmoothing(smooth);
		}
	}
}

//----------------------------------------
float ofxOpenNITracker::getSmoothing(){
	return smoothing_factor;
}

//----------------------------------------
void ofxOpenNITracker::update(){
	vector<XnUserID> users(MAX_NUMBER_USERS);
	XnUInt16 max_users = MAX_NUMBER_USERS;
	user_generator.GetUsers(&users[0], max_users);

	set<XnUserID> current_tracked_users;

	for(int i = 0; i < MAX_NUMBER_USERS; ++i) {
		if(user_generator.GetSkeletonCap().IsTracking(users[i])) {
			ofxOpenNIUser & user = tracked_users[users[i]];
			user.id = users[i];
			XnPoint3D center;
			user_generator.GetCoM(users[i], center);
			user.center = toOf(center);

			for(int j=0;j<ofxOpenNIUser::NumLimbs;j++){
				XnSkeletonJointPosition a,b;
				user_generator.GetSkeletonCap().GetSkeletonJointPosition(user.id, user.limbs[j].start_joint, a);
				user_generator.GetSkeletonCap().GetSkeletonJointPosition(user.id, user.limbs[j].end_joint, b);
				user_generator.GetSkeletonCap().GetSkeletonJointOrientation(user.id,user.limbs[j].start_joint, user.limbs[j].orientation);
				if(a.fConfidence < 0.3f || b.fConfidence < 0.3f) {
					user.limbs[j].found = false;
					continue;
				}

				user.limbs[j].found = true;
				user.limbs[j].begin = openNI->worldToProjective(a.position);
				user.limbs[j].end = openNI->worldToProjective(b.position);
				user.limbs[j].worldBegin = toOf(a.position);
				user.limbs[j].worldEnd = toOf(b.position);
			}

			if (usePointClouds) updatePointClouds(user);
			if (useMaskPixels) updateUserPixels(user);

			current_tracked_users.insert(user.id);
		}
	}

	set<XnUserID>::iterator it;
	for(it=prev_tracked_users.begin();it!=prev_tracked_users.end();it++){
		if(current_tracked_users.find(*it)==current_tracked_users.end()){
			tracked_users.erase(*it);
		}
	}

	prev_tracked_users = current_tracked_users;
	tracked_users_index.assign(prev_tracked_users.begin(),prev_tracked_users.end());

	//if (useMaskPixels) updateUserPixels();
}

//----------------------------------------
void ofxOpenNITracker::updatePointClouds(ofxOpenNIUser & user) {

	const XnRGB24Pixel*		pColor;
	const XnDepthPixel*		pDepth = openNI->getDepthMetaData().Data();

	bool hasImageGenerator = openNI->getImageGenerator().IsValid();

	if (hasImageGenerator) {
		pColor = openNI->getImageMetaData().RGB24Data();
	}

	xn::SceneMetaData smd;
	unsigned short *userPix;

	if (user_generator.GetUserPixels(user.id, smd) == XN_STATUS_OK) {
		userPix = (unsigned short*)smd.Data();
	}

	int step = 1;
	int nIndex = 0;

	user.pointCloud.getVertices().clear();
	user.pointCloud.getColors().clear();
	user.pointCloud.setMode(OF_PRIMITIVE_POINTS);

	for (int nY = 0; nY < height; nY += step) {
		for (int nX = 0; nX < width; nX += step, nIndex += step) {
			if (userPix[nIndex] == user.id) {
				user.pointCloud.addVertex(ofPoint( nX,nY,pDepth[nIndex] ));
				ofColor color;
				if(hasImageGenerator){
					user.pointCloud.addColor(ofColor(pColor[nIndex].nRed,pColor[nIndex].nGreen,pColor[nIndex].nBlue));
				}else{
					user.pointCloud.addColor(ofFloatColor(1,1,1));
				}
			}
		}
	}
}

//----------------------------------------
void ofxOpenNITracker::updateUserPixels(ofxOpenNIUser & user){
	xn::SceneMetaData smd;
	unsigned short *userPix;

	if (user_generator.GetUserPixels(user.id, smd) == XN_STATUS_OK) { //	GetUserPixels is supposed to take a user ID number,
		userPix = (unsigned short*)smd.Data();					//  but you get the same data no matter what you pass.
	}															//	userPix actually contains an array where each value
																//  corresponds to the user being tracked.
																//  Ie.,	if userPix[i] == 0 then it's not being tracked -> it's the background!
																//			if userPix[i] > 0 then the pixel belongs to the user who's value IS userPix[i]
																//  // (many thanks to ascorbin who's code made this apparent to me)

	user.maskPixels.allocate(width,height,OF_IMAGE_GRAYSCALE);

	for (int i =0 ; i < width * height; i++) {
		if (userPix[i] == user.id) {
			user.maskPixels[i] = 255;
		} else {
			user.maskPixels[i] = 0;
		}

	}
}

//----------------------------------------
void ofxOpenNITracker::draw(){
	ofPushStyle();
	// show green/red circle if any one is found
	if (tracked_users_index.size() > 0) {

		// draw all the users
		for(int i = 0;  i < (int)tracked_users_index.size(); ++i) {
			drawUser(i);
		}

	}
	ofPopStyle();
}

//----------------------------------------
void ofxOpenNITracker::drawUser(int nUserNum) {
	if(nUserNum - 1 > (int)tracked_users_index.size())
		return;
	tracked_users[tracked_users_index[nUserNum]].debugDraw();
}


//----------------------------------------
void ofxOpenNITracker::startPoseDetection(XnUserID nID) {
	ofLogVerbose(LOG_NAME) << "Start pose detection for user" << nID;
	user_generator.GetPoseDetectionCap().StartPoseDetection(calibration_pose, nID);
}


//----------------------------------------
void ofxOpenNITracker::stopPoseDetection(XnUserID nID) {
	user_generator.GetPoseDetectionCap().StopPoseDetection(nID);
}


//----------------------------------------
void ofxOpenNITracker::requestCalibration(XnUserID nID) {
	ofLogVerbose(LOG_NAME) << "Calibration requested for user" << nID;
	user_generator.GetSkeletonCap().RequestCalibration(nID, TRUE);
}

//----------------------------------------
void ofxOpenNITracker::startTracking(XnUserID nID) {
	user_generator.GetSkeletonCap().StartTracking(nID);
}

//----------------------------------------
bool ofxOpenNITracker::needsPoseForCalibration() {
	return needs_pose;
}

//----------------------------------------
xn::UserGenerator&	ofxOpenNITracker::getXnUserGenerator(){
	return user_generator;
}

//----------------------------------------
int	ofxOpenNITracker::getNumberOfTrackedUsers(){
	return tracked_users_index.size();
}

//----------------------------------------
ofxOpenNIUser&	ofxOpenNITracker::getTrackedUser(int nUserNum){
	return tracked_users[tracked_users_index[nUserNum]];
}

//----------------------------------------
float ofxOpenNITracker::getWidth(){
	return width;
}

//----------------------------------------
float ofxOpenNITracker::getHeight(){
	return height;
}
