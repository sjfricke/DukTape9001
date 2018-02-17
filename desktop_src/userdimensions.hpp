#include <iostream>
#include <vector>

#include <openpose/core/headers.hpp>

float dist(cv::Point2f a, cv::Point2f b);
std::vector<float> to_nice_float_array(op::Array<float>);

class ConfidencePoint3f {
    public:
    ConfidencePoint3f() { }
    cv::Point3f pos;
    float conf;
    ConfidencePoint3f(float x, float y, float z, float conf);
    
};

class ConfidencePoint2f {
    public:
    ConfidencePoint2f() { }
    cv::Point2f pos;
    float conf;
    ConfidencePoint2f(float x, float y, float conf);
    //Overall confidence of two points
    float pairConf(ConfidencePoint2f other);
    //Distance between two points weighted by confidence
    float confDist(ConfidencePoint2f other);
};

class UserTracked {
    public:
    UserTracked() { }
    ConfidencePoint2f nose, chest, rshoulder, relbow, rwrist, 
                    lshoulder, lelbow, lwrist, rhip, rknee,
                    rankle, lhip, lknee, lankle, reye, leye,
                    rear, lear, background;
    UserTracked(std::vector<float> keypoints);
};

//Metrics about the user, where all dimensions are in pixels on the calibration
//frame, where the user is placed at 6ft from the camera in T-position
class UserMetrics {
public:
    UserMetrics() { }
    float chest_to_nose = 0.0f; //"Chest" here actually means something more like "bottom of neck"
    float nose_to_ear = 0.0f;
    float chest_to_shoulder = 0.0f;
    float elbow_to_wrist = 0.0f;
    float shoulder_to_elbow = 0.0f;
    float hip_to_knee = 0.0f; //This measurement is based on the distance from one of left/right hip to the knee (averaged)
    float knee_to_ankle = 0.0f;
    //From the structured representation of the tracked user over a collection of sample
    //frames in the T pose, deduce the measurements above
    UserMetrics(std::vector<UserTracked> trackedFrames);
    std::string to_string();
};
