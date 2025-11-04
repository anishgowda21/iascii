#include <fstream>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

using namespace std;

static const char CHAR_ARRAY[] = {
    '@',    '#',    'W',    '$',    '9',    '8',    '7',    '6',    '5',
    '4',    '3',    '2',    '1',    '0',    '?',    '!',    'a',    'b',
    'c',    ';',    ':',    '+',    '=',    '-',    ',',    '.',    '_',
    '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20'};

static const string reset = "\033[0m";

enum OptionAction {
  CONTINUE,
  ERROR,
  RETURN,
};

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
  }

  string rgb_fg(int r, int g, int b, char c) {
    return "\033[38;2;" + to_string(r) + ";" + to_string(g) + ";" +
           to_string(b) + "m" + c;
  }

  string getAsciiText() {
    string ascii_text;
    ascii_text.reserve(
        img.rows * img.cols *
        20); // Reserve sufficient space for all ascii texts beforehand
    for (int i = 0; i < img.rows; i++) {
      for (int j = 0; j < img.cols; j++) {
        int intensity = img.at<uchar>(i, j);
        int char_index = (size(CHAR_ARRAY) - 1) -
                         int((intensity / 255.0) * (size(CHAR_ARRAY) - 1));
        ascii_text += CHAR_ARRAY[char_index];
      }
      ascii_text += '\n';
    }
    return ascii_text;
  }

  string getColoredAsciiText() {
    string ascii_text;
    ascii_text.reserve(
        img.rows * img.cols *
        20); // Reserve sufficient space for all ascii texts beforehand
    for (int i = 0; i < img.rows; i++) {
      for (int j = 0; j < img.cols; j++) {
        cv::Vec3b pixel = img.at<cv::Vec3b>(i, j);
        int intensity =
            0.114 * pixel[0] + 0.587 * pixel[1] +
            0.299 * pixel[2]; // Use weighted calulation to calulate intensity
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

  string getImageName() {
    filesystem::path p(imagePath);
    return p.stem();
  }

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

  int write_to_file(string optionStr) {
    grayify();
    string outFilename = getImageName() + "_ascii.txt";
    string ascii_text = getAsciiText();

    ofstream outFile(outFilename, ios::binary);
    if (outFile.is_open()) {
      outFile << ascii_text;
      outFile.close();
      return 0;
    } else {
      cerr << "Error while writing to file: " << outFilename << endl;
      return 1;
    }
  }

  OptionAction handleOptions(string arg) {
    if (arg == "-nc" || arg == "--no-color") {
      grayify();
    } else if (!arg.rfind("--save-file") || !arg.rfind("-S")) {
      return write_to_file(arg) ? OptionAction::ERROR : OptionAction::RETURN;
    } else {
      cerr << "No args match " << arg << endl;
      return OptionAction::ERROR;
    }
    return OptionAction::CONTINUE;
  }
};

int main(int argv, char *argc[]) {
  if (argv < 2) {
    cerr << "Usage: iascii [--options] <filname>";
    return 1;
  }

  vector<string> args(argc, argc + argv);

  string filename = args.back();

  ImgToAscii ita(filename);

  for (int i = 1; i < args.size() - 1; i++) {
    string arg = args.at(i);
    int ret = ita.handleOptions(arg);
    switch (ret) {
    case OptionAction::CONTINUE:
      break;
    case OptionAction::ERROR: {
      return 1;
    }
    case OptionAction::RETURN: {
      return 0;
    }
    }
  }

  ita.printAscii();

  return 0;
}
