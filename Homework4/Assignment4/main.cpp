#include <chrono>
#include <iostream>
#include <opencv2/opencv.hpp>

std::vector<cv::Point2f> control_points;

void mouse_handler(int event, int x, int y, int flags, void *userdata) 
{
    if (event == cv::EVENT_LBUTTONDOWN && control_points.size() < 4) 
    {
        std::cout << "Left button of the mouse is clicked - position (" << x << ", "
        << y << ")" << '\n';
        control_points.emplace_back(x, y);
    }     
}

void naive_bezier(const std::vector<cv::Point2f> &points, cv::Mat &window) 
{
    auto &p_0 = points[0];
    auto &p_1 = points[1];
    auto &p_2 = points[2];
    auto &p_3 = points[3];

    for (double t = 0.0; t <= 1.0; t += 0.001) 
    {
        auto point = std::pow(1 - t, 3) * p_0 + 3 * t * std::pow(1 - t, 2) * p_1 +
                 3 * std::pow(t, 2) * (1 - t) * p_2 + std::pow(t, 3) * p_3;

        window.at<cv::Vec3b>(point.y, point.x)[2] = 255;
    }
}

cv::Point2f recursive_bezier(const std::vector<cv::Point2f> &control_points, float t) 
{
    // TODO: Implement de Casteljau's algorithm
    std::vector<cv::Point2f> temp_points = control_points;

    for (size_t i = 1; i < temp_points.size(); i++)
    {
        for (size_t j = 0; j < temp_points.size() - i; j++)
        {
            temp_points[j].x = (1 - t) * temp_points[j].x + t * temp_points[j + 1].x;
            temp_points[j].y = (1 - t) * temp_points[j].y + t * temp_points[j + 1].y;
        }
    }

    return temp_points[0];

}

void bezier(const std::vector<cv::Point2f> &control_points, cv::Mat &window) 
{
    // TODO: Iterate through all t = 0 to t = 1 with small steps, and call de Casteljau's 
    // recursive Bezier algorithm.
	//第一种利用双线性插值思想，对于邻居四个点，面积作为权值进行抗锯齿处理
    //第二种利用距离权重进行抗锯齿处理，可扩展性强
    for (double t = 0.0; t <= 1.0; t += 0.001)
    {
        auto point = recursive_bezier(control_points, t);
        //判断邻居四个点的位置，找到左上角的点
        int x_int = static_cast<int>(floor(point.x));
        int y_int = static_cast<int>(floor(point.y));
        float x_frac = point.x - x_int;
        float y_frac = point.y - y_int;

        int x0 = x_frac < 0.5f ? x_int - 1 : x_int;
        int y0 = y_frac < 0.5f ? y_int - 1 : y_int;

        for (int i = 0; i <= 1; i++) {
            for (int j = 0; j <= 1; j++) {
                float dx = abs(point.x - x0 - i - 0.5f);
                float dy = abs(point.y - y0 - j - 0.5f);
                float distance_square = dx * dx + dy * dy;

                float weight = 1 - distance_square / 2.0f;

                float color = window.at<cv::Vec3b>(y0 + j, x0 + i)[1];//获取上一次的颜色
                window.at<cv::Vec3b>(y0 + j, x0 + i)[1] = std::max(255.0f * weight, color);//更新为最大值，防止被覆盖
            }
        }
    }
}

int main() 
{
    cv::Mat window = cv::Mat(700, 700, CV_8UC3, cv::Scalar(0));
    cv::cvtColor(window, window, cv::COLOR_BGR2RGB);
    cv::namedWindow("Bezier Curve", cv::WINDOW_AUTOSIZE);

    cv::setMouseCallback("Bezier Curve", mouse_handler, nullptr);

    int key = -1;
    while (key != 27) 
    {
        for (auto &point : control_points) 
        {
            cv::circle(window, point, 3, {255, 255, 255}, 3);
        }

        if (control_points.size() == 4) 
        {
            //naive_bezier(control_points, window);
            bezier(control_points, window);

            cv::imshow("Bezier Curve", window);
            cv::imwrite("my_bezier_curve.png", window);
            key = cv::waitKey(0);

            return 0;
        }

        cv::imshow("Bezier Curve", window);
        key = cv::waitKey(20);
    }

return 0;
}
