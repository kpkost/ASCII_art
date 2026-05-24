#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cstdlib>
#include <thread>
#include <chrono>
#include <windows.h>
#include <conio.h>


using namespace std;


// Масштабирование изображения с фактором (int)
cv::Mat img_scale(const cv::Mat& img, int scale_factor) {
    if (scale_factor <= 0) return img.clone();
    int xscale = scale_factor;
    int yscale = scale_factor;
    int new_height = img.rows / yscale;
    int new_width = img.cols / xscale;

    if (new_height <= 0 || new_width <= 0) {
        return cv::Mat();
    }
    cv::Mat res_img(new_height, new_width, CV_8UC1, cv::Scalar(0));

    for (int y = 0; y < new_height; ++y) {
        for (int x = 0; x < new_width; ++x) {
            double sum = 0;
            for (int dy = 0; dy < yscale; ++dy) {
                for (int dx = 0; dx < xscale; ++dx) {
                    sum += img.at<uchar>(y * yscale + dy, x * xscale + dx);
                }
            }
            res_img.at<uchar>(y, x) = static_cast<uchar>(sum / (xscale * yscale));
        }
    }
    return res_img;
}


// Масштабирование до целевого размера (width, height)
cv::Mat img_scale(const cv::Mat& img, pair<int, int> target_size) {
    int target_width = target_size.first;
    int target_height = target_size.second;

    if (target_width <= 0 || target_height <= 0 ||
        target_width > img.cols || target_height > img.rows) {
        return img.clone();
    }

    int xscale = img.cols / target_width;
    int yscale = img.rows / target_height;

    cv::Mat res_img(target_height, target_width, CV_8UC1, cv::Scalar(0));

    for (int y = 0; y < target_height; ++y) {
        for (int x = 0; x < target_width; ++x) {
            double sum = 0;
            for (int dy = 0; dy < yscale; ++dy) {
                for (int dx = 0; dx < xscale; ++dx) {
                    sum += img.at<uchar>(y * yscale + dy, x * xscale + dx);
                }
            }
            res_img.at<uchar>(y, x) = static_cast<uchar>(sum / (xscale * yscale));
        }
    }
    return res_img;
}


// Конвертация изображения в оттенки серого
cv::Mat grayscale(const string& image_path) {
    cv::Mat image = cv::imread(image_path, cv::IMREAD_COLOR);
    if (image.empty()) {
        cerr << "Error: Could not load image " << image_path << endl;
        return cv::Mat();
    }

    cv::Mat image_array(image.rows, image.cols, CV_8UC1);

    for (int i = 0; i < image.rows; ++i) {
        for (int j = 0; j < image.cols; ++j) {
            cv::Vec3b pixel = image.at<cv::Vec3b>(i, j);
            uchar gray = static_cast<uchar>(
                pixel[2] * 0.299 +  // R
                pixel[1] * 0.587 +  // G
                pixel[0] * 0.114    // B
                );
            image_array.at<uchar>(i, j) = gray;
        }
    }
    return image_array;
}


// Конвертация в ASCII-арт
vector<string> ascii_convert(const string& image_path, int scale = 0) {
    cv::Mat image_array = grayscale(image_path);
    if (image_array.empty()) {
        return {};
    }

    if (scale > 0) {
        image_array = img_scale(image_array, scale);
        if (image_array.empty()) {
            return {};
        }
    }

    const string ascii_symbols = " `.-':_,=><+!rc*/z?sLTv)J7(|Fi{C}fI31tluneoZ5Yxjya2ESwqk6h9d4pOGbUAKXHm8RD#Bg0MNWQ%&@@";

    vector<string> ascii_image(image_array.rows);
    for (int i = 0; i < image_array.rows; ++i) {
        ascii_image[i].reserve(image_array.cols);
        for (int j = 0; j < image_array.cols; ++j) {
            int idx = image_array.at<uchar>(i, j) / 3;
            idx = min(idx, static_cast<int>(ascii_symbols.length()) - 1);
            idx = max(idx, 0);
            ascii_image[i] += ascii_symbols[idx];
        }
    }
    return ascii_image;
}


//Очистка экрана
inline void clearScreen() {
    std::cout << "\033[2J\033[H" << std::flush;
}
 

//Анимация
void ascii_animation(int frames_count, const std::string& frames_path,
    const std::string& file_name = "frame", int scale = 0, int frame_rate = 30) {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hOut, &cursorInfo);
    bool cursorWasVisible = cursorInfo.bVisible;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hOut, &cursorInfo);
    std::vector<std::string> frames;
    frames.reserve(frames_count);
    size_t max_w = 0, max_h = 0;
    for (int i = 0; i < frames_count; ++i) {
        std::ostringstream oss;
        oss << frames_path << "\\" << file_name << std::setfill('0') << std::setw(4) << (i + 1) << ".jpg";
        auto ascii = ascii_convert(oss.str(), scale);
        std::vector<std::string> lines;
        lines.reserve(ascii.size());
        for (const auto& row : ascii) {
            std::string spaced;
            spaced.reserve(row.size() * 3);
            for (size_t c = 0; c < row.size(); ++c) {
                spaced += row[c];
                if (c < row.size() - 1) spaced += "  ";
            }
            lines.push_back(spaced);
            if (spaced.size() > max_w) max_w = spaced.size();
        }
        if (lines.size() > max_h) max_h = lines.size();
        std::string frame_str;
        frame_str.reserve(max_w * max_h * 3);
        for (const auto& line : lines) {
            frame_str += line + std::string(max_w - line.size(), ' ') + "\n";
        }
        for (size_t l = lines.size(); l < max_h; ++l) {
            frame_str += std::string(max_w, ' ') + "\n";
        }
        frames.push_back(frame_str);
    }
    COORD startPos = { 0, 0 };
    DWORD charsWritten = 0;
    const auto target_dt = std::chrono::microseconds(1000000 / frame_rate); 
    for (const auto& frame : frames) {
        auto frame_start = std::chrono::steady_clock::now();
        SetConsoleCursorPosition(hOut, startPos);
        WriteConsoleA(hOut, frame.c_str(), static_cast<DWORD>(frame.length()), &charsWritten, nullptr);
        auto frame_end = std::chrono::steady_clock::now();
        auto elapsed = frame_end - frame_start;
        if (elapsed < target_dt) {
            std::this_thread::sleep_for(target_dt - elapsed);
        }
    }
}

int main() {
    std::cin.tie(nullptr);
    int frame_count, scale, frame_rate;
    string path, file_name;
    cout << "";
    cout << "Input frame count (max 9999):" << endl << endl;
    cin >> frame_count;
    cout << endl;
    cout << "Input frame rate (by default 30):" << endl << endl;
    cin >> frame_rate;
    cout << endl;
    cout << "Input path to animation frames:" << endl << endl;
    cin >> path;
    cout << endl;
    cout << "Input compression ratio (by default no compression, some issues are possible if image is bigger than console window size):" << endl << endl;
    cin >> scale;
    cout << endl;
    cout << "Input file name (frames should be named 'file_name'_number.jpg, example: frame_0001):" << endl << endl;
    cin >> file_name;
    cout << endl;
    cout << "Press any key to continue...";
    _getch();
    cout << endl << endl;
    cout << "Wait...";
    ascii_animation(frame_count, path, file_name + "_", scale, frame_rate);
    clearScreen();
    cout << "Press any key to exit";
    _getch();
    return 0;
}
