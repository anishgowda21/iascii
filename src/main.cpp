#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>

using namespace std;

static const char CHAR_ARRAY[] = {
    '@',    '#',    'W',    '$',    '9',    '8',    '7',    '6',    '5',
    '4',    '3',    '2',    '1',    '0',    '?',    '!',    'a',    'b',
    'c',    ';',    ':',    '+',    '=',    '-',    ',',    '.',    '_',
    '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20'};

class ImgToAscii {
private:
  string imagePath;
  cv::Mat img;
  int img_width = 100;

  void resize_image() {

    double ratio = (img.rows / double(img.cols)) /
                   1.65; // Since monospapce chars have more height then width
                         // we try to suppress the height by reduing the ratio.

    int n_height = int(ratio * img_width);

    cv::Mat resized_image;

    cv::resize(img, resized_image, cv::Size_(img_width, n_height));

    this->img = resized_image;

    printf("Resized image is %dX%d\n", img.cols, img.rows);
  }

  void grayify() {
    cv::Mat grayImg;
    cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);
    this->img = grayImg;
  }

  string getAsciiText() {
    int counter = 0;
    string ascii_text = "";
    for (int i = 0; i < img.rows; i++) {
      for (int j = 0; j < img.cols; j++) {
        int intensity = img.at<uchar>(i, j);
        int char_index = (size(CHAR_ARRAY) - 1) -
                         int((intensity / 255.0) * (size(CHAR_ARRAY) - 1));
        ascii_text += CHAR_ARRAY[char_index];
        counter += 1;
        if (counter == 100) {
          ascii_text += '\n';
          counter = 0;
        }
      }
    }
    return ascii_text;
  }

public:
  ImgToAscii(string img_path) : imagePath(img_path) {
    img = cv::imread(imagePath);

    if (img.empty()) {
      cerr << "Cannot read the input file\n";
      exit(1);
    }

    resize_image();
    grayify();
  }

  void disPlayImg() {
    cv::imshow("Image", img);
    cv::waitKey(0);
  }

  void getPixelAt(int x, int y) {
    auto p = img.at<uchar>(x, y);
    printf("The p was %d\n", (int)p);
  }

  string getImagePath() { return imagePath; }

  void printAscii() {
    string ascii_text = getAsciiText();
    cout << ascii_text << endl;
  }
};

int main(int argv, char *argc[]) {
  if (argv != 2) {
    cerr << "Need Filename\n";
    return 1;
  }

  string filename = argc[1];

  ImgToAscii ita(filename);
  ita.printAscii();

  return 0;
}
