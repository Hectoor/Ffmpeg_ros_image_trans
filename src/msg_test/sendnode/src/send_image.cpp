#include <sstream>
#include "ros/ros.h"
#include "std_msgs/String.h"
#include "transdata/transdata.h"
#include <cv_bridge/cv_bridge.h>
#include <image_transport/image_transport.h>


Transdata transdata;
std::mutex mImage_buf;
int main(int argc, char ** argv)
{
    ros::init(argc,argv,"talker");

    ros::NodeHandle n;

    image_transport::ImageTransport it(n);
    image_transport::Publisher pub = it.advertise("camera/image", 1);
    ros::Rate loop_rate(10);

    int count = 0;
    sensor_msgs::ImagePtr msg;

    if(transdata.Transdata_init() < 0)
    {
        cout <<"init error !" << endl;
        return -1;
    }

    while(ros::ok())
    {
        //接收图像并显示
        transdata.Transdata_Recdata();

        //需要使用互斥锁 因为要操作图像数据
        mImage_buf.lock();
        if(!transdata.image_test.empty())
        {
//            imshow("test",transdata.image_test);
//            waitKey(10);
            msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", transdata.image_test).toImageMsg();
            pub.publish(msg);
            cout << " send image " << count << endl;
            count ++;
            transdata.image_test.release();
        }
        mImage_buf.unlock();

        ros::spinOnce();
    }
    return 0;
}



