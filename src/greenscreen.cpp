// greenscreen.cpp : Defines the entry point for the console application.
//

#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;



int low_h = 63, low_s = 200, low_v = 100;
int high_h = 66, high_s = 255, high_v = 200;

Mat alphaBlend(Mat foreground, Mat background, Mat alpha);

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        cout << " Usage: greenscreen VideoToProcess ImageToLoadAsBackground" << endl;
        return -1;
    }
    Mat image;
    VideoCapture video(argv[1]);
    image = imread(argv[2], IMREAD_COLOR); // Read the file
    if (image.empty()) // Check for invalid input
    {
        cout << "Could not open or find the image" << std::endl;
        return -1;
    }
    if (!video.isOpened()) // Check for invalid input
    {
        cout << "Could not open or find the video" << std::endl;
        return -1;
    }
    namedWindow("Original Video", WINDOW_AUTOSIZE); // Create a window for display.
    namedWindow("Mask", WINDOW_AUTOSIZE); // Create a window for display.
    namedWindow("Output", WINDOW_AUTOSIZE); // Create a window for display.

    while ((char)waitKey(1) != 'q') {
        Mat frame;
        video >> frame;
        Mat mask;

        cvtColor(frame, frame, COLOR_BGR2HSV);
        inRange(frame, Scalar(low_h, low_s, low_v), Scalar(high_h, high_s, high_v), mask);
        cvtColor(frame, frame, COLOR_HSV2BGR);

        Mat output = alphaBlend(frame, image, mask);

        imshow("Original Video", frame); // Show our image inside it.
        imshow("Mask", mask); // Show our image inside it.
        imshow("Output", output); // Show our image inside it.
    }

    destroyAllWindows();
    return 0;
}

Mat alphaBlend(Mat foreground, Mat background, Mat alpha)
{
    int depth = foreground.depth();
    int type = foreground.type();
    Mat outImage = foreground.clone();
    // Convert Mat to float data type
    foreground.convertTo(foreground, CV_32FC3);
    background.convertTo(background, CV_32FC3);
    outImage.convertTo(outImage, CV_32FC3);

    cvtColor(alpha, alpha, COLOR_GRAY2BGR);

    // Normalize the alpha mask to keep intensity between 0 and 1
    alpha.convertTo(alpha, CV_32FC3, 1.0 / 255); // 

    int a = alpha.channels();
    int fg = foreground.channels();
    int bg = background.channels();
    // Find number of pixels. 
    int numberOfPixels = foreground.rows * foreground.cols * foreground.channels();

    // Get floating point pointers to the data matrices
    float* fptr = reinterpret_cast<float*>(foreground.data);
    float* bptr = reinterpret_cast<float*>(background.data);
    float* aptr = reinterpret_cast<float*>(alpha.data);
    float* outImagePtr = reinterpret_cast<float*>(outImage.data);

    // Loop over all pixesl ONCE
    for (
        int i = 0;
        i < numberOfPixels;
        i++, outImagePtr++, fptr++, aptr++, bptr++
        )
    {
        *outImagePtr = (*fptr)*(1 - *aptr) + (*bptr)*(*aptr);
    }
    outImage.convertTo(outImage, 16);
    return outImage;
}