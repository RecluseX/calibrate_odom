#include <ros/ros.h>
#include <geometry_msgs/Twist.h>
#include <nav_msgs/Odometry.h>
#include <dynamic_reconfigure/server.h>
#include <calibrate_odom/calibrateConfig.h>

double odom_test_start_dis = 0;
double odom_test_distance;
double odom_scale;
double actual_distance;
bool reach_test_distance = false;
bool bStartTest = false;
double odom_x;
double temp_odom_x;
double limit;

void odom_callback(const nav_msgs::OdometryConstPtr& odom_t)
{
    nav_msgs::Odometry msg = *odom_t;
    temp_odom_x = msg.pose.pose.position.x;
    odom_x = msg.pose.pose.position.x - odom_test_start_dis;
    ROS_INFO("odom_x:%f", odom_x);
}


void cfg_callback(calibrate_odom::calibrateConfig &config, uint32_t level)
{
    odom_test_distance = config.test_distance;
    actual_distance = config.actual_distance;
    odom_scale = actual_distance / odom_test_distance;
    bStartTest = config.start_test;
    odom_test_start_dis = temp_odom_x;
    ROS_INFO("test_distance:%f  actual_distance:%f  odom_scale:%f  start_test:%d", odom_test_distance, actual_distance, odom_scale, bStartTest);
}

void calibrate_odom_test()
{
    if (fabs(odom_test_distance - odom_x) < limit)
    {
        reach_test_distance = true;
    }
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "calibrate_odom_node");
    ros::NodeHandle nn;
    ros::NodeHandle nh_("~");
    ros::Publisher cmd_pub;
    ros::Subscriber odom_sub;

    cmd_pub = nn.advertise<geometry_msgs::Twist>("/cmd_vel", 10);
    odom_sub = nn.subscribe("/odom", 1, &odom_callback);
    dynamic_reconfigure::Server<calibrate_odom::calibrateConfig> server;
    dynamic_reconfigure::Server<calibrate_odom::calibrateConfig>::CallbackType f;
    f = boost::bind(&cfg_callback, _1, _2);
    server.setCallback(f);
/*
    nh_.param("/calibrate_odom_node/odom_test_distance", odom_test_distance, 2.0);
    ROS_INFO("odom_test_distance:%f", odom_test_distance);
    nh_.param("/calibrate_odom_node/odom_scale", odom_scale, 1.0);
    ROS_INFO("odom_scale:%f", odom_scale);
    nh_.param("/calibrate_odom_node/limit", limit, 0.02);
    ROS_INFO("limit:%f", limit);
*/
    ros::Rate loop_rate(50);

    while(ros::ok())
    {
        geometry_msgs::Twist vel_cmd;
        vel_cmd.linear.y = 0;
        vel_cmd.linear.z = 0;
        vel_cmd.angular.x = 0;
        vel_cmd.angular.y = 0;
        vel_cmd.angular.z = 0;
        if (bStartTest)
        {
            vel_cmd.linear.x = 0.1;
            calibrate_odom_test();
            if (reach_test_distance)
            {
                bool bStartTest = false;
            }
        }
        else
        {
            vel_cmd.linear.x = 0.0;
            ROS_INFO("test_distance:%f", odom_x);
        }
        cmd_pub.publish(vel_cmd);
        ros::spinOnce();
        loop_rate.sleep();
    }


}
