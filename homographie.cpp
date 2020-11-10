#include <opencv2/highgui/highgui.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace std;
using namespace cv;


int main()
{
	Mat I1 = imread("../IMG_0045.JPG", IMREAD_GRAYSCALE); //Load as grayscale
	Mat I2 = imread("../IMG_0046.JPG", IMREAD_GRAYSCALE); //Load as grayscale
	// Mat I2 = imread("../IMG_0046r.JPG", IMREAD_GRAYSCALE); //Load as grayscale
	
	imshow("Image 1", I1); waitKey(0);
	imshow("Image 2", I2); waitKey(0);

	// The goal of is to create a panorama creation pipeline from 2 images, using opencv functions
	// the documentation is available at https://docs.opencv.org/master, mostly in the feature2D and calib3d modules.

    	cout << "AKAZE INITIALIZATION" << endl;
	Ptr<AKAZE> D = AKAZE::create(); 			// we create AKAZE
	vector<KeyPoint> m1, m2; 				// mi will store a keypoint found by a keypoint detector (Harris corner detec, SIFT, ..)

	Mat J1, J2; 							
	D->detectAndCompute(I1, noArray(), m1, J1); 		// we detect and compute AKAZE keypoints (m1) and descriptors (J1)
	D->detectAndCompute(I2, noArray(), m2, J2); 		// We detect and compute AKAZE keypoints (m2) and descriptors (J2)
	
    	drawKeypoints(I1, m1, J1); 				// we add the results to image
    	drawKeypoints(I2, m2, J2); 				// we add the results to image
	
	imshow("Akaze result on image 1", J1); waitKey(0);
	imshow("Akaze result on image 2", J2); waitKey(0);
	
	imwrite("akaze_result_I1.jpg", J1); 			// we save the image
    	imwrite("akaze_result_I2.jpg", J2); 			// we save the image


    	cout << "BFMatcher" << endl;

	Mat desc1, desc2;
    	D->detectAndCompute(I1, noArray(), m1, desc1);
    	D->detectAndCompute(I2, noArray(), m2, desc2);
								
	Ptr<BFMatcher> matcher = BFMatcher::create();		// takes the desc of 1 feat in set1 and is matched with all other features in set2 using some dist calc & the closest one is returned
	vector<DMatch> vector_match;
	matcher->DescriptorMatcher::match(desc1, desc2, vector_match);
	
	Mat res1;
    	drawMatches(I1, m1, I2, m2, vector_match, res1);
    	imshow("BFMatcher: feature correspondances",res1); waitKey(0);
    	imwrite("feature_correspondances_result.png", res1);


	// Get the keypoints from the good matches
   	vector<Point2f> matches1, matches2;
    	for (DMatch good_matches : vector_match){
        	Point2f p1(m1[good_matches.DMatch::queryIdx].pt);
        	Point2f p2(m2[good_matches.DMatch::trainIdx].pt);
        	matches1.push_back(p1);
        	matches2.push_back(p2);
    	}

    	Mat mask_input;								// mask input to draw the inlier matches
    	Mat H = findHomography(matches2, matches1, RANSAC, 3, mask_input); 	// find the transform between matched keypoints

    	Mat res2;
    	drawMatches(I1, m1, I2, m2, vector_match, res2, Scalar::all(-1), Scalar::all(-1), mask_input);
    	imshow("Inliers after RANSAC selection", res2); waitKey(0);
    	imwrite("inliers_after_RANSAC_selection.png", res2);


    	Mat K(I1.rows, 1.5*I1.cols, CV_8U); Mat K1(I1.rows, 1.5*I1.cols, CV_8U); Mat K2(I1.rows, 1.5*I1.cols, CV_8U);

    	warpPerspective(I1, K1, Mat::eye(3,3, CV_32F), K1.size());
    	warpPerspective(I2, K2, H, K2.size());
    	K = max(K1, K2);

    	imshow("Panorama", K); waitKey(0);
    	imwrite("panorama.png", K);

	
	waitKey(0);
	return 0;
}