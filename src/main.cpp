#include <cstdlib>
#include <fstream>
#include <iostream>
#include <opencv2/core/utils/logger.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <poll.h>
#include <termios.h>
#include <unistd.h>

using namespace std;

static const char CHAR_ARRAY[] = {
    '@',    '#',    'W',    '$',    '9',    '8',    '7',    '6',    '5',
    '4',    '3',    '2',    '1',    '0',    '?',    '!',    'a',    'b',
    'c',    ';',    ':',    '+',    '=',    '-',    ',',    '.',    '_',
    '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20', '\x20'};

static const string reset = "\033[0m";

struct OptionConfig {
  string inputFile;
  bool noColor = false;
  bool saveToFile = false;
  int width = 70;
  bool isLive = false;
};

class ImgToAscii {
private:
  string imagePath;
  cv::Mat img;
  int img_width;
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
    grayify();
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
  ImgToAscii(string img_path, bool isColor = true, int width = 70)
      : imagePath(img_path), isColor(isColor), img_width(width) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    img = cv::imread(imagePath);

    if (img.empty()) {
      cerr << "\nInvalid input file format\n\n";
      exit(EXIT_FAILURE);
      ;
    }

    resize_image();
  }

  ImgToAscii(cv::Mat mat, bool isColor = true, int width = 70)
      : img(mat), isColor(isColor), img_width(width) {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);

    if (img.empty()) {
      cerr << "\nInvalid input file format\n\n";
      exit(EXIT_FAILURE);
      ;
    }

    resize_image();
  }

  void updateImg(cv::Mat mat) {
    img = mat;
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

  int getHeight() { return img.rows; }

  string getImageName() {
    filesystem::path p(imagePath);
    return p.stem();
  }

  void printAscii() {
    string ascii_text = isColor ? getColoredAsciiText() : getAsciiText();
    cout << ascii_text << endl;
  }

  string getRawAsciiText() {
    string ascii_text = isColor ? getColoredAsciiText() : getAsciiText();
    return ascii_text;
  }

  void grayify() {
    cv::Mat grayImg;
    cv::cvtColor(img, grayImg, cv::COLOR_BGR2GRAY);
    this->img = grayImg;
  }

  void write_to_file() {
    string outFilename = getImageName() + "_ascii.txt";
    string ascii_text = isColor ? getColoredAsciiText() : getAsciiText();

    ofstream outFile(outFilename, ios::binary);
    if (outFile.is_open()) {
      outFile << ascii_text;
      outFile.close();
      cout << "Output successfully written to " << outFilename << endl;
      return;
    } else {
      cerr << "Error while writing to file: " << outFilename << endl;
      exit(EXIT_FAILURE);
      ;
    }
  }
};

void printHelp() {
  cout << "iascii - Image to ASCII art converter\n\n";
  cout << "Usage: iascii [options] <filename>\n\n";
  cout << "Options:\n";
  cout << "  -nc, --no-color      Output in grayscale\n";
  cout << "  -s,  --save-file     Save to file instead of stdout\n";
  cout << "  -w,  --width <width> Specify custom row width to the output "
          "ascii\n";
  cout << "  -h,  --help          Show this help\n";
}

OptionConfig parseArgs(int argc, char *argv[]) {
  OptionConfig config;

  int index = 1;

  while (index < argc) {
    string arg = argv[index];
    if (arg == "-h" || arg == "--help") {
      printHelp();
      exit(EXIT_SUCCESS);
    } else if (arg == "-nc" || arg == "--no-color") {
      config.noColor = true;
    } else if (arg == "-s" || arg == "--save-file") {
      config.saveToFile = true;
    } else if (arg == "-w" || arg == "--width") {
      try {
        config.width = std::stoi(argv[++index]);
      } catch (...) {
        cerr << "Error: invalid width argument at index " << index << " ("
             << argv[index] << ")" << std::endl;
        exit(EXIT_FAILURE);
      }
    } else if (arg == "-l" || arg == "--live") {
      config.isLive = true;
      config.inputFile = "no file";
    } else if (arg[0] != '-') {
      config.inputFile = arg;
    } else {
      cerr << "Unknown option: " << arg << endl;
      exit(EXIT_FAILURE);
      ;
    }
    index++;
  }

  if (config.inputFile.empty()) {
    cerr << "No input file specified\n";
    exit(EXIT_FAILURE);
    ;
  }

  return config;
}

void setRawMode(bool enable) {
  static struct termios oldt, newt;

  if (enable) {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  } else {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  }
}

int main(int argc, char *argv[]) {
  OptionConfig cfg = parseArgs(argc, argv);

  if (cfg.isLive) {
    auto cap = cv::VideoCapture(0);

    cv::Mat frame;
    cap.read(frame);

    cout << "\033[2J\033[H";
    cout.flush();

    ImgToAscii itva(frame, !cfg.noColor, cfg.width);
    string ascii;

    int linesToClear = 0;
    setRawMode(true);

    struct pollfd pfd = {STDIN_FILENO, POLLIN, 0};

    while (1) {
      if (poll(&pfd, 1, 0) > 0) {
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0 && (c == 'q' || c == 'Q')) {
          break;
        }
      }
      cap.read(frame);
      if (frame.empty()) {
        cerr << "ERROR! blank frame grabbed\n";
        break;
      }

      itva.updateImg(frame);
      ascii = itva.getRawAsciiText();

      linesToClear = std::count(ascii.begin(), ascii.end(), '\n');
      cout << "\033[" << linesToClear << "A";
      cout << ascii;
      cout.flush();
    }

    setRawMode(false);
    cout << "\033[2J\033[H";
    cout.flush();
    return 0;
  }

  ImgToAscii ita(cfg.inputFile, !cfg.noColor, cfg.width);
  if (cfg.saveToFile) {
    ita.write_to_file();
  } else {
    ita.printAscii();
  }

  return 0;
}
