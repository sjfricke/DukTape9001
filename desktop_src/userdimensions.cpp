#include "math.h"
#include "stdio.h"

//Metrics about the user, where all dimensions are in pixels on the calibration
//frame, where the user is placed at 6ft from the camera in T-position
class UserMetrics {
public:
    float chest_to_nose = 0.0f; //"Chest" here actually means something more like "bottom of neck"
    float nose_to_ear = 0.0f;
    float chest_to_shoulder = 0.0f;
    float elbow_to_wrist = 0.0f;
    float shoulder_to_elbow = 0.0f;
    float hip_to_knee = 0.0f; //This measurement is based on the distance from one of left/right hip to the knee (averaged)
    float knee_to_ankle = 0.0f;
    //From the structured representation of the tracked user over a collection of sample
    //frames in the T pose, deduce the measurements above
    UserMetrics(Array<UserTracked> trackedFrames) {
        float nose_to_ear_weight = 0.0f;
        float chest_to_nose_weight = 0.0f;
        float chest_to_shoulder_weight = 0.0f;
        float elbow_to_wrist_weight = 0.0f;
        float shoulder_to_elbow_weight = 0.0f;
        float hip_to_knee_weight = 0.0f;
        float knee_to_ankle_weight = 0.0f;
        for (int i = 0; i < trackedFrames.length(); i++) {
            frame = trackedFrames[i];

            nose_to_ear_weight += frame.nose.pairConf(frame.lear) + frame.nose.pairConf(frame.rear);
            this->nose_to_ear += frame.nose.confDist(frame.lear) + frame.nose.confDist(frame.rear);

            chest_to_nose_weight += frame.nose.pairConf(frame.chest);
            this->chest_to_nose += frame.nose.confDist(frame.chest);

            chest_to_shoulder_weight += frame.chest.pairConf(frame.lshoulder) + 
                                        frame.chest.pairConf(frame.rshoulder);
            this->chest_to_shoulder += frame.chest.confDist(frame.lshoulder) +
                                       frame.chest.confDist(frame.rshoulder);

            elbow_to_wrist_weight += frame.relbow.pairConf(frame.rwrist) + 
                                     frame.lelbow.pairConf(frame.lwrist);
            this->elbow_to_wrist += frame.relbow.confDist(frame.rwrist) + 
                                     frame.lelbow.confDist(frame.lwrist);

            shoulder_to_elbow_weight += frame.rshoulder.pairConf(frame.relbow) + 
                                     frame.lshoulder.pairConf(frame.lelbow);
            this->shoulder_to_elbow += frame.rshoulder.confDist(frame.relbow) + 
                                     frame.lshoulder.confDist(frame.lelbow);

            hip_to_knee_weight += frame.rhip.pairConf(frame.rknee) + 
                                     frame.lhip.pairConf(frame.lknee);
            this->hip_to_knee += frame.rhip.confDist(frame.rknee) + 
                                     frame.lhip.confDist(frame.lknee);

            knee_to_ankle_weight += frame.rankle.pairConf(frame.rknee) + 
                                     frame.lankle.pairConf(frame.lknee);
            this->knee_to_ankle += frame.rankle.confDist(frame.rknee) + 
                                     frame.lankle.confDist(frame.lknee);

            //Welp, glad all that trash is outta the way
        }
        //Done tracking all the frames, now normalize by probability weight
        this->nose_to_ear /= nose_to_ear_weight;
        this->chest_to_nose /= chest_to_nose_weight;
        this->chest_to_shoulder /= chest_to_shoulder_weight;
        this->elbow_to_wrist /= elbow_to_wrist_weight;
        this->shoulder_to_elbow /= shoulder_to_elbow_weight;
        this->hip_to_knee /= hip_to_knee_weight;
        this->knee_to_ankle /= knee_to_ankle_weight;
        //We did it :D
    }
    
};

class Point2d {
public:
    float x, y;
    Point2d(float x, float y) {
        this->x = x;
        this->y = y;
    }
    float dist(Point2d other) {
        return sqrtf(this->x * other.x + this-> y * other.y)
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
    //Overall confidence of two points
    float pairConf(ConfidencePoint2d other) {
        return this->conf * other.conf;
    }
    //Distance between two points weighted by confidence
    float confDist(ConfidencePoint2d other) {
        return this->pairwiseConfidence(other) * this->pos.dist(other.pos);
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




