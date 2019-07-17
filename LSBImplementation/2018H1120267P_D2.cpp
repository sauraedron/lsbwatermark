/*
	Author : Saurabh A. Wankhade
	ID : 2018H1120267P
	LSB Watermarking.
	Notes: This code represents a possible implementation of "LSB based digital image watermarking for gray scale image" by Sanjay Gaur.
	Paper Link :https://www.researchgate.net/profile/Sanjay_Gaur/publication/280055810_F0613641/links/55a5e24908ae00cf99c98c75.pdf
*/

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>
#include <iostream>
#include <bitset>
#include <string>
#include <fstream>

using namespace std;
using namespace cv;


// Turns N signficants bits to zero
int n_sig_to_zero(int number, int n)
{
	number = number >> n;
	number = number << n;
	return number;
	number &= ~((1 << n) - 1);
}


int shift_right_by_8_minus_n(int number, int n)
{
	number = number >> (8 - n);
	return number;
}

int shift_left_by_8_minus_n(int number, int n)
{
	return (number << (8 - n));
}

class InputImage
{
protected:
	Mat original_image;
	Mat watermark_image;
	int bit_to_modify;
	
	InputImage(string oi_path, string wi_path, int bit)
	{
		original_image = imread(oi_path);
		watermark_image = imread(wi_path);
		if (original_image.rows != watermark_image.rows || original_image.cols != watermark_image.cols)
		{
			cout << "Original image and Watermark image are not of same size." << endl;
			cout << "************************" << endl;
			cout<<"EXITING"<<endl;
			cout << "************************" << endl;
			exit(-1);
		}
		bit_to_modify = bit;
	}
public:
	string recoveredImage;
	virtual Mat processWatermark(int i) = 0;
	virtual void saveDifference(string original, string wmked, int i) = 0;
	virtual void ExtractWatermark(string wm_image) = 0;
};

class GrayScaleWaterMarking : public InputImage
{
public:
	GrayScaleWaterMarking(string oi_path, string wi_path, int bit) : InputImage(oi_path, wi_path, bit)
	{
		cout << "\t Converting to GrayScale" << endl;
		ConvertToGrayScale(original_image);
		ConvertToGrayScale(watermark_image);
	}
	////Convert image to grayscale as the Paper mentions only about grayscale images.
	void ConvertToGrayScale(Mat &img)
	{
		cvtColor(img, img, COLOR_RGB2GRAY);
		cout << "\t\t GrayScale conversion complete." << endl;
	}
public:
	Mat processWatermark(int bit_to_set)
	{
		cout << "\t Beginning ProcessWatermark" << endl;
		Mat result(original_image.rows, original_image.cols, 0);
		original_image.copyTo(result);/*
		cout << result.channels();*/
		//Iterate over all rows an columns getting the pixel value and performing appropriate steps.
		cout << "\t Iterating over pixels"<<endl;
		for (int i = 0; i < original_image.rows; i++)
		{
			for (int j = 0; j < original_image.cols; j++)
			{
				uchar oi_pixel = original_image.at<uchar>(i, j);
				uchar wi_pixel = watermark_image.at<uchar>(i, j);
				int base_pixel = n_sig_to_zero((int)oi_pixel, bit_to_set);
				int base_wi_pixel = shift_right_by_8_minus_n((int)wi_pixel, bit_to_set);

				//Bug hunt:  are pixels out of bound ?
				if ((base_pixel + base_wi_pixel) > 255)
					cout << "Out of bounds";
				result.at<uchar>(i, j) = (uchar)(base_pixel + base_wi_pixel);
			}
		}
		cout << "\t Iterating over pixels complete" << endl;
		return result;
	}
	void saveDifference(string a, string b, int i)
	{
		Mat og = imread(a);
		Mat wm = imread(b);
		Mat diff;
		og.copyTo(diff);
		for (int i = 0; i < og.rows; i++)
		{
			for (int j = 0; j < og.cols; j++)
			{
				//cout << j;
				uchar coverPixel = og.at<uchar>(i, j);
				uchar Wmpixel = wm.at<uchar>(i, j);
				diff.at<uchar>(i, j) = (uchar)abs((int)Wmpixel - (int)coverPixel);
			}

		}
		string diffF = "results//" + to_string(i) + "_diff.bmp";
		imwrite(diffF, diff);
	}
	void ExtractWatermark(string watermarked_image_path)
	{
		cout << "\t Processing ExtractWatermark" << endl;
		Mat watermarked_image = imread(watermarked_image_path);
		ConvertToGrayScale(watermarked_image);
		Mat recovered_watermark(watermarked_image.rows, watermarked_image.cols, 0);
		cout << "\t Processing pixels for watermark image" << endl;
		for (int i = 0; i < watermarked_image.rows; i++)
		{
			for (int j = 0; j < watermarked_image.cols; j++)
			{
				uchar watermarked_image_pixel = watermarked_image.at<uchar>(i, j);
				recovered_watermark.at<uchar>(i, j) = (uchar)shift_left_by_8_minus_n((int)watermarked_image_pixel, bit_to_modify);
			}
		}
		cout << "\t Writing recovered watermark image to " << recoveredImage<< endl;
		imwrite(recoveredImage, recovered_watermark);
	}
};

string make_path_recognizable(string s)
{
	auto pos = s.find("\\");
	while (s.find("\\") != string::npos)
	{
		s.replace(pos, pos + 1, "\\\\");
		pos += 4;
	}
	return s;
}

int main(int args, char **argv)
{
	string original_image_path = "dataset\\2018H1120267P_ip1.bmp";
	string fileName = original_image_path.substr(original_image_path.find_last_of("_") + 1);
	string watermark_image_path = "dataset\\2018H1120267P_ip3.jpg";
	string outputFolder = "results\\";
	/*if (args == 1)
	{
		cout << "No input specified\r\n";
		return 0;
	}
	else
	{
		for (int i = 1; i < args; i++)
		{
			string argumentPassed = argv[i];
			if (argumentPassed.find("-i") != string::npos)
			{
				original_image_path = argumentPassed.substr(argumentPassed.find_first_of("-i") + 3);
			}
			else if (argumentPassed.find("-w") != string::npos)
			{
				watermark_image_path = argumentPassed.substr(argumentPassed.find_first_of("-w") + 3);
			}
			else if (argumentPassed.find("-o") != string::npos)
			{
				outputFolder = argumentPassed.substr(argumentPassed.find_first_of("-o") + 3);
			}
		}
	}*/
	//Both images original and cover should be of same size. 
	//Images can be found in 2018H1120267P_D2\LSBImplementation\dataset
	//2018H1120267P_ip3 is the watermark image
	//string original_image_path = "dataset\\2018H1120267P_ip2.bmp";
	
	fileName[0] = 'o';
	cout << "Beginning Watermarking" << endl;
	InputImage *img = new GrayScaleWaterMarking(original_image_path, watermark_image_path, 1);
	img->recoveredImage = outputFolder + "\\2018H1120267P_D2_recovered_WM.bmp";
	
	auto result = img->processWatermark(1);
	string resultant_wm_image = outputFolder +"2018H1120267P_"+fileName;
	try
	{
		cout << "Writing resultant watermark image to " << resultant_wm_image << endl;
		imwrite(resultant_wm_image, result);
	}
	catch (const std::exception& e)
	{
		cout << "Caught exception" << e.what();
	}
	cout << "Beginning Watermark extraction" << endl;
	cout << "Setting recovered Image to " << img->recoveredImage;
	img->ExtractWatermark(resultant_wm_image);
	delete(img);
}

