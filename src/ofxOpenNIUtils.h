/*
 * ofxOpenNIUtils.h
 *
 *  Created on: 11/10/2011
 *      Author: arturo
 */

#ifndef OFXOPENNIUTILS_H_
#define OFXOPENNIUTILS_H_

void YUV422ToRGB888(const XnUInt8* pYUVImage, XnUInt8* pRGBImage, XnUInt32 nYUVSize, XnUInt32 nRGBSize);

#define SHOW_RC(rc, what)											\
	ofLogNotice(LOG_NAME) << what << "status:" << xnGetStatusString(rc);


#endif /* OFXOPENNIUTILS_H_ */
