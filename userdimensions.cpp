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
    float dist(Pont2d other) {
      return sqrt(pow((this->x - other.x), 2.0) + pow((this-> y - other.y), 2.0))
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
    ConfidencePoint3d nose3d, chest3d, rshoulder3d, relbow3d, rwrist3d, 
                      lshoulder3d, lelbow3d, lwrist3d, rhip3d, rknee3d,
                      lankle3d, lhip3d, lknee3d, lankle3d, reye3d, leye3d,
                      rear3d, lear3d, background3d;
    int numParts = 18;
    int defaultDistance = 10.0;
    UserMetrics metrics;
    UserTracked(Array<float> keypoints, Array<UserTracked> trackedFrames) {
        calibrate(trackedFrames);
        update(keypoints);
    }
    
    void update(Array<float> keypoints) {
        ConfidencePoint2d** parts = {&nose, &chest, &rshoulder, &relbow, &rwrist,
                                     &lshoulder, &lelbow, &lwrist, &lhip, &rknee, &lankle,
                                     &lhip, &lknee, &lankle, &reye, &leye, &rear, &lear, &background};

        ConfidencePoint3d** parts3d = {&nose3d, &chest3d, &rshoulder3d, &relbow3d, &rwrist3d,
                                     &lshoulder3d, &lelbow3d, &lwrist3d, &lhip3d, &rknee3d, &lankle3d,
                                     &lhip3d, &lknee3d, &lankle3d, &reye3d, &leye3d, &rear3d, &lear3d, &background3d};
        int offset = 0;
        for (int i = 0; i < numParts; i++) {
            Confidence2d* part = parts[i];
            &part = ConfidencePoint2d(keypoints[offset], keypoints[offset + 1],
                                      keypoints[offset + 2]);
            part3d = parts3d[i];
            &part3d = giveDepth(part, 0, 0, defaultDistance);
            offset += 3;
        }
            // float chest_to_nose = 0.0f; //"Chest" here actually means something more like "bottom of neck"
            // float nose_to_ear = 0.0f;
            // float chest_to_shoulder = 0.0f;
            // float elbow_to_wrist = 0.0f;
            // float shoulder_to_elbow = 0.0f;
            // float hip_to_knee = 0.0f; //This measurement is based on the distance from one of left/right hip to the knee (averaged)
            // float knee_to_ankle = 0.0f;
        // this->chest3d = giveDepth(chest, 0, 0, defaultDistance);
        // this->nose3d = giveDepth(nose, metric::chest_to_nose, nose::dist(chest), chest3d::z);
        // this->rear3d = addZ(rear, nose::z);
        // this->lear3d = addZ(rear, nose::z);
        // this->rshoulder3d giveDepth(rshoulder, metric::chest_to_shoulder, chest::dist(rshoulder), chest3d::z);
        // this->lshoulder3d giveDepth(lshoulder, metric::chest_to_shoulder, chest::dist(lshoulder), chest3d::z);
        // this->relbow3d giveDepth(relbow, metric::shoulder_to_elbow, rshoulder::dist(relbow), rshoulder::z);
        // this->lelbow3d giveDepth(lelbow, metric::shoulder_to_elbow, lshoulder::dist(lelbow), lshoulder::z);
        // this->rwrist3d giveDepth(rwrist, metric::elbow_to_wrist, relbow::dist(rwrist), relbow::z);
        // this->lwrist3d giveDepth(lwrist, metric::elbow_to_wrist, lelbow::dist(rwrist), lelbow::z);
        // this->rhip3d giveDepth(rhip, 0, 0, defaultDistance);
        // this->lhip3d giveDepth(rhip, 0, 0, defaultDistance);
        // this->rknee3d giveDepth(rknee, metric::hip_to_knee, rhip::dist(rknee), rhip3d::z);
        // this->lknee3d giveDepth(lknee, metric::hip_to_knee, rhip::dist(lknee), lhip3d::z);
        // this->rankle3d giveDepth(rankle, metric::knee_to_ankle, rankle::dist(rankle), rknee::z);
        // this->lankle3d giveDepth(lankle, metric::knee_to_ankle, lankle::dist(lankle), lknee::z);
    }
      
    UserMetrics calibrate(Array<UserTracked> trackedFrames) {
        this->metrics = new UserMetric(trackedFrames);
    }
};

// a quick abstraction to add a dimention to a 2d point
ConfidencePoint3d addZ(ConfidencePoint2d point, float z) {
    return new ConfidencePoint3d(point->x, point->y, z);
}


ConfidencePoint3d giveDepth(ConfidencePoint2d flatpoint, float metric, float metricNew, float offset) {
    float z = sqrt(pow(metric, 2.0), pow(metricNew, 2.0));
    addZ(point, z + offset);
}

