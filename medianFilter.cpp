#include <opencv2/opencv.hpp>
#include <vector>
#include <omp.h>

using namespace std;
using namespace cv;
using namespace std::chrono;

// this function divides the picture into n parts, where n = number of cores the user selects.
vector<Mat> divideImage(const Mat& image, int n) {
    vector<Mat> imageParts;

    int partWidth = image.cols / n;
    int partHeight = image.rows / n;

    // stores images parts into a vector.
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            // defines the section part of image to be stored.
            Rect roi(j * partWidth, i * partHeight, partWidth, partHeight);
            
            // crop from the main image.
            Mat part = image(roi).clone();

            // store in the array.
            imageParts.push_back(part);
        }
    }
    return imageParts;
}

// merge divided image parts into one image.
Mat combineImageParts(const vector<Mat>& imageParts, int n) {
    // calculating the dimensions image to be combined.
    int combinedWidth = imageParts[0].cols * n;
    int combinedHeight = imageParts[0].rows * n;

    // Create an empty image to store combined result
    Mat combinedImage(combinedHeight, combinedWidth, imageParts[0].type());

    int index = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            
            Rect roi(j * imageParts[index].cols, i * imageParts[index].rows, imageParts[index].cols, imageParts[index].rows);
            // merge the part into the empty image
            imageParts[index].copyTo(combinedImage(roi));
            ++index;
        }
    }
    return combinedImage;
}

int main() {
    Mat inputImage = imread("binary.png"); 
    if (inputImage.empty()) {
        cerr << "Error: Could not read the image." << endl;
        return -1;
    }
    // resize(inputImage,inputImage, Size(512,512));
    int cores = 1;
    cout << "Please enter the number of cores to use: ";
    cin >> cores;

    // divide the image into parts.
    vector<Mat> imageParts = divideImage(inputImage, cores);
    int partWidth = inputImage.cols / cores;
    auto start = high_resolution_clock::now(); // note timestap for starting time.
    
    // performing 
    omp_set_num_threads(cores);
    #pragma omp parallel 
    {
        #pragma omp for schedule(dynamic)
        for (int i = 0; i < imageParts.size(); ++i)
        {
            medianBlur(imageParts[i], imageParts[i], 3);
        }
    }
    
    auto stop = high_resolution_clock::now(); // // note timestap for ending time.
    auto duration = duration_cast<microseconds>(stop - start).count(); // calculate the total execution time.

    // combining all the parts into one image.
    Mat combinedImage = combineImageParts(imageParts, cores);
    cout << "\nMedian Filtering Completed Successfully, image saved.";
    imwrite("medianFiltered.png", combinedImage);
    cout << "\nDuration: " << duration << " microseconds\n";

    return 0;
}