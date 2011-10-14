/*
 * ofxOpenNIUtils.h
 *
 *  Created on: 11/10/2011
 *      Author: arturo
 */

#ifndef OFXOPENNIUTILS_H_
#define OFXOPENNIUTILS_H_

#include <XnTypes.h>
#include "ofPoint.h"

void YUV422ToRGB888(const XnUInt8* pYUVImage, XnUInt8* pRGBImage, XnUInt32 nYUVSize, XnUInt32 nRGBSize);

#define SHOW_RC(rc, what)											\
	ofLogNotice(LOG_NAME) << what << "status:" << xnGetStatusString(rc);

inline ofPoint toOf(const XnPoint3D & p){
	return *(ofPoint*)&p;
	/*
	 this is more future safe, but currently unnecessary and slower:
	return ofPoint(p.X,p.Y,p.Z);
	 */
}

inline XnPoint3D toXn(const ofPoint & p){
	
	return *(XnPoint3D*)&p;
	/*
	 this is more future safe, but currently unnecessary and slower:
	XnPoint3D r;
	r.X = p.x;
	r.Y = p.y;
	r.Z = p.z;
	return r;
	 */
}
#endif /* OFXOPENNIUTILS_H_ */
