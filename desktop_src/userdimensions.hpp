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
    float getZ() {
        return this->pos.z;
    }
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



class UserTracked2d {
    public:
    UserTracked2d() { }
    ConfidencePoint2f nose, chest, rshoulder, relbow, rwrist, 
                    lshoulder, lelbow, lwrist, rhip, rknee,
                    rankle, lhip, lknee, lankle, reye, leye,
                    rear, lear;
    UserTracked2d(std::vector<float> keypoints);
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
    UserMetrics(std::vector<UserTracked2d> trackedFrames);
    std::string to_string();
};


ConfidencePoint3f addZ(ConfidencePoint2f point, float z);

ConfidencePoint3f giveDepth(ConfidencePoint2f flatpoint, float metric, float metricNew, float offset);

class UserTracked3d {

    public:
    int numParts = 18;
    int defaultDistance = 10.0;
    UserTracked3d() { }
    ConfidencePoint3f nose, chest, rshoulder, relbow, rwrist, 
                    lshoulder, lelbow, lwrist, rhip, rknee,
                    rankle, lhip, lknee, lankle, reye, leye,
                    rear, lear;
    UserTracked3d(UserMetrics metrics, UserTracked2d user);
};
float distc(ConfidencePoint2f x, ConfidencePoint2f y);
