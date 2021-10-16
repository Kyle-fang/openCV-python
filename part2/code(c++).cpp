#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/photo/photo.hpp>

using namespace std;
using namespace cv;
#define WINDOW_NEME0 "原始图"
#define WINDOW_NEME1 "中值滤波"
#define WINDOW_NEME2 "第一次修补后的图像" 
#define WINDOW_NEME3 "第二次修补后的图像"
Mat srcImage, meduimBlurImage, inpaintMask;
Point previousPoint(-1, -1);
int minSize = 3;  // 滤波器窗口的起始尺寸
int maxSize = 7;  // 滤波器窗口的最大尺寸
static void On_Mouse(int event, int x, int y, int flags, void*);
uchar adaptiveProcess(const Mat& im, int row, int col, int kernelSize, int maxSize);
Mat FliterProcss(const Mat ime, void*);

int main(int args, char** argv)
{
	srcImage = imread("E:\\jupyter_notebook_files\\ImageAndVideoProcess\\test2\\3_5.jpg", 0);
	//srcImage = imread("E:\\jupyter_notebook_files\\ImageAndVideoProcess\\test2\\3_10.jpg",0);
	if (!srcImage.data)
	{
		printf("读取图像错误！");
		return false;
	}
	imshow(WINDOW_NEME0, srcImage);

	srcImage = FliterProcss(srcImage, 0);
	imshow(WINDOW_NEME2, srcImage);
	srcImage = FliterProcss(srcImage, 0);
	srcImage = FliterProcss(srcImage, 0);
	imshow(WINDOW_NEME3, srcImage);
	//按任意键退出
	waitKey();
	return 0;
}
static void On_Mouse(int event, int x, int y, int flags, void*)
{
	if (event == CV_EVENT_LBUTTONUP || !(flags & CV_EVENT_FLAG_LBUTTON))
		previousPoint == Point(-1, -1);
	else if (event == CV_EVENT_LBUTTONDOWN)
		previousPoint == Point(x, y);
	else if (event == CV_EVENT_MOUSEMOVE && (flags & CV_EVENT_FLAG_LBUTTON))
	{
		Point pt(x, y);
		if (previousPoint.x < 0)
			previousPoint = pt;
		line(inpaintMask, previousPoint, pt, Scalar::all(255), 5);
		line(meduimBlurImage, previousPoint, pt, Scalar::all(255), 5);
		previousPoint = pt;
		imshow(WINDOW_NEME1, meduimBlurImage);
	}
}

Mat FliterProcss(const Mat ime, void*)
{
	Mat border_Image, stmImage;

	copyMakeBorder(srcImage, border_Image, maxSize / 2, maxSize / 2, maxSize / 2, maxSize / 2, BORDER_REFLECT);
	border_Image.copyTo(stmImage);
	for (int y = maxSize / 2; y < border_Image.rows - maxSize / 2; y++)
	{
		for (int x = maxSize / 2; x < border_Image.cols * border_Image.channels() - maxSize / 2; x++)
		{
			stmImage.at<uchar>(y, x) = adaptiveProcess(border_Image, y, x, minSize, maxSize);
		}
	}
	for (int y = 0; y < srcImage.rows; y++)
	{
		for (int x = 0; x < srcImage.cols * srcImage.channels(); x++)
		{
			srcImage.at<uchar>(y, x) = stmImage.at<uchar>(y + maxSize / 2, x + maxSize / 2);
		}
	}
	return srcImage;
}

uchar adaptiveProcess(const Mat& im, int row, int col, int kernelSize, int maxSize)
{
	int black = 0;
	int white = 0;
	vector <uchar> pixels; //将滑窗内的所有元素放入该容器内
	for (int i = -kernelSize / 2; i <= kernelSize / 2; i++)//滑窗进行
	{
		for (int j = -kernelSize / 2; j <= kernelSize / 2; j++)
		{
			pixels.push_back(im.at<uchar>(row + i, col + j));//
		}
	}
	sort(pixels.begin(), pixels.end()); //对数组内的元素进行排序
	//auto min = pixels[0]; //取最小值 放在数组的第一位
	//auto max = pixels[kernelSize * kernelSize - 1];//取最大值 放在数组最后一位
	auto medium = pixels[kernelSize * kernelSize / 2 + 1];//取中间值
	auto grayScale = im.at<uchar>(row, col);//实际点的灰度值
	for (int i = 0; i < kernelSize * kernelSize; i++)
	{
		if (pixels[i] > 127)
			white++;
		else
			black++;
	}
	if (black > white)//检测背景是黑色还是白色，当背景是黑色时候
	{
		if (white > kernelSize - 1) //如果某个区域内白色点比较多，就说明是线要留着
			return grayScale;
		else if (white == kernelSize - 1)//如果该白色点是噪点还是线无法确定，就要窗口方放大来确定
		{
			kernelSize = kernelSize + 2;//窗口放大
			if (kernelSize < maxSize)
			{
				return adaptiveProcess(im, row, col, kernelSize, maxSize);
			}// 增大窗口尺寸，继续A过程。
			else
			{
				return grayScale;
			}
		}
		else
			return medium; //如果某个区域内白色点较少，就是噪点要祛除
	}
	else
	{
		if (black > kernelSize - 1)
			return grayScale;
		else if (black == kernelSize - 1)
		{
			if (kernelSize < maxSize)
			{
				kernelSize = kernelSize + 2;
				return adaptiveProcess(im, row, col, kernelSize, maxSize); // 增大窗口尺寸，继续A过程。
			}
			else return grayScale;
		}
		else
			return medium;
	}
}
