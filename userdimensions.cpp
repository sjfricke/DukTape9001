#include "math.h"
#include "stdio.h"

//Metrics about the user, where all dimensions are in pixels on the calibration
//frame, where the user is placed at 6ft from the camera in T-position
class UserMetrics {
public:
    float chest_to_head; //"Chest" here actually means something more like "bottom of neck"
    float chest_to_shoulder;
    float elbow_to_wrist;
    float shoulder_to_elbow;
    float hip_to_knee; //This measurement is based on the distance from one of left/right hip to the knee (averaged)
    float knee_to_ankle;
    //From the structured representation of the tracked user, 
    UserMetrics(Array<UserTracked>) {
        
    }
    
};

class Point2d {
public:
    float x, y;
    Point2d(float x, float y) {
        this->x = x;
        this->y = y;
    }
};

class ConfidencePoint2d {
    public:
    Point2d pos;
    float conf;
    ConfidencePoint2d(float x, float y, float conf) {
        this->pos = new Point2d(x, y, conf);
        this->conf = conf;
    }
};

class Point3d {
    float x, y, z;
};

class ConfidencePoint3d {
    public:
    Point3d pos;
    float conf;
    ConfidencePoint3d(float x, float y, float z, float conf) {
        this->pos = new Point3d(x, y, conf);
        this->conf = conf;
    }
};

class UserTracked {
    public:
    ConfidencePoint2d nose, chest, rshoulder, relbow, rwrist, 
                    lshoulder, lelbow, lwrist, rhip, rknee,
                    lankle, lhip, lknee, lankle, reye, leye,
                    rear, lear, background;
    UserTracked(Array<float> keypoints) {
        numParts = 18;
        ConfidencePoint2d** parts = {&nose, &chest, &rshoulder, &relbow, &rwrist,
                                     &lshoulder, &lelbow, &lwrist, &lhip, &rknee, &lankle,
                                     &lhip, &lknee, &lankle, &reye, &leye, &rear, &lear, &background};
        offset = 0;
        for (int i = 0; i < numParts; i++) {
            part = parts[i];
            &part = ConfidencePoint2d(keypoints[offset], keypoints[offset + 1],
                                      keypoints[offset + 2]);
            offset += 3;
        }
    }
};




