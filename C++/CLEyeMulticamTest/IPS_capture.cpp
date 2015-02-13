#include "stdafx.h"

// Sample camera capture class
class CLEyeCameraCapture
{
	CHAR _windowName[256];
	GUID _cameraGUID;
	CLEyeCameraInstance _cam;
	CLEyeCameraColorMode _mode;
	CLEyeCameraResolution _resolution;
	float _fps;
	HANDLE _hThread;
	bool _running;
public:
	CLEyeCameraCapture(LPSTR windowName, GUID cameraGUID, CLEyeCameraColorMode mode, CLEyeCameraResolution resolution, float fps) :
		_cameraGUID(cameraGUID), _cam(NULL), _mode(mode), _resolution(resolution), _fps(fps), _running(false)
	{
		strcpy(_windowName, windowName);
	}
	bool StartCapture()
	{
		_running = true;
		cvNamedWindow(_windowName, CV_WINDOW_AUTOSIZE);
		// Start CLEye image capture thread
		_hThread = CreateThread(NULL, 0, &CLEyeCameraCapture::CaptureThread, this, 0, 0);
		if(_hThread == NULL)
		{
			MessageBox(NULL,"Could not create capture thread","CLEyeMulticamTest", MB_ICONEXCLAMATION);
			return false;
		}
		return true;
	}
	void StopCapture()
	{
		if(!_running)	return;
		_running = false;
		WaitForSingleObject(_hThread, 1000);
		cvDestroyWindow(_windowName);
	}

	void Run()
	{
		int w=300, h=100;
		IplImage *pCapImage;
		PBYTE pCapBuffer = NULL;
		// Create camera instance
		_cam = CLEyeCreateCamera(_cameraGUID, _mode, _resolution, _fps);
		if(_cam == NULL)		return;
		// Get camera frame dimensions
		CLEyeCameraGetFrameDimensions(_cam, w, h);
		// Depending on color mode chosen, create the appropriate OpenCV image
		if(_mode == CLEYE_COLOR_PROCESSED || _mode == CLEYE_COLOR_RAW)
			pCapImage = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 4);
		else
			pCapImage = cvCreateImage(cvSize(w, h), IPL_DEPTH_8U, 1);


		// Start capturing
		CLEyeCameraStart(_cam);
		cvGetImageRawData(pCapImage, &pCapBuffer);
		// image capturing loop
		while(_running)
		{
			CLEyeCameraGetFrame(_cam, pCapBuffer);

			cvShowImage(_windowName, pCapImage);
		}
		// Stop camera capture
		CLEyeCameraStop(_cam);
		// Destroy camera object
		CLEyeDestroyCamera(_cam);
		// Destroy the allocated OpenCV image
		cvReleaseImage(&pCapImage);
		_cam = NULL;
	}
	static DWORD WINAPI CaptureThread(LPVOID instance)
	{
		// seed the rng with current tick count and thread id
		srand(GetTickCount() + GetCurrentThreadId());
		// forward thread to Capture function
		CLEyeCameraCapture *pThis = (CLEyeCameraCapture *)instance;
		pThis->Run();
		return 0;
	}
};

/*
int _tmain(int argc, _TCHAR* argv[])
{
	CLEyeCameraCapture *cam[2] = { NULL };
	srand(GetTickCount());
	// Query for number of connected cameras
	int numCams = CLEyeGetCameraCount();
	if(numCams == 0)
	{
		printf("No PS3Eye cameras detected\n");
		return -1;
	}
	printf("Found %d cameras\n", numCams);
	for(int i = 0; i < numCams; i++)
	{
		char windowName[64];
		// Query unique camera uuid
		GUID guid = CLEyeGetCameraUUID(i);	
		cam[i] = new CLEyeCameraCapture(windowName, guid, CLEYE_COLOR_PROCESSED, CLEYE_VGA, 30);
		cam[i]->StartCapture();
	}
	
	CLEyeCameraCapture *pCam = NULL;


	int param = -1, key;
	while((key = cvWaitKey(0)) != 0x1b)
	{
		switch(key)
		{
		case 'e' : break;
		}
	}


	for(int i = 0; i < numCams; i++)
	{
		printf("Stopping capture on camera %d\n", i+1);
		cam[i]->StopCapture();
		delete cam[i];
	}
	return 0;
}
*/