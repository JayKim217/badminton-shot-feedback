#include "CourtLinePixelDetector.h"
#include "GlobalParameters.h"
#include "DebugHelpers.h"
#include "TimeMeasurement.h"

#include <cmath>

using namespace cv;

bool CourtLinePixelDetector::debug = false;
const std::string CourtLinePixelDetector::windowName = "CourtLinePixelDetector";

CourtLinePixelDetector::Parameters::Parameters()
{
  threshold = 80;
  diffThreshold = 20;
  t = 4;
  gradientKernelSize = 3;
  kernelSize = 21;
  minCourtLineYRatio = 0.40;
}

CourtLinePixelDetector::CourtLinePixelDetector()
  : CourtLinePixelDetector(Parameters())
{

}

CourtLinePixelDetector::CourtLinePixelDetector(CourtLinePixelDetector::Parameters p)
  : parameters(p)
{

}

Mat CourtLinePixelDetector::run(const Mat& frame)
{
  TimeMeasurement::start("CourtLinePixelDetector::run"); 

  TimeMeasurement::start("\tgetLuminanceChannel");
  Mat luminanceImage = getLuminanceChannel(frame);
  TimeMeasurement::stop("\tgetLuminanceChannel");

  TimeMeasurement::start("\tgetContrastImage");
  Mat contrastImage = getContrastImage(frame);
  TimeMeasurement::stop("\tgetContrastImage");

  TimeMeasurement::start("\tdetectLinePixels");
  Mat image = detectLinePixels(contrastImage);
  TimeMeasurement::stop("\tdetectLinePixels");

  TimeMeasurement::start("\tfilterLinePixels");
  image = filterLinePixels(image, luminanceImage);
  TimeMeasurement::stop("\tfilterLinePixels");

  // TimeMeasurement::start("\tblurLinePixels");
  // image = blurLinePixels(image, 16);
  // TimeMeasurement::stop("\tblurLinePixels");

  TimeMeasurement::stop("CourtLinePixelDetector::run");
  return image;
}

// cv::Mat CourtLinePixelDetector::blurLinePixels(const cv::Mat& image, int ksize) {
//   cv::Mat pixelImage[2] = {image, image};
//   int dx[] = {0, 1, 0, -1, 1, 1, -1, -1};
//   int dy[] = {1, 0, -1, 0, 1, -1, 1, -1};

//   for (int k = 0; k < ksize; k++) {
//     for (int x = 0; x < image.cols; x++) {
//       for (int y = 0; y < image.rows; y++) {
//         int nbrs = 0, prev = pixelImage[(k+1)%2].at<uchar>(y,x), curr = 0;
//         for (int i = 0; i < 8; i++) {
//           if (y+dy[i] < 0 || y+dy[i] >= image.rows ||
//               x+dx[i] < 0 || x+dx[i] >= image.cols) {
//             continue;
//           }

//           curr += pixelImage[(k+1)%2].at<uchar>(y+dy[i], x+dx[i]); 
//           nbrs += 1;
//         }
//         pixelImage[k%2] = max(prev, curr / nbrs);
//       }
//     }
//   }

//   if (debug)
//   {
//     displayImage(windowName, pixelImage[(ksize+1)%2]);
//   }

//   return pixelImage[(ksize+1)%2];
// }

cv::Mat CourtLinePixelDetector::getLuminanceChannel(const cv::Mat& frame)
{
  Mat luminanceChannel;
  if (frame.channels() == 1)
  {
    luminanceChannel = frame.clone();
  }
  else
  {
    cvtColor(frame, luminanceChannel, cv::COLOR_BGR2GRAY);
  }

  if (debug)
  {
    displayImage(windowName, frame);
    displayImage(windowName, luminanceChannel);
  }

  return luminanceChannel;
}

cv::Mat CourtLinePixelDetector::getContrastImage(const cv::Mat& frame)
{
  Mat contrastImage;
  if (frame.channels() == 1)
  {
    frame.convertTo(contrastImage, CV_32F);
  }
  else
  {
    Mat bgrImage;
    if (frame.channels() == 4)
    {
      cvtColor(frame, bgrImage, cv::COLOR_BGRA2BGR);
    }
    else
    {
      bgrImage = frame;
    }

    cvtColor(bgrImage, contrastImage, cv::COLOR_BGR2Lab);
    contrastImage.convertTo(contrastImage, CV_32F);
  }

  if (debug)
  {
    displayImage(windowName, contrastImage);
  }

  return contrastImage;
}

Mat CourtLinePixelDetector::detectLinePixels(const cv::Mat& image)
{
  Mat pixelImage(image.rows, image.cols, CV_8UC1, Scalar(GlobalParameters().bgValue));

  for (int x = 0; x < image.cols; ++x)
  {
    for (int y = 0; y < image.rows; ++y)
    {
      if (y < parameters.minCourtLineYRatio * image.rows)
      {
        continue;
      }
      pixelImage.at<uchar>(y,x) = isLinePixel(image, x, y);
    }
  }

  if (debug)
  {
    displayImage(windowName, pixelImage);
  }

  return pixelImage;
}


uchar CourtLinePixelDetector::isLinePixel(const Mat& image, unsigned int x, unsigned int y)
{
  if (x < parameters.t || image.cols - x <= parameters.t)
  {
    return GlobalParameters().bgValue;
  }
  if (y < parameters.t || image.rows - y <= parameters.t)
  {
    return GlobalParameters().bgValue;
  }

  const unsigned int topY = y - parameters.t;
  const unsigned int bottomY = y + parameters.t;
  const unsigned int leftX = x - parameters.t;
  const unsigned int rightX = x + parameters.t;

  const float leftContrast = pixelDistance(image, x, y, leftX, y);
  const float rightContrast = pixelDistance(image, x, y, rightX, y);
  const float verticalSideContrast = pixelDistance(image, leftX, y, rightX, y);

  const float topContrast = pixelDistance(image, x, y, x, topY);
  const float bottomContrast = pixelDistance(image, x, y, x, bottomY);
  const float horizontalSideContrast = pixelDistance(image, x, topY, x, bottomY);

  if (leftContrast > parameters.diffThreshold && rightContrast > parameters.diffThreshold &&
      verticalSideContrast < parameters.threshold)
  {
    return GlobalParameters().fgValue;
  }

  if (topContrast > parameters.diffThreshold && bottomContrast > parameters.diffThreshold &&
      horizontalSideContrast < parameters.threshold)
  {
    return GlobalParameters().fgValue;
  }

  return GlobalParameters().bgValue;
}

float CourtLinePixelDetector::pixelDistance(const Mat& image, unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) const
{
  if (image.channels() == 1)
  {
    return std::abs(image.at<float>(y1,x1) - image.at<float>(y2,x2));
  }

  Vec3f p1 = image.at<Vec3f>(y1,x1);
  Vec3f p2 = image.at<Vec3f>(y2,x2);
  Vec3f diff = p1 - p2;
  return std::sqrt(diff.dot(diff));
}


Mat CourtLinePixelDetector::filterLinePixels(const Mat& binaryImage, const Mat& luminanceImage)
{
  Mat dx2, dxy, dy2;
  computeStructureTensorElements(luminanceImage, dx2, dxy, dy2);
  Mat outputImage(binaryImage.rows, binaryImage.cols, CV_8UC1, Scalar(GlobalParameters().bgValue));
  for (int x = 0; x < binaryImage.cols; ++x)
  {
    for (int y = 0; y < binaryImage.rows; ++y)
    {
      uchar value = binaryImage.at<uchar>(y,x);
      if (value == GlobalParameters().fgValue)
      {
        Mat t(2, 2, CV_32F);
        t.at<float>(0, 0) = dx2.at<float>(y,x);
        t.at<float>(0, 1) = dxy.at<float>(y,x);
        t.at<float>(1, 0) = dxy.at<float>(y,x);
        t.at<float>(1, 1) = dy2.at<float>(y,x);
        Mat l;
        eigen(t, l);
        if (l.at<float>(0,0) > 4 * l.at<float>(0,1))
        {
          outputImage.at<uchar>(y,x) = GlobalParameters().fgValue;
        }
      }
    }
  }

  if (debug)
  {
    displayImage(windowName, outputImage);
  }

  return outputImage;
}

void CourtLinePixelDetector::computeStructureTensorElements(const cv::Mat& image, cv::Mat& dx2, cv::Mat& dxy, cv::Mat& dy2)
{
  Mat floatImage, dx, dy;
  image.convertTo(floatImage, CV_32F);
  GaussianBlur(floatImage, floatImage, Size(5,5), 0);
  Sobel(floatImage, dx, CV_32F, 1, 0, parameters.gradientKernelSize);
  Sobel(floatImage, dy, CV_32F, 0, 1, parameters.gradientKernelSize);
  multiply(dx, dx, dx2);
  multiply(dx, dy, dxy);
  multiply(dy, dy, dy2);
  Mat kernel = Mat::ones(parameters.kernelSize, parameters.kernelSize, CV_32F);
  filter2D(dx2, dx2, -1, kernel);
  filter2D(dxy, dxy, -1, kernel);
  filter2D(dy2, dy2, -1, kernel);
}
