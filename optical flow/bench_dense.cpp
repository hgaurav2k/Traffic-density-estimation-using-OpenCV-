//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include<vector>

using namespace cv;
using namespace std;

// returns the dyanamic density in "nxt" frame, using optical flow method
double dynamic_density( Mat nxt ,Mat prvs )
{
          
        // flow mat is generated , it indicates optical flow from "prvs" to "nxt"
        Mat flow(prvs.size(), CV_32FC2);
        calcOpticalFlowFarneback(prvs, nxt, flow, 0.5, 3, 15, 3, 5, 1.2, 0);

        // flow calculated is visualised in hsv format by assigning them color based on the magnitude and angle of flow vectors
        Mat flow_parts[2];
        split(flow, flow_parts);
        Mat magnitude, angle, magn_norm;
        cartToPolar(flow_parts[0], flow_parts[1], magnitude, angle, true);
        normalize(magnitude, magn_norm, 0.0f, 1.0f, NORM_MINMAX);
        angle *= ((1.f / 360.f) * (180.f / 255.f));
        
        //build the  hsv image
        Mat _hsv[3], hsv, hsv8, bgr,bw,thresh;
         vector<vector < Point>> contours;


        _hsv[0] = angle;
        _hsv[1] = Mat::ones(angle.size(), CV_32F);
        _hsv[2] = magn_norm;
        merge(_hsv, 3, hsv);
        hsv.convertTo(hsv8, CV_8U, 255.0);
        cvtColor(hsv8, bgr, COLOR_HSV2BGR); // hsv image is converted to bgr
        cvtColor(bgr, bw, COLOR_BGR2GRAY);  // bgr image is converted to gray

        threshold(bw,  thresh ,4, 255, THRESH_BINARY); // gray image is converted to black and white image, where all pixels having value more the 4 are white and other are black
        
        erode(thresh,thresh, Mat(), Point(-1, -1), 1, 1, 1);  // thresh image is eroded for better results

        findContours(thresh, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE); // contours are calculated in the eroded image
        double area = 0 ;
     
        Mat x=  Mat::zeros(thresh.rows, thresh.cols, CV_8UC3);
        // area encloded by countors is calculated
        for(int idx = 0 ; idx < contours.size(); idx++)
            {
             if(contourArea(contours[idx])>7000)  // area less than 7000 is mostly noise in image
                {
                    Scalar color( 0, 255, 0);
                    drawContours( x, contours, idx, color, FILLED, 8 );  // filled countours are drawn in frame x, for visualisation of recognised contours
                    area += contourArea(contours[idx]);
                }
        }

//imshow("dynamic_density", x);
      return area/(544*867);  //dyanamic density is returned

}

// function to generate data
void generate(VideoCapture cap, Mat img,Mat h)
{
    Mat frame,frame_2,prvs;
    
    cap >> frame ;
    Rect roi(831,211,544,867);
    warpPerspective(frame,frame_2, h,Size(1920,1080));
    frame = frame_2(roi);
    cvtColor(frame, prvs, COLOR_BGR2GRAY);     // first frame is projected and cropped and stored in prvs matrix

    int cnt = 0 ; // denotes the frame number
    double time;  // denotes time stamp of frame
    while(true)
    {
        Mat nxt;
        cap >> frame ;
        if(frame.empty()) break ;
        cap >> frame ;
        if(frame.empty()) break ;
        cap >> frame ;
        if(frame.empty()) break ;     //in order to shorten the length of video we pick every third frame  ( 15/3 = 5 fps)
        
        warpPerspective(frame,frame_2, h,Size(1920,1080));
        frame_2 = frame_2(roi);
        cvtColor(frame_2, nxt, COLOR_BGR2GRAY); //frame is projected and cropped and converted to gray scale

        cnt+=3;
        time=(double)cnt/15;

        cout<<time<<","<<dynamic_density(nxt, prvs)<<"\n";
        
        prvs = nxt; // frame_2 is coverted to black-white and stored in prvs for optical flow calculation for next iteration

    }
}


int main(int argc, char** argv)
{
    string vid = argv[1];
    Mat image_proj;
    Mat image_src = imread("empty2.jpg" , IMREAD_GRAYSCALE);  // background image used in queue density, it has been extracted from the video itself

    vector<Point2f> pts_src,pts_dst;
    pts_src={Point2f(985,245),Point2f(1292,255),Point2f(1520,1056),Point2f(349,1055)};   //hardcoded the boundary points of road
    pts_dst={Point2f(831,211),Point2f(1375,211),Point2f(1375,1078),Point2f(831,1078)};
    Mat h = findHomography(pts_src,pts_dst);  // returns the homographic matrix
    warpPerspective(image_src,image_proj, h,Size(1920,1080));   // transformed image (1920x1080)
    Rect roi(831,211,544,867);
    Mat bg = image_proj(roi); // cropped image (544x867)
 
    VideoCapture cap(vid);

    freopen("out_dense","w",stdout); // file in which data will be written

    cout<<"time(in secs),dynamic density"<<"\n";

    generate(cap,bg,h);  // function to generate data


    return 0;
}






