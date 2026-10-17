#pragma once

#include <opencv2/opencv.hpp>

class CourtLinePixelDetector
{
public:
  struct Parameters
  {
    Parameters();
    // Maximum contrast allowed between samples on opposite sides of a candidate line.
    uchar threshold;
    // Minimum center-to-side contrast required on both sides of a candidate line.
    uchar diffThreshold;
    unsigned int t;
    int gradientKernelSize;
    int kernelSize;
    // Ignore high-frame contrast edges that usually belong to walls, banners, and ceiling.
    double minCourtLineYRatio;
  };

  CourtLinePixelDetector();

  CourtLinePixelDetector(Parameters p);

  cv::Mat run(const cv::Mat& frame);

  static bool debug;
  static const std::string windowName;

private:
  cv::Mat getLuminanceChannel(const cv::Mat& frame);

  cv::Mat getContrastImage(const cv::Mat& frame);

  cv::Mat detectLinePixels(const cv::Mat& image);

  cv::Mat filterLinePixels(const cv::Mat& binaryImage, const cv::Mat& luminanceImage);

  // cv::Mat blurLinePixels(const cv::Mat& image, int ksize=16);

  uchar isLinePixel(const cv::Mat& image, unsigned int x, unsigned int y);

  float pixelDistance(const cv::Mat& image, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) const;

  void computeStructureTensorElements(const cv::Mat& image, cv::Mat& dx2, cv::Mat& dxy, cv::Mat& dy2);

  Parameters parameters;
};
