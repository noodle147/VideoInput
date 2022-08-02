#ifndef _VIDEOINPUT
#define _VIDEOINPUT

//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//THE SOFTWARE.

//////////////////////////////////////////////////////////
//Written by Theodore Watson - theo.watson@gmail.com    //
//Do whatever you want with this code but if you find   //
//a bug or make an improvement I would love to know!    //
//														//
//Warning This code is experimental 					//
//use at your own risk :)								//
//////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
/*                     Shoutouts

Thanks to:

		   Dillip Kumar Kara for crossbar code.
		   Zachary Lieberman for getting me into this stuff
		   and for being so generous with time and code.
		   The guys at Potion Design for helping me with VC++
		   Josh Fisher for being a serious C++ nerd :)
		   Golan Levin for helping me debug the strangest
		   and slowest bug in the world!

		   And all the people using this library who send in
		   bugs, suggestions and improvements who keep me working on
		   the next version - yeah thanks a lot ;)

*/
/////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma comment(lib,"Strmiids.lib") 
#endif

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <wchar.h>
#include <string>
#include <vector>

//this is for TryEnterCriticalSection
#ifndef _WIN32_WINNT
#   define _WIN32_WINNT 0x501
#endif
#include <windows.h>
#include "tchar.h"
#include "dshow.h"

//Example Usage
/*
	//create a videoInput object
	videoInput VI;

	//Prints out a list of available devices and returns num of devices found
	int numDevices = VI.listDevices();

	int device1 = 0;  //this could be any deviceID that shows up in listDevices
	int device2 = 1;  //this could be any deviceID that shows up in listDevices

	//if you want to capture at a different frame rate (default is 30)
	//specify it here, you are not guaranteed to get this fps though.
	//VI.setIdealFramerate(dev, 60);

	//setup the first device - there are a number of options:

	VI.setupDevice(device1); 						  //setup the first device with the default settings
	//VI.setupDevice(device1, VI_COMPOSITE); 			  //or setup device with specific connection type
	//VI.setupDevice(device1, 320, 240);				  //or setup device with specified video size
	//VI.setupDevice(device1, 320, 240, VI_COMPOSITE);  //or setup device with video size and connection type

	//VI.setFormat(device1, VI_NTSC_M);					//if your card doesn't remember what format it should be
														//call this with the appropriate format listed above
														//NOTE: must be called after setupDevice!

	//optionally setup a second (or third, fourth ...) device - same options as above
	VI.setupDevice(device2);

	//As requested width and height can not always be accomodated
	//make sure to check the size once the device is setup

	int width 	= VI.getWidth(device1);
	int height 	= VI.getHeight(device1);
	int size	= VI.getSize(device1);

	unsigned char * yourBuffer1 = new unsigned char[size];
	unsigned char * yourBuffer2 = new unsigned char[size];

	//to get the data from the device first check if the data is new
	if(VI.isFrameNew(device1)){
		VI.getPixels(device1, yourBuffer1, false, false);	//fills pixels as a BGR (for openCV) unsigned char array - no flipping
		VI.getPixels(device1, yourBuffer2, true, true); 	//fills pixels as a RGB (for openGL) unsigned char array - flipping!
	}

	//same applies to device2 etc

	//to get a settings dialog for the device
	VI.showSettingsWindow(device1);


	//Shut down devices properly
	VI.stopDevice(device1);
	VI.stopDevice(device2);
*/


//////////////////////////////////////   VARS AND DEFS   //////////////////////////////////


//STUFF YOU DON'T CHANGE

//videoInput defines
#define VI_VERSION	 0.200
#define VI_MAX_CAMERAS  20
#define VI_NUM_TYPES    19 //DON'T TOUCH
#define VI_NUM_FORMATS  18 //DON'T TOUCH

//defines for setPhyCon - tuner is not as well supported as composite and s-video
#define VI_COMPOSITE 0
#define VI_S_VIDEO   1
#define VI_TUNER     2
#define VI_USB       3
#define VI_1394		 4

//defines for formats
#define VI_NTSC_M	0
#define VI_PAL_B	1
#define VI_PAL_D	2
#define VI_PAL_G	3
#define VI_PAL_H	4
#define VI_PAL_I	5
#define VI_PAL_M	6
#define VI_PAL_N	7
#define VI_PAL_NC	8
#define VI_SECAM_B	9
#define VI_SECAM_D	10
#define VI_SECAM_G	11
#define VI_SECAM_H	12
#define VI_SECAM_K	13
#define VI_SECAM_K1	14
#define VI_SECAM_L	15
#define VI_NTSC_M_J	16
#define VI_NTSC_433	17

// added by gameover
#define VI_MEDIASUBTYPE_RGB24   0
#define VI_MEDIASUBTYPE_RGB32   1
#define VI_MEDIASUBTYPE_RGB555  2
#define VI_MEDIASUBTYPE_RGB565  3
#define VI_MEDIASUBTYPE_YUY2    4
#define VI_MEDIASUBTYPE_YVYU    5
#define VI_MEDIASUBTYPE_YUYV    6
#define VI_MEDIASUBTYPE_IYUV    7
#define VI_MEDIASUBTYPE_UYVY    8
#define VI_MEDIASUBTYPE_YV12    9
#define VI_MEDIASUBTYPE_YVU9    10
#define VI_MEDIASUBTYPE_Y411    11
#define VI_MEDIASUBTYPE_Y41P    12
#define VI_MEDIASUBTYPE_Y211    13
#define VI_MEDIASUBTYPE_AYUV    14
#define VI_MEDIASUBTYPE_Y800    15
#define VI_MEDIASUBTYPE_Y8      16
#define VI_MEDIASUBTYPE_GREY    17
#define VI_MEDIASUBTYPE_MJPG    18

//allows us to directShow classes here with the includes in the cpp
struct ICaptureGraphBuilder2;
struct IGraphBuilder;
struct IBaseFilter;
struct IAMCrossbar;
struct IMediaControl;
struct ISampleGrabber;
struct IMediaEventEx;
struct IAMStreamConfig;
struct _AMMediaType;
class SampleGrabberCallback;
typedef _AMMediaType AM_MEDIA_TYPE;




////////////////////////////////////////   VIDEO DEVICE   ///////////////////////////////////

class videoDevice {


public:

	videoDevice();
	void setSize(int w, int h);
	void NukeDownstream(IBaseFilter* pBF);
	void destroyGraph();
	~videoDevice();

	int videoSize;
	int width;
	int height;
	int tryWidth;
	int tryHeight;

	ICaptureGraphBuilder2* pCaptureGraph;	// Capture graph builder object
	IGraphBuilder* pGraph;					// Graph builder object
	IMediaControl* pControl;				// Media control object
	IBaseFilter* pVideoInputFilter;  		// Video Capture filter
	IBaseFilter* pGrabberF;
	IBaseFilter* pDestFilter;
	IAMStreamConfig* streamConf;
	ISampleGrabber* pGrabber;    			// Grabs frame
	AM_MEDIA_TYPE* pAmMediaType;

	IMediaEventEx* pMediaEvent;

	GUID videoType;
	long formatType;

	SampleGrabberCallback* sgCallback;

	bool tryDiffSize;
	bool useCrossbar;
	bool readyToCapture;
	bool sizeSet;
	bool setupStarted;
	bool specificFormat;
	bool autoReconnect;
	int  nFramesForReconnect;
	unsigned long nFramesRunning;
	int  connection;
	int	 storeConn;
	int  myID;
	long requestedFrameTime; //ie fps

	char 	nDeviceName[255];
	WCHAR 	wDeviceName[255];

	unsigned char* pixels;
	char* pBuffer;

};




//////////////////////////////////////   VIDEO INPUT   /////////////////////////////////////



class videoInput {

public:
	videoInput();
	~videoInput();

	//turns off console messages - default is to print messages
	static void setVerbose(bool _verbose);

	//this allows for multithreaded use of VI ( default is single threaded ).
	//call this before any videoInput calls. 
	//note if your app has other COM calls then you should set VIs COM usage to match the other COM mode 
	static void setComMultiThreaded(bool bMulti);

	//Functions in rough order they should be used.
	static int listDevices(bool silent = false);
	static std::vector <std::string> getDeviceList();

	//needs to be called after listDevices - otherwise returns NULL
	static const char* getDeviceName(int deviceID);
	static int getDeviceIDFromName(const char* name);

	//needs to be called after listDevices - otherwise returns empty string
	static const std::wstring& getUniqueDeviceName(int deviceID);
	static int getDeviceIDFromUniqueName(const std::wstring& uniqueName);

	//choose to use callback based capture - or single threaded
	void setUseCallback(bool useCallback);

	//call before setupDevice
	//directshow will try and get the closest possible framerate to what is requested
	void setIdealFramerate(int deviceID, int idealFramerate);

	//some devices will stop delivering frames after a while - this method gives you the option to try and reconnect
	//to a device if videoInput detects that a device has stopped delivering frames.
	//you MUST CALL isFrameNew every app loop for this to have any effect
	void setAutoReconnectOnFreeze(int deviceNumber, bool doReconnect, int numMissedFramesBeforeReconnect);

	//Choose one of these four to setup your device
	bool setupDevice(int deviceID);
	bool setupDevice(int deviceID, int w, int h);

	//These two are only for capture cards
	//USB and Firewire cameras souldn't specify connection
	bool setupDevice(int deviceID, int connection);
	bool setupDevice(int deviceID, int w, int h, int connection);

	//If you need to you can set your NTSC/PAL/SECAM
	//preference here. if it is available it will be used.
	//see #defines above for available formats - eg VI_NTSC_M or VI_PAL_B
	//should be called after setupDevice
	//can be called multiple times
	bool setFormat(int deviceNumber, int format);
	void setRequestedMediaSubType(int mediatype); // added by gameover

	//Tells you when a new frame has arrived - you should call this if you have specified setAutoReconnectOnFreeze to true
	bool isFrameNew(int deviceID);

	bool isDeviceSetup(int deviceID);

	//Returns the pixels - flipRedAndBlue toggles RGB/BGR flipping - and you can flip the image too
	unsigned char* getPixels(int deviceID, bool flipRedAndBlue = true, bool flipImage = false);

	//Or pass in a buffer for getPixels to fill returns true if successful.
	bool getPixels(int id, unsigned char* pixels, bool flipRedAndBlue = true, bool flipImage = false);

	//Launches a pop up settings window
	//For some reason in GLUT you have to call it twice each time.
	void showSettingsWindow(int deviceID);

	//Manual control over settings thanks.....
	//These are experimental for now.
	bool setVideoSettingFilter(int deviceID, long Property, long lValue, long Flags = 0, bool useDefaultValue = false);
	bool setVideoSettingFilterPct(int deviceID, long Property, float pctValue, long Flags = 0);
	bool getVideoSettingFilter(int deviceID, long Property, long& min, long& max, long& SteppingDelta, long& currentValue, long& flags, long& defaultValue);

	bool setVideoSettingCamera(int deviceID, long Property, long lValue, long Flags = 0, bool useDefaultValue = false);
	bool setVideoSettingCameraPct(int deviceID, long Property, float pctValue, long Flags = 0);
	bool getVideoSettingCamera(int deviceID, long Property, long& min, long& max, long& SteppingDelta, long& currentValue, long& flags, long& defaultValue);

	//bool setVideoSettingCam(int deviceID, long Property, long lValue, long Flags = 0, bool useDefaultValue = false);

	//get width, height and number of pixels
	int  getWidth(int deviceID);
	int  getHeight(int deviceID);
	int  getSize(int deviceID);

	//completely stops and frees a device
	void stopDevice(int deviceID);

	//as above but then sets it up with same settings
	bool restartDevice(int deviceID);

	//number of devices available
	int  devicesFound;

	long propBrightness;
	long propContrast;
	long propHue;
	long propSaturation;
	long propSharpness;
	long propGamma;
	long propColorEnable;
	long propWhiteBalance;
	long propBacklightCompensation;
	long propGain;

	long propPan;
	long propTilt;
	long propRoll;
	long propZoom;
	long propExposure;
	long propIris;
	long propFocus;

private:

	void setPhyCon(int deviceID, int conn);
	void setAttemptCaptureSize(int deviceID, int w, int h);
	bool setup(int deviceID);
	void processPixels(unsigned char* src, unsigned char* dst, int width, int height, bool bRGB, bool bFlip);
	int  start(int deviceID, videoDevice* VD);
	int  getDeviceCount();
	void getMediaSubtypeAsString(GUID type, char* typeAsString);

	HRESULT getDevice(IBaseFilter** pSrcFilter, int deviceID, WCHAR* wDeviceName, char* nDeviceName);
	static HRESULT ShowFilterPropertyPages(IBaseFilter* pFilter);
	HRESULT SaveGraphFile(IGraphBuilder* pGraph, WCHAR* wszPath);
	HRESULT routeCrossbar(ICaptureGraphBuilder2** ppBuild, IBaseFilter** pVidInFilter, int conType, GUID captureMode);

	//don't touch
	static bool comInit();
	static bool comUnInit();

	int  connection;
	int  callbackSetCount;
	bool bCallback;

	GUID CAPTURE_MODE;
	GUID requestedMediaSubType;

	//Extra video subtypes
	GUID MEDIASUBTYPE_Y800;
	GUID MEDIASUBTYPE_Y8;
	GUID MEDIASUBTYPE_GREY;

	videoDevice* VDList[VI_MAX_CAMERAS];
	GUID mediaSubtypes[VI_NUM_TYPES];
	long formatTypes[VI_NUM_FORMATS];

	static void __cdecl basicThread(void* objPtr);

	static char deviceNames[VI_MAX_CAMERAS][255];

	static std::vector<std::wstring> deviceUniqueNames;

};

//////////////////////////////////////   UTILS   /////////////////////////////////////

const UCHAR WHITE[] = { UCHAR(235), UCHAR(128), UCHAR(128) };
const UCHAR RED[] = { UCHAR(82), UCHAR(90), UCHAR(240) };
const UCHAR GREEN[] = { UCHAR(145), UCHAR(54), UCHAR(34) };
const UCHAR BLUE[] = { UCHAR(41), UCHAR(240), UCHAR(110) };

class VIUtils {
public:
	static HRESULT EnumPins(IBaseFilter* pFilter) {
		if (pFilter == NULL)
		{
			return E_POINTER;
		}
		HRESULT hr;
		IEnumPins* pEnum = NULL;
		hr = pFilter->EnumPins(&pEnum);
		if (FAILED(hr)) {
			return hr;
		}
		IPin* pPin = NULL;
		while (pEnum->Next(1, &pPin, 0) == S_OK)
		{
			PIN_DIRECTION PinDir;
			hr = pPin->QueryDirection(&PinDir);
			if (FAILED(hr)) {
				pPin->Release();
				pEnum->Release();
				return hr;
			}
			PIN_INFO pinInfo;
			hr = pPin->QueryPinInfo(&pinInfo);
			if (FAILED(hr)) {
				pPin->Release();
				pEnum->Release();
				return hr;
			}
			_tprintf(TEXT("-->> PIN DIRECTION %s %s\n"), (PinDir == PINDIR_INPUT) ? TEXT("PINDIR_INPUT") : TEXT("PINDIR_OUTPUT"), pinInfo.achName);
			IEnumMediaTypes* pMTEnum = NULL;
			hr = pPin->EnumMediaTypes(&pMTEnum);
			if (FAILED(hr)) {
				pPin->Release();
				pEnum->Release();
				return hr;
			}
			AM_MEDIA_TYPE* mt = NULL;
			while (pMTEnum->Next(1, &mt, 0) == S_OK) {
				PrintAmMediaType(mt);
			}
		}
	}

	static HRESULT GetPin(IBaseFilter* pFilter, PIN_DIRECTION PinDir, IPin** ppPin)
	{
		IEnumPins* pEnum = NULL;
		IPin* pPin = NULL;
		HRESULT    hr;

		if (ppPin == NULL)
		{
			return E_POINTER;
		}

		hr = pFilter->EnumPins(&pEnum);
		if (FAILED(hr))
		{
			return hr;
		}
		while (pEnum->Next(1, &pPin, 0) == S_OK)
		{
			PIN_DIRECTION PinDirThis;
			hr = pPin->QueryDirection(&PinDirThis);
			if (FAILED(hr))
			{
				pPin->Release();
				pEnum->Release();
				return hr;
			}
			if (PinDir == PinDirThis)
			{
				// Found a match. Return the IPin pointer to the caller.
				*ppPin = pPin;
				pEnum->Release();
				return S_OK;
			}
			// Release the pin for the next time through the loop.
			pPin->Release();
		}
		// No more pins. We did not find a match.
		pEnum->Release();
		return E_FAIL;
	}

	static void PrintAmMediaType(AM_MEDIA_TYPE* type) {
		TCHAR formattype[256];
		int cFormatType = StringFromGUID2(type->formattype, formattype, _countof(formattype));
		TCHAR majortype[256];
		int cMajorType = StringFromGUID2(type->majortype, majortype, _countof(majortype));
		TCHAR subtype[256];
		int cSubType = StringFromGUID2(type->subtype, subtype, _countof(subtype));

		_tprintf(TEXT("-->> AM_MEDIA_TYPE \n" \
			"     formattype % s % s\n" \
			"     majortype %s %s\n" \
			"     subtype %s %s\n" \
			"     bFixedSizeSamples %d\n"\
			"     lSampleSize %d\n"\
			"     cbFormat %d\n"),
			formattype, media_type_guid_to_string(type->formattype),
			majortype, media_type_guid_to_string(type->majortype),
			subtype, media_type_guid_to_string(type->subtype),
			type->bFixedSizeSamples,
			type->lSampleSize,
			type->cbFormat);
		if (type->cbFormat > 0 && type->formattype == FORMAT_VideoInfo) {
			VIDEOINFOHEADER* pInfo = reinterpret_cast<VIDEOINFOHEADER*>(type->pbFormat);
			_tprintf(TEXT("     bitrate %d\n"), pInfo->dwBitRate);
			_tprintf(TEXT("     biWidth %d\n"\
				"     biHeight %d\n"\
				"     biPlanes %d\n"\
				"     biSize %d\n"\
				"     biBitCount %d\n"\
				"     biSizeImage %d\n"),
				HEADER(pInfo)->biWidth,
				HEADER(pInfo)->biHeight,
				HEADER(pInfo)->biPlanes,
				HEADER(pInfo)->biSize,
				HEADER(pInfo)->biBitCount,
				HEADER(pInfo)->biSizeImage);
		}
	}

	static LPTSTR media_type_guid_to_string(GUID mediatype) {
		if (mediatype == MEDIATYPE_Video) {
			return TEXT("MEDIATYPE_Video");
		}
		else if (mediatype == MEDIATYPE_Audio) {
			return TEXT("MEDIATYPE_Audio");
		}
		else if (mediatype == FORMAT_VideoInfo) {
			return TEXT("FORMAT_VideoInfo");
		}
		else if (mediatype == MEDIASUBTYPE_NV12) {
			return TEXT("MEDIASUBTYPE_NV12");
		}
		else if (mediatype == MEDIASUBTYPE_YUY2) {
			return TEXT("MEDIASUBTYPE_YUY2");
		}
		else {
			return TEXT("OTHERS");
		}
	}

	static LPCTSTR MediaSubtypeToString(GUID subtype) {
		if (subtype == MEDIASUBTYPE_RGB24) {
			return TEXT("RGB24");
		}
		else if (subtype == MEDIASUBTYPE_RGB32) {
			return TEXT("RGB32");
		}
		else if (subtype == MEDIASUBTYPE_RGB555) {
			return TEXT("RGB555");
		}
		else if (subtype == MEDIASUBTYPE_RGB565) {
			return TEXT("RGB565");
		}
		else if (subtype == MEDIASUBTYPE_YUY2) {
			return TEXT("YUY2");
		}
		else if (subtype == MEDIASUBTYPE_YVYU) {
			return TEXT("YVYU");
		}
		else if (subtype == MEDIASUBTYPE_YUYV) {
			return TEXT("YUYV");
		}
		else if (subtype == MEDIASUBTYPE_IYUV) {
			return TEXT("IYUV");
		}
		else if (subtype == MEDIASUBTYPE_UYVY) {
			return TEXT("UYVY");
		}
		else if (subtype == MEDIASUBTYPE_YV12) {
			return TEXT("YV12");
		}
		else if (subtype == MEDIASUBTYPE_YVU9) {
			return TEXT("YVU9");
		}
		else if (subtype == MEDIASUBTYPE_Y411) {
			return TEXT("Y411");
		}
		else if (subtype == MEDIASUBTYPE_Y41P) {
			return TEXT("Y41P");
		}
		else if (subtype == MEDIASUBTYPE_Y211) {
			return TEXT("Y211");
		}
		else if (subtype == MEDIASUBTYPE_AYUV) {
			return TEXT("AYUV");
		}
		//else if (type == MEDIASUBTYPE_Y800) TEXT("Y800");
		//else if (type == MEDIASUBTYPE_Y8) TEXT("Y8");
		//else if (type == MEDIASUBTYPE_GREY) TEXT("GREY");
		else if (subtype == MEDIASUBTYPE_NV12) {
			return TEXT("NV12");
		}
		else {
			return TEXT("OTHER");
		}
	}

	static inline HRESULT Check(double value, double min, double max) {
		if (value >= min && value <= max) {
			return S_OK;
		}
		_tprintf(TEXT("%f < %f or %f > %f"), value, min, value, max);
		return E_FAIL;
	}

	static inline double Clamp(double value, double min, double max) {
		if (value < min) {
			return min;
		}
		if (value > max) {
			return max;
		}
		return value;
	}

	static inline void YUVfromRGB(double& Y, double& U, double& V, const double R, const double G, const double B)
	{
		Y = 0.257 * R + 0.504 * G + 0.098 * B + 16;
		U = -0.148 * R - 0.291 * G + 0.439 * B + 128;
		V = 0.439 * R - 0.368 * G - 0.071 * B + 128;
		Clamp(Y, 0, 255);
		Clamp(U, 0, 255);
		Clamp(V, 0, 255);
	}

	static void RGBfromYUV(double& R, double& G, double& B, double Y, double U, double V)
	{
		Check(Y, 0, 255);
		Check(U, 0, 255);
		Check(V, 0, 255);
		Y -= 16;
		double Cb = U - 128;
		double Cr = V - 128;
		R = 1.164 * Y + 1.596 * Cr;
		G = 1.164 * Y - 0.183 * Cr - 0.392 * Cb;
		B = 1.164 * Y + 2.017 * Cb;

		R = Clamp(R, 0, 255);
		G = Clamp(G, 0, 255);
		B = Clamp(B, 0, 255);
	}

	static HRESULT NV12ToRGB24(UCHAR* rgb24, ULONG rgb24Size, UCHAR* nv12, ULONG nv12Size, int width, int height, bool verticalFlip = false) {
		if (width <= 0 || height <= 0 || rgb24 == NULL || nv12 == NULL || width % 2 != 0 || height % 2 != 0) {
			return E_FAIL;
		}
		if (rgb24Size != width * height * 3) {
			return E_FAIL;
		}
		if (nv12Size != width * height * 3 / 2) {
			return E_FAIL;
		}

		for (int j = 0; j < height / 2; j++) { // Row
			for (int i = 0; i < width / 2; i++) { // Column
				DOUBLE U = nv12[width * height + j * width + i * 2];
				DOUBLE V = nv12[width * height + j * width + i * 2 + 1];
				for (int k = 0; k < 2; k++) {
					for (int l = 0; l < 2; l++) {
						int yPos = (2 * j + l) * width + 2 * i + k;
						DOUBLE Y = nv12[yPos];
						DOUBLE R, G, B;
						RGBfromYUV(R, G, B, Y, U, V);
						if (verticalFlip) {
							yPos = (height - 1 - 2 * j - l) * width + 2 * i + k;
						}
						rgb24[yPos * 3] = (UCHAR)R;
						rgb24[yPos * 3 + 1] = (UCHAR)G;
						rgb24[yPos * 3 + 2] = (UCHAR)B;
					}
				}
			}
		}
		return S_OK;
	}

	static UCHAR* NV12ToRGB24(UCHAR* data, int width, int height) {
		int sizeRgb24 = width * height * 3;
		int sizeNv12 = width * height * 3 / 2;
		UCHAR* rgb24 = new UCHAR[sizeRgb24];
		if (rgb24) {
			VIUtils::NV12ToRGB24(rgb24, sizeRgb24, data, sizeNv12, width, height);
		}
		return rgb24;
	}

	static HRESULT SaveToFile(LPCTSTR fileName, UCHAR* data, size_t cbSize) {
		if (!fileName || !data || cbSize == 0) {
			return E_FAIL;
		}
		FILE* f = _tfopen(fileName, TEXT("wb"));
		if (!f) {
			return E_FAIL;
		}
		fwrite(data, sizeof(UCHAR), cbSize, f);
		fflush(f);
		fclose(f);
		return S_OK;
	}

	static HRESULT SetYUV(UCHAR* data, int width, int height, UCHAR y, UCHAR u, UCHAR v) {
		if (!data || width <= 0 || height <= 0 || width % 2 != 0 || height % 2 != 0) {
			return E_FAIL;
		}
		memset(data, y, width * height);
		for (int i = 0; i < width * height / 4; i++) {
			data[width * height + 2 * i] = u;
			data[width * height + 2 * i + 1] = v;
		}
		return S_OK;
	}

	static UCHAR* UVByY(int& width, int& height, UCHAR y) {
		width = 512;
		height = 512;
		UCHAR* yuv = new UCHAR[width * height * 3 / 2];
		memset(yuv, 0, width * height * 3 / 2);
		memset(yuv, y, width * height);

		int offset = width * height;
		for (int j = 0; j < height / 2; j++) {
			for (int i = 0; i < width / 2; i++) {

				int uPos = j * width + i * 2;
				int vPos = j * width + i * 2 + 1;

				yuv[offset + uPos] = j;
				yuv[offset + vPos] = i;
			}
		}

		return yuv;
	}

	static UCHAR* TestCreateAllYuv(int& width, int& height) {
		width = 4096;
		height = 4096;
		UCHAR* yuv = new UCHAR[width * height * 3 / 2];
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				yuv[j * width + i] = (UCHAR)(i % 16 + (j % 16) * 16);
			}
		}
		for (int i = 0; i < width * height / 2; i++) {
			yuv[width * height + i] = UCHAR(128);
		}
		return yuv;
	}

	static HRESULT TestSetYuv(UCHAR* data, int width, int height) {
		if (!data || width <= 0 || height <= 0 || width % 2 != 0 || height % 2 != 0) {
			return E_FAIL;
		}
		for (int j = 0; j < height; j++) {
			for (int i = 0; i < width; i++) {
				UCHAR Y;
				if (i < width / 2 && j < height / 2) {
					Y = WHITE[0];
				}
				else if (i < width / 2 && j >= height / 2) {
					Y = RED[0];
				}
				else if (i >= width / 2 && j < height / 2) {
					Y = GREEN[0];
				}
				else {
					Y = BLUE[0];
				}
				data[j * width + i] = Y;
			}
		}

		for (int j = 0; j < height / 2; j++) {
			for (int i = 0; i < width / 2; i++) {
				UCHAR U, V;
				if (i < width / 4 && j < height / 4) {
					U = WHITE[1];
					V = WHITE[2];
				}
				else if (i < width / 4 && j >= height / 4) {
					U = RED[1];
					V = RED[2];
				}
				else if (i >= width / 4 && j < height / 4) {
					U = GREEN[1];
					V = GREEN[2];
				}
				else {
					U = BLUE[1];
					V = BLUE[2];
				}
				data[width * height + j * width + 2 * i] = U;
				data[width * height + j * width + 2 * i + 1] = V;
			}
		}
		return S_OK;
	}

	static inline HRESULT SetYUV(UCHAR* dst, int dstWidth, int dstHeight, UCHAR* src, int srcWidth, int srcHeight, int posX, int posY) {
		if (!dst || dstWidth <= 0 || dstHeight <= 0) {
			return E_FAIL;
		}
		if (!src || srcWidth <= 0 || srcHeight <= 0) {
			return E_FAIL;
		}
		if (posX % 2 != 0 || posY % 2 != 0) {
			return E_FAIL;
		}
		int dstOffset = dstWidth * dstHeight;
		int srcOffset = srcWidth * srcHeight;
		for (int srcJ = 0; srcJ < srcHeight; srcJ++) {
			for (int srcI = 0; srcI < srcWidth; srcI++) {
				int dstX = posX + srcI;
				int dstY = posY + srcJ;
				if (dstX >= dstWidth) {
					continue;
				}
				if (dstY >= dstHeight) {
					break;
				}
				dst[dstY * dstWidth + dstX] = src[srcJ * srcWidth + srcI];
				if (srcI % 2 == 0 && srcJ % 2 == 0) {
					dst[dstOffset + dstY / 2 * dstWidth + dstX / 2 * 2] = src[srcOffset + srcJ / 2 * srcWidth + srcI / 2 * 2];
					dst[dstOffset + dstY / 2 * dstWidth + dstX / 2 * 2 + 1] = src[srcOffset + srcJ / 2 * srcWidth + srcI / 2 * 2 + 1];
				}
			}
		}
		return S_OK;
	}

	static inline void SetYUV(UCHAR* data, int width, int height, int offsetX, int offsetY, UCHAR Y, UCHAR U, UCHAR V) {
		data[offsetY * width + offsetX] = Y;
		data[width * height + offsetY * width / 2 + offsetX / 2 * 2] = U;
		data[width * height + offsetY * width / 2 + offsetX / 2 * 2 + 1] = V;
	}

	static HRESULT FindYUVBound(UCHAR* data, int width, int height, int& yMin, int& yMax, int& uMin, int& uMax, int& vMin, int& vMax) {
		if (!data || width <= 0 || height <= 0) {
			return E_FAIL;
		}
		yMin = uMin = vMin = 256;
		yMax = uMax = vMax = -1;
		for (int i = 0; i < width * height; i++) {
			if (data[i] < yMin) {
				yMin = data[i];
			}
			if (data[i] > yMax) {
				yMax = data[i];
			}
		}

		return S_OK;
	}
};

class FrameSaver {
public:

	static HRESULT Save(UCHAR* frame, ULONG cbSize) {
		LPCTSTR yuvPath = TEXT("D:\\Projects\\3.yuv");
		LPCTSTR rgb24Path = TEXT("D:\\Projects\\3.rgb24");
		if (frameCount == 0) {
			if (f) {
				fflush(f);
				fclose(f);
				f = NULL;
			}
			f = _tfopen(yuvPath, TEXT("wb"));
			_tprintf(TEXT("Frame %d: Create yuv file %s\n"), frameCount, yuvPath);
			fRgb24 = _tfopen(rgb24Path, TEXT("wb"));
			_tprintf(TEXT("Frame %d: Create rgb24 file %s\n"), frameCount, rgb24Path);
		}

		if (frameCount < 30) {
			if (f) {
				UCHAR* tmp = new UCHAR[cbSize];
				memcpy(tmp, frame, cbSize);
				fwrite(tmp, sizeof(unsigned char), cbSize, f);
				fflush(f);
				delete[] tmp;
				tmp = NULL;
				_tprintf(TEXT("Frame %d: actual is rgb24 Saved size %d\n"), frameCount, cbSize);

				/*	ULONG sizeRgb24 = 1920 * 1080 * 3;
					UCHAR* rgb24 = new UCHAR[sizeRgb24];
					VIUtils::NV12ToRGB24(rgb24, sizeRgb24, frame, cbSize, 1920, 1080);
					fwrite(rgb24, sizeof(UCHAR), sizeRgb24, fRgb24);
					_tprintf(TEXT("Frame %d: Save frame to %s\n"), frameCount, rgb24Path);
					delete[] rgb24;*/
			}
		}
		else if (frameCount == 30) {
			if (f) {
				fflush(f);
				fclose(f);
				_tprintf(TEXT("Frame %d: Close file\n"), frameCount);
				f = NULL;
			}
		}
		else {

		}
		frameCount++;
		return S_OK;
	}
private:
	static ULONG frameCount;
	static FILE* f;
	static FILE* fRgb24;
};

#endif
