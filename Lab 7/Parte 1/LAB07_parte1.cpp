#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <iostream>
 
using namespace std;
using namespace cv;

void detectAndDisplay( Mat frame );
 
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
 
int main()
{
 
    String face_cascade_name = samples::findFile("haarcascade_frontalface_alt.xml");
    String eyes_cascade_name = samples::findFile("haarcascade_righteye_2splits.xml"); 
 
    if( !face_cascade.load( face_cascade_name ) )
    {
        cout << "--(!)Error loading face cascade\n";
        return -1;
    };
    if( !eyes_cascade.load( eyes_cascade_name ) )
    {
        cout << "--(!)Error loading eyes cascade\n";
        return -1;
    };
     
	
    Mat frame = imread("foto_grupo.jpeg", IMREAD_COLOR);
    
    if( frame.empty() )
    {
       printf("Sem imagem");
       return 0;
	}
 
     //-- 3. Apply the classifier to the frame
     
     detectAndDisplay( frame );
	
     if( waitKey(0) == 27 )
     {
       return 0; // escape
     }
	  
	 int s = waitKey(0);
	 if( s == 's')
	 {
		 imwrite("Lab07_parte1.png", frame);
	 }
    
     return 0;
}

void detectAndDisplay(Mat frame )
{
    Mat frame_gray;
    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );
 
    //-- Detect faces
    std::vector<Rect> faces;
    face_cascade.detectMultiScale( frame_gray, faces );
 
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
        ellipse( frame, center, Size( faces[i].width/2, faces[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 255 ), 4 ); 
 
        Mat faceROI = frame_gray( faces[i] );
 
        //-- In each face, detect eyes
        std::vector<Rect> eyes;
        eyes_cascade.detectMultiScale( faceROI, eyes );
 
        for ( size_t j = 0; j < eyes.size(); j++ )
        {
            Point eye_center( faces[i].x + eyes[j].x + eyes[j].width/2, faces[i].y + eyes[j].y + eyes[j].height/2 );
            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( frame, eye_center, radius, Scalar( 255, 0, 0 ), 4 );
        }
    }
	
    imshow("Display Image./LAB07",frame);
}

