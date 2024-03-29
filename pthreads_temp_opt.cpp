
//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>
#include<vector>
#include<pthread.h>
#include<fstream>
#include<chrono>
using namespace cv;
using namespace std;
using namespace std::chrono;
// returns the queue density in frame, by subracting the background image

int thread_num=3;

struct dyanamic_params{
    Mat nxt ; Mat prv ;
    double area;
    //vector<double> &v;
};

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


void *f1(void *x)
{
    dyanamic_params *z = (dyanamic_params *)x;
    double area = dynamic_density(z->nxt, z->prv);
    double *val ;
    val = &area;
    z->area=area;
    
    pthread_exit(NULL);
}


vector<double> run_dynamic_density(VideoCapture cap, Mat img,Mat h)
{
    Mat frame,frame_2,prvs[thread_num+1] ,cropped;

    pthread_t threads[thread_num];
    vector<double> vals;

    Rect roi(831,211,544,867);

    cap >> frame ;
    warpPerspective(frame,frame_2, h,Size(1920,1080));
    frame = frame_2(roi);
    cvtColor(frame, frame, COLOR_BGR2GRAY);
    prvs[0]=frame.clone();   // first frame is projected and cropped and stored in prvs matrix


    int cnt = 0 ; // denotes the frame number
    double time;  // denotes time stamp of frame
    
    while(true)
    {
      dyanamic_params parts[thread_num];
        
       for(int i=0;i<thread_num;i++){
           cap >> frame ;
           if(frame.empty()) return vals ;
           cap >> frame ;
           if(frame.empty()) return vals ;
           cap >> frame ;
           if(frame.empty()) return vals ;
           
           warpPerspective(frame,frame_2, h,Size(1920,1080));
           cropped = frame_2(roi);
           cvtColor(cropped, cropped, COLOR_BGR2GRAY);
           
           parts[i] = {cropped.clone(),prvs[i]};
           prvs[i+1]=cropped.clone();
           
           int l = pthread_create(&threads[i] , NULL , f1 , (void *)(&parts[i]));
           
           if(l){
               cout << "could not make thread" ;
               exit(-1);
           }
           
       }
       cnt+=3*thread_num;
       time=cnt/15;
       
       double area=0.0;
       
       for(int i=0;i<thread_num;i++){
           pthread_join(threads[i] , NULL);
           vals.push_back(parts[i].area);
       }
        
       prvs[0]=prvs[thread_num].clone();

    }
    return vals ;

}


void generate(string vid, Mat img,Mat h)
{
    VideoCapture cap1(vid);
    vector<double> v1 = run_dynamic_density(cap1 , img , h) ;

    int cnt = 1 ;
    for(auto x : v1)
    {
        cout << cnt << "," << x << "\n";
        cnt++;
    }
}


int main(int argc, char** argv)
{
    auto start = high_resolution_clock::now();
    
    string vid = argv[1];
    thread_num = atoi(argv[2]);
    Mat image_proj;
    Mat image_src = imread("empty2.jpg" , IMREAD_GRAYSCALE);  // background image used in queue density, it has been extracted from the video itself

    vector<Point2f> pts_src,pts_dst;
    pts_src={Point2f(985,245),Point2f(1292,255),Point2f(1520,1056),Point2f(349,1055)};   //hardcoded the boundary points of road
    pts_dst={Point2f(831,211),Point2f(1375,211),Point2f(1375,1078),Point2f(831,1078)};
    Mat h = findHomography(pts_src,pts_dst);  // returns the homographic matrix
    
    warpPerspective(image_src,image_proj, h,Size(1920,1080));   // transformed image (1920x1080)
    
    Rect roi(831,211,544,867);
    Mat bg = image_proj(roi); // cropped image (544x867)
 
    // VideoCapture cap(vid);
   
    freopen("out_3.txt","w",stdout); // file in which data will be written
    cout<<"frame,dynamic density"<<"\n";

    generate(vid,bg,h);  // function to generate data
    //out.close();
    // fclose(stdout);
    auto stop = high_resolution_clock::now();
    
    
    auto duration = duration_cast<microseconds>(stop - start);
    
   // cout << duration.count();
    return 0;
}









