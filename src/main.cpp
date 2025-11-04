#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <random>
#include <string>

using namespace std;

static const char CHAR_ARRAY[] = {
    '@',    '#',    'W',    '$',    '9',    '8',    '7',    '6',    '5',
    '4',    '3',    '2',    '1',    '0',    '?',    '!',    'a',    'b',
    'c',    ';',    ':',    '+',    '=',    '-',    ',',    '.',    '_',
    '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20'};

static const string reset = "\033[0m";

class ImgToAscii {
private:
  string imagePath;
  cv::Mat img;
  int img_width = 100;
  bool isColor;

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

  string rgb_fg(int r, int g, int b, char c) {
    return "\033[38;2;" + to_string(r) + ";" + to_string(g) + ";" +
           to_string(b) + "m" + c;
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
      }
      ascii_text += '\n';
    }
    return ascii_text;
  }

  string getColoredAsciiText() {
    int counter = 0;
    string ascii_text = "";
    for (int i = 0; i < img.rows; i++) {
      for (int j = 0; j < img.cols; j++) {
        cv::Vec3b pixel = img.at<cv::Vec3b>(i, j);
        int intensity = (pixel[0] + pixel[1] + pixel[2]) / 3;
        int char_index = (size(CHAR_ARRAY) - 1) -
                         int((intensity / 255.0) * (size(CHAR_ARRAY) - 1));
        ascii_text +=
            rgb_fg(pixel[2], pixel[1], pixel[0], CHAR_ARRAY[char_index]);
      }
      ascii_text += '\n';
    }
    return ascii_text + reset + "\n";
  }

public:
  ImgToAscii(string img_path, bool isColor = false)
      : imagePath(img_path), isColor(isColor) {
    img = cv::imread(imagePath);

    if (img.empty()) {
      cerr << "Cannot read the input file\n";
      exit(1);
    }

    resize_image();
    if (!isColor)
      grayify();
  }

  void disPlayImg() {
    cv::imshow("Image", img);
    cv::waitKey(0);
  }

  void getPixelAt(int x, int y) {
    auto p = img.at<cv::Vec3b>(x, y);
    cout << "The p was " << p << endl;
  }

  string getImagePath() { return imagePath; }

  void printAscii() {
    string ascii_text = isColor ? getColoredAsciiText() : getAsciiText();
    cout << ascii_text << endl;
  }
};

int randInt(int lo, int hi) {
  static std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<int> dist(lo, hi);
  return dist(gen);
}

int main(int argv, char *argc[]) {
  if (argv != 2) {
    cerr << "Need Filename\n";
    return 1;
  }
  //
  // string arg1 = argc[1];
  // string output = "";
  //
  // for (int i = 0; i < size(arg1); i++) {
  //   int r = randInt(0, 255);
  //   int g = randInt(0, 255);
  //   int b = randInt(0, 255);
  //   output += rgb_fg(r, g, b, arg1.at(i));
  // }
  //
  // output += reset;
  //
  // cout << output << endl;

  string filename = argc[1];

  ImgToAscii ita(filename, true);
  ita.printAscii();

  return 0;
}
