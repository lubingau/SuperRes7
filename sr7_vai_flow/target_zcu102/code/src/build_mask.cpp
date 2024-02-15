#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>

using namespace cv;
using namespace std;

void fill_with_1(Mat& grid, int i, int j, int endi, int endj) {
    Mat temp(endi - i, endj - j, CV_8U, Scalar(0));
    for (int x = i, it = 0; x < endi && it < endi - i; ++x, ++it) {
        for (int y = j, jt = 0; y < endj && jt < endj - j; ++y, ++jt) {
            temp.at<uchar>(it, jt) = grid.at<uchar>(x, y); 
        }
    }
    for (int x = i, it = 0; x < endi && it < endi - i; ++x, ++it) {
        for (int y = j, jt = 0; y < endj && jt < endj - j; ++y, ++jt) {
            grid.at<uchar>(x, y) = temp.at<uchar>(it, jt) + 1; 
        }
    }
}


void build_grid0(Mat& grid, int patch_size, int stride) {
    // Build the grid for regular patches starting from the top left corner of the image
    int height = grid.rows; // hauteur de la matrice
    int width = grid.cols; // largeur de la matrice

    for (int s_i = 0; s_i < height; s_i += stride) {
        for (int s_j = 0; s_j < width; s_j += stride) {
            if (s_i + patch_size < height && s_j + patch_size < width) {
                if (s_i % (2 * stride) == 0 && s_j % (2 * stride) == 0) {
                    fill_with_1(grid, s_i, s_j, s_i + patch_size, s_j + patch_size);
                    if (s_i + stride + patch_size < height && s_j + stride + patch_size < width) {
                        fill_with_1(grid, s_i + stride, s_j + stride, s_i + stride + patch_size, s_j + stride + patch_size);
                    }
                }
            }
        }
    }
}

void build_grid1(Mat& grid, int patch_size, int stride) {
    int height = grid.rows;
    int width = grid.cols;

    for (int s_i = 0; s_i < height; s_i += stride) {
        for (int s_j = 0; s_j < width; s_j += stride) {
            if (s_i + patch_size < height && s_j + patch_size < width) {
                if (s_i % (2 * stride) == 0 && s_j % (2 * stride) == 0) {
                    if (s_i == 0 && s_j + 2 * stride < width) {
                        fill_with_1(grid, s_i, s_j + stride, s_i + patch_size, s_j + patch_size + stride);
                    }
                    if (s_i + patch_size + stride < height && s_j + patch_size < width) {
                        fill_with_1(grid, s_i + stride, s_j, s_i + stride + patch_size, s_j + patch_size);
                    }
                    if (s_i + 2 * stride + patch_size < height && s_j + 2 * stride < width) {
                        fill_with_1(grid, s_i + 2 * stride, s_j + stride, s_i + 2 * stride + patch_size, s_j + stride + patch_size);
                    }}}}}}

void build_grid2(Mat& grid, int patch_size, int stride) {
    int height = grid.rows;
    int width = grid.cols;

    for (int s_i = 0; s_i < height; s_i += stride) {
        if (s_i + patch_size < height) {
            fill_with_1(grid, s_i, width - patch_size, s_i + patch_size, width);
        }
    }
    for (int s_j = 0; s_j < width; s_j += stride) {
        if (s_j + patch_size < width) {
            fill_with_1(grid, height - patch_size, s_j, height, s_j + patch_size);
        }
    }
    fill_with_1(grid, height - patch_size, width - patch_size, height, width);
}


int main(int argc, char** argv) {
    if (argc != 3) {
        cout << "Usage: ./build_matrix <image_path> <output_path>\n";
        return -1;
    }

    // Input image path
    string path_sensor_image = argv[1];
    string path_output = argv[2];
    cout << "[SR7 INFO] Loading the image from " << path_sensor_image << endl;
    Mat image = imread(path_sensor_image);
    if (image.rows % 2 != 0) {
        cout << "[SR7 WARNING] The image height is not even. Last pixels deleted" << endl;
        image = image(Rect(0, 0, image.cols, image.rows - 1));
    }
    if (image.cols % 2 != 0) {
        cout << "[SR7 WARNING] The image width is not even. Last pixels deleted" << endl;
        image = image(Rect(0, 0, image.cols - 1, image.rows));
    }
    cout << "[SR7 INFO] Image loaded" << endl;
    cout << "[SR7 INFO] Image shape: " << image.size() << endl;
    int WIDTH = 2*image.cols;
    int HEIGHT = 2*image.rows;

    // Patches parameters
    int patch_size = 256;
    double stride_ratio = 0.1;
    int stride = static_cast<int>((1 - stride_ratio) * patch_size);
    int n_patches_i = image.rows / stride;
    int n_patches_j = image.cols / stride;
    int n_patches_edge = n_patches_i + n_patches_j + 1;
    int n_patches = n_patches_i * n_patches_j + n_patches_edge;

    Mat grid0(image.rows, image.cols, CV_8U, Scalar(0));
    Mat grid1(image.rows, image.cols, CV_8U, Scalar(0));
    Mat grid2(image.rows, image.cols, CV_8U, Scalar(0));

    build_grid0(grid0, patch_size, stride);
    build_grid1(grid1, patch_size, stride);
    build_grid2(grid2, patch_size, stride);

    Mat grid = grid0 + grid1 + grid2;

    //grid = 1 / grid * 255; //For display

    grid.convertTo(grid, CV_8U);
    imwrite(path_output + format("mask_%d_%d.png", HEIGHT, WIDTH), grid);

    cout << "[SR7 INFO] Grid shape: " << image.size() << endl;
    cout << "[SR7 INFO] Patch size: " << patch_size << "x" << patch_size << endl;
    cout << "[SR7 INFO] Stride ratio: " << static_cast<int>(stride_ratio * 100) << "%" << endl;
    cout << "[SR7 INFO] Stride: " << stride << endl;
    cout << "[SR7 INFO] Number of patches: " << n_patches << endl;

    // cout << "[SR7 WARNING] Press 'q' to quit (or ctrl+C in the terminal)" << endl;
    // while (true) {
    //     char key = waitKey(1);
    //     if (key == 'q' || key == 'Q')
    //         break;
    // }
    // destroyAllWindows();

    return 0;
}