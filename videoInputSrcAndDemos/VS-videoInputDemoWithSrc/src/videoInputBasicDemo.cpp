// videoInputBasicDemo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
triangleApp TAPP;

void GLFWCALL keyfun( int key, int action )
{
    if( action != GLFW_PRESS )
    {
        return;
    }

    switch( key )
    {
	    case GLFW_KEY_ESC:
        	printf( "ESC => quit program\n" );
        	break;    	
       
    }

	TAPP.keyDown(key);

    fflush( stdout );
}

void Test() {
    int width = 4096;
    int height = 4096;
    _tprintf(TEXT("-->> Create Start\n"));
    UCHAR* nv12 = VIUtils::TestCreateAllYuv(width, height);
    _tprintf(TEXT("-->> Create End (%d,%d)\n"), width, height);
    ULONG sizeNv12 = width * height * 3 / 2;
    //UCHAR* nv12 = new UCHAR[sizeNv12];

    //VIUtils::SetYuv(nv12, width, height, RED[0], RED[1], RED[2]);
    //VIUtils::SetYuv(nv12, width, height, GREEN[0], GREEN[1], GREEN[2]);
    //VIUtils::SetYuv(nv12, width, height, BLUE[0], BLUE[1], BLUE[2]);
    //VIUtils::TestSetYuv(nv12, width, height);
    
    VIUtils::SaveToFile(TEXT("D:\\Projects\\all.yuv"), nv12, sizeNv12);

    UCHAR* rgb24 = VIUtils::NV12ToRGB24(nv12, width, height);
    VIUtils::SaveToFile(TEXT("D:\\Projects\\all.rgb24"), rgb24, width * height * 3);

    delete[] nv12;
}

void TestNV12ToRGB(UCHAR y) {
    TCHAR name[64];

    int width, height;
    
    _stprintf(name, TEXT("D:\\Projects\\yuv_rgb\\Y_%d_NV12.nv12"), y);
    UCHAR* nv12 = VIUtils::UVByY(width, height, y);
    VIUtils::SaveToFile(name, nv12, width * height * 3 / 2);
    _tprintf(TEXT("Save to %s\n"), name);

    _stprintf(name, TEXT("D:\\Projects\\yuv_rgb\\Y_%d_RGB24.rgb24"), y);
    UCHAR* rgb24 = VIUtils::NV12ToRGB24(nv12, width, height);
    VIUtils::SaveToFile(name, rgb24, width * height * 3);
    _tprintf(TEXT("Save to %s\n"), name);

    _stprintf(name, TEXT("D:\\Projects\\yuv_rgb\\Y_%d_RGBA32.rgba32"), y);
    UCHAR* rgba32 = VIUtils::NV12ToRGBA32(nv12, width, height);
    VIUtils::SaveToFile(name, rgba32, width * height * 4);
    _tprintf(TEXT("Save to %s\n"), name);

    delete[] nv12;
    delete[] rgb24;
    delete[] rgba32;
}

void TestSetYUVBlock() {
    int dstW = 1024;
    int dstH = 1024;
    UCHAR* dst = new UCHAR[dstW * dstH * 3 / 2];
    HRESULT hr = VIUtils::SetYUV(dst, dstW, dstH, BLUE[0], BLUE[1], BLUE[2]);
    int srcW, srcH;
    UCHAR* src = VIUtils::UVByY(srcW, srcH, 88);
    hr = VIUtils::SetYUV(dst, dstW, dstH, src, srcW, srcH, 896, 256);
    if (hr != S_OK) {
        _tprintf(TEXT("SetYUV block failed\n"));
    }
    VIUtils::SaveToFile(TEXT("D:\\Projects\\TestSetYUVBlock_src.yuv"), src, srcW * srcH * 3 / 2);
    VIUtils::SaveToFile(TEXT("D:\\Projects\\TestSetYUVBlock_dst.yuv"), dst, dstW * dstH * 3 / 2);
    delete[] src;
    delete[] dst;
}

void TestGetDeviceIdByName() {
    videoInput::listDevices();
    std::vector<std::string> deviceNames = videoInput::getDeviceList();
    if (deviceNames.size() > 0) {
        for (const auto deviceName : deviceNames) {
            int deviceId = videoInput::getDeviceIDFromName("OBS Virtual Camera");
            printf("Device %s -> %d\n", deviceName.c_str(), deviceId);
            deviceId = videoInput::getDeviceIDFromName(deviceName.c_str());
            printf("Device %s -> %d\n", deviceName.c_str(), deviceId);
        }
    }
}

int main( void )
{
    //Test();
    //TestSetYUVBlock();
    //TestNV12ToRGB(16);

    HANDLE h = GetCurrentThread();
    BOOL result = SetThreadPriority(h, THREAD_PRIORITY_HIGHEST);
    if (result == FALSE) {
        DWORD lastError = GetLastError();
        printf("SetThreadPriority failed %ld", lastError);
    }

    int     width, height, running, frames, x, y;
    double  t, t0, fps;
    char    titlestr[ 200 ];
	

    // Initialise GLFW
    glfwInit();

    // Open OpenGL window    
    if (!glfwOpenWindow(960,540,    // Open window
    24, 24, 24,                                // Red, green, and blue bits for color buffer
    24,                                        // Bits for alpha buffer
    24,                                        // Bits for depth buffer (Z-buffer)
    24,                                        // Bits for stencil buffer
    GLFW_WINDOW)){
        glfwTerminate();
        return 0;
    }
    
    glfwSetKeyCallback( keyfun );
    TAPP.init();
    glfwEnable( GLFW_STICKY_KEYS );
    // Disable vertical sync (on cards that support it)
    glfwSwapInterval( 1 );

    // Main loop
    running = GL_TRUE;
    frames = 0;
    t0 = glfwGetTime();
    while( running ){
    
        // Get time and mouse position
        t = glfwGetTime();
        glfwGetMousePos( &x, &y );
        
        // Calculate and display FPS (frames per second)
        if( (t-t0) > 1.0 || frames == 0 )
        {
            fps = (double)frames / (t-t0);
            sprintf( titlestr, "videoInput Demo App (%.1f FPS)", fps );
            glfwSetWindowTitle( titlestr );
            t0 = t;
            frames = 0;
        }
        frames ++;
        TAPP.idle();
		 
		
        // Get window size (may be different than the requested size)
        glfwGetWindowSize( &width, &height );
        height = height > 0 ? height : 1;
        // Set viewport
        glViewport( 0, 0, width, height );
        // Clear color buffer
        glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
        glClear( GL_COLOR_BUFFER_BIT );

		  TAPP.draw();			
        // Swap buffers
        glfwSwapBuffers();
		
        // Check if the ESC key was pressed or the window was closed
        running = !glfwGetKey( GLFW_KEY_ESC ) &&
                  glfwGetWindowParam( GLFW_OPENED );
    }

    // Close OpenGL window and terminate GLFW
    glfwTerminate();
    return 0;
}
