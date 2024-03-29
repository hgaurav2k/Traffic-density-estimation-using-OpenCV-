//Uncomment the following line if you are compiling this code in Visual Studio
//#include "stdafx.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <algorithm>

using namespace cv;
using namespace std;

vector<Point2f> pts_src,pts_dst;

Mat image_src ; // original image
Mat image_proj ; // image after homogographic projection;
Mat image_crop ; // cropped image
Mat image_src_1;
string window_src  , window_proj, window_crop ;

void callbackfnc(int event,int x,int y,int flags,void *userdata)
{
    if(flags == (EVENT_FLAG_CTRLKEY + EVENT_FLAG_LBUTTON)){
        pts_src.clear();
        image_src_1=image_src.clone();
        imshow(window_src,image_src_1);
    }
    else if(event == EVENT_LBUTTONDOWN && (pts_src.size()<4)){
        // clicked points
        pts_src.push_back(Point2f(x,y));
        circle(image_src_1 , Point(x,y) , 16 , Scalar(0,0,0) ,FILLED, 8) ; // to draw a circle at the clicked points
        imshow(window_src,image_src_1);
    }

}

/*sort the pts in anti-clockwise order starting from top-left pt */
void sort_inputpts(vector<Point2f> &pts_src)
{
    sort(pts_src.begin(),pts_src.end(),[](const cv::Point2f &a, const cv::Point2f &b){
        return a.y<b.y ;
    });        // sort pts by y co-ordinate
    
    if(pts_src[0].x>pts_src[1].x){
        swap(pts_src[0],pts_src[1]);
    }
    if(pts_src[2].x<pts_src[3].x){
        swap(pts_src[2],pts_src[3]);
    }
}

/* to display and save the image*/
bool display_and_save( string window_name , Mat image)
{
    imshow(window_name , image);
    waitKey(0);
    destroyWindow(window_name);
    return imwrite(window_name + string(".jpg") , image);
    
}
/* printing error message*/
void save_error(string name )
{
    cout<<"The image "+ name + " could not be saved\n" ;
    cin.get();
}
int main(int argc, char** argv)
{
    if(!(string(argv[1]).compare(string("empty")) == 0 or string(argv[1]).compare(string("traffic")) == 0 ))
    {
        cout << "Run the executable using command "" make run args=filename"". Keep in mind absence of spaces on both the sides of the '=' sign.";
        cin.get(); return -1 ;
    }
    image_src = imread(string(argv[1])+string(".jpg") , IMREAD_GRAYSCALE);
   // image_src_1 = imread(string(argv[1])+string(".jpg") , IMREAD_GRAYSCALE);
    image_src_1=image_src.clone();
    if(image_src.empty())
    {
        cout<<"Source image not found"<<endl;
        cin.get();  return -1;
    }

    window_src = "Original "+string(argv[1]) ;  window_proj= "Transformed "+string(argv[1]) ; window_crop = "Cropped "+string(argv[1]);

    namedWindow(window_src);
    
    setMouseCallback(window_src,callbackfnc,NULL);
    imshow(window_src,image_src_1);
    waitKey(0);
    //for(Point2f p : pts_src) circle(image_src , Point(p.x,p.y) , 16 , Scalar(255,0,0) ,FILLED, 8) ; // to draw a circle at the clicked points
    destroyWindow(window_src);
   
   
    pts_dst.push_back(Point2f(831,211));        // destination points where the corners of main road are mapped to, better points can be chosen
    pts_dst.push_back(Point2f(1375,211));
    pts_dst.push_back(Point2f(1375,1078));
    pts_dst.push_back(Point2f(831,1078));
    
    sort_inputpts(pts_src);     // sort the inputs in anti-clockwise order starting from top-left point
    
    Mat h =findHomography(pts_src,pts_dst);  // returns the homographic matrix
    warpPerspective(image_src,image_proj, h,Size(1920,1080));   // transformed image (1920x1080)
    
    if( !display_and_save(window_proj,image_proj) ) {
        save_error(window_proj) ; return -1;
    }
    
    Rect roi(831,211,544,867);              // cropped image (544x867)
    image_crop = image_proj(roi);
    
    if( !display_and_save(window_crop,image_crop) ) {
        save_error(window_crop) ; return -1;
    }
    
    return 0;
}


