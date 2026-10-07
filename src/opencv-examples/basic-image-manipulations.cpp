#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <iostream>

using namespace cv;
using namespace std;

int main()
{
    cout << "Built with OpenCV " << CV_VERSION << endl;

    Mat imgOriginal;
    Mat imgCinza, imgBlur, imgCanny, imgBlurCanny;
    Mat imgCrop, imgReSize, imgScale;

    imgOriginal = imread("husky.jpg");
    resize(imgOriginal, imgScale, Size(), 0.6, 0.6);
    cvtColor(imgScale, imgCinza, COLOR_BGR2GRAY);
    GaussianBlur(imgCinza, imgBlur, Size(13, 13), 5, 0);
    Canny(imgCinza, imgCanny, 25, 75);
    Canny(imgBlur, imgBlurCanny, 25, 75);

    Mat grad_x, grad_y;
    Sobel(imgCinza, grad_x, CV_16S, 1, 0, 1, 1, 0, BORDER_DEFAULT);
    Sobel(imgCinza, grad_y, CV_16S, 0, 1, 1, 1, 0, BORDER_DEFAULT);

    Mat abs_grad_x, abs_grad_y, grad;
    convertScaleAbs(grad_x, abs_grad_x);
    convertScaleAbs(grad_y, abs_grad_y);
    addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, grad);

    Mat imgEqual;
    equalizeHist(imgCinza, imgEqual);

    Mat imgDilate, imgErode;
    Mat kernel = getStructuringElement(MORPH_RECT, Size(5, 5));
    dilate(imgBlurCanny, imgDilate, kernel);
    erode(imgDilate, imgErode, kernel);

    Rect areaCrop(100, 100, 200, 200);
    imgCrop = imgOriginal(areaCrop);

    imshow("Imagem Original", imgScale);
    imshow("Imagem Grayscale", imgCinza);
    imshow("Imagem Borrada", imgBlur);
    imshow("Imagem Borda Grayscale", imgCanny);
    imshow("Imagem Borda Borrada", imgBlurCanny);
    imshow("Sobel Result", grad);
    imshow("Imagem Equalizada", imgEqual);
    imshow("Imagem Dilatada", imgDilate);
    imshow("Imagem Erodida", imgErode);
    imshow("Imagem Cortada", imgCrop);

    waitKey(0);
    
    return 0;
}