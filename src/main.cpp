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
#include <vector>

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
  ImgToAscii(string img_path, bool isColor = true)
      : imagePath(img_path), isColor(isColor) {
    img = cv::imread(imagePath);

    if (img.empty()) {
      cerr << "Cannot read the input file\n";
      exit(1);
    }

    resize_image();
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

  void grayify() {
    this->isColor = false;
    cv::Mat grayImg;
    cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);
    this->img = grayImg;
  }

  int handleOptions(string arg) {
    if (arg == "-nc") {
      grayify();
    } else {
      return 1;
    }
    return 0;
  }
};

int randInt(int lo, int hi) {
  static std::mt19937 gen{std::random_device{}()};
  std::uniform_int_distribution<int> dist(lo, hi);
  return dist(gen);
}

int main(int argv, char *argc[]) {
  if (argv < 2) {
    cerr << "Usage: iascii [--options] <filname>";
    return 1;
  }

  vector<string> args(argc, argc + argv);

  string filename = args.back();

  ImgToAscii ita(filename);

  for (int i = 1; i < args.size() - 1; i++) {
    int ret = ita.handleOptions(args.at(i));
    if (ret == 1) {
      cerr << "No args matching " << args.at(i) << endl;
      exit(1);
    }
  }

  ita.printAscii();

  return 0;
}
