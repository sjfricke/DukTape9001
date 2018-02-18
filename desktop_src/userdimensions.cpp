#include "math.h"
#include "stdio.h"
#include <iostream>
#include "userdimensions.hpp"
using std::cout;
using std::endl;

std::string UserMetrics::to_string() {
    return "Chest to Nose " + std::to_string(chest_to_nose) + " , Nose to Ear " + std::to_string(nose_to_ear)
    + ", Chest to Shoulder " + std::to_string(chest_to_shoulder) + ", Elbow to Wrist " + std::to_string(elbow_to_wrist)
    + ", Shoulder to Elbow " + std::to_string(shoulder_to_elbow) + ", Hip to Knee " + std::to_string(hip_to_knee)
    + ", Knee to Ankle " + std::to_string(knee_to_ankle);
}

UserMetrics::UserMetrics(std::vector<UserTracked2d> trackedFrames) {
    float nose_to_ear_weight = 0.0f;
    float chest_to_nose_weight = 0.0f;
    float chest_to_shoulder_weight = 0.0f;
    float elbow_to_wrist_weight = 0.0f;
    float shoulder_to_elbow_weight = 0.0f;
    float hip_to_knee_weight = 0.0f;
    float knee_to_ankle_weight = 0.0f;
    for (int i = 0; i < trackedFrames.size(); i++) {
        UserTracked2d frame = trackedFrames[i];

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

float dist(cv::Point2f a, cv::Point2f b) {
    float xdist = a.x - b.x;
    float ydist = a.y - b.y;
    return sqrtf(xdist * xdist + ydist * ydist);
}


ConfidencePoint2f::ConfidencePoint2f(float x, float y, float conf) {
    this->pos = cv::Point2f(x, y);
    this->conf = conf;
}

float ConfidencePoint2f::pairConf(ConfidencePoint2f other) {
    return this->conf * other.conf;
}

float ConfidencePoint2f::confDist(ConfidencePoint2f other) {
    return this->pairConf(other) * dist(this->pos, other.pos);
}

ConfidencePoint3f::ConfidencePoint3f(float x, float y, float z, float conf) {
    this->pos = cv::Point3f(x, y, conf);
    this->conf = conf;
}

std::vector<float> to_nice_float_array(op::Array<float> input) {
    std::vector<float> result = std::vector<float>();
    for (int i = 0; i < input.getSize(1); i++) {
        for (int j = 0; j < input.getSize(2); j++) {
            result.push_back(input[{0, i, j}]);
        }
    }
    return result;
}

// a quick abstraction to add a dimention to a 2d point
ConfidencePoint3f addZ(ConfidencePoint2f point, float z) {
    return ConfidencePoint3f(point.pos.x, point.pos.y, z, point.conf);
}


ConfidencePoint3f giveDepth(ConfidencePoint2f flatpoint, float metric, float metricNew, float offset) {
    float z = sqrt(pow(metric, 2.0) + pow(metricNew, 2.0));
    addZ(flatpoint, z + offset);
}

UserTracked3d::UserTracked3d(UserMetrics metrics, UserTracked2d user) {
    ConfidencePoint2f twodeeparts[numParts] = {user.nose, user.chest, user.rshoulder, user.relbow, user.rwrist,
                                 user.lshoulder, user.lelbow, user.lwrist, user.lhip, user.rknee, user.lankle,
                                 user.lhip, user.lknee, user.lankle, user.reye, user.leye, user.rear, user.lear};

    ConfidencePoint3f* threedeeparts[numParts] = {&nose, &chest, &rshoulder, &relbow, &rwrist,
                                                 &lshoulder, &lelbow, &lwrist, &lhip, &rknee, &lankle,
                                                 &lhip, &lknee, &lankle, &reye, &leye, &rear, &lear};
    UserMetrics metrics = metrics;
    

    
    this->chest3d = giveDepth(chest, 0, 0, defaultDistance);
    this->nose3d = giveDepth(nose, metric::chest_to_nose, nose::dist(chest), chest3d::z);
    this->rear3d = addZ(rear, nose::z);
    this->lear3d = addZ(rear, nose::z);
    this->rshoulder3d giveDepth(rshoulder, metric::chest_to_shoulder, chest::dist(rshoulder), chest3d::z);
    this->lshoulder3d giveDepth(lshoulder, metric::chest_to_shoulder, chest::dist(lshoulder), chest3d::z);
    this->relbow3d giveDepth(relbow, metric::shoulder_to_elbow, rshoulder::dist(relbow), rshoulder::z);
    this->lelbow3d giveDepth(lelbow, metric::shoulder_to_elbow, lshoulder::dist(lelbow), lshoulder::z);
    this->rwrist3d giveDepth(rwrist, metric::elbow_to_wrist, relbow::dist(rwrist), relbow::z);
    this->lwrist3d giveDepth(lwrist, metric::elbow_to_wrist, lelbow::dist(rwrist), lelbow::z);
    this->rhip3d giveDepth(rhip, 0, 0, defaultDistance);
    this->lhip3d giveDepth(rhip, 0, 0, defaultDistance);
    this->rknee3d giveDepth(rknee, metric::hip_to_knee, rhip::dist(rknee), rhip3d::z);
    this->lknee3d giveDepth(lknee, metric::hip_to_knee, rhip::dist(lknee), lhip3d::z);
    this->rankle3d giveDepth(rankle, metric::knee_to_ankle, rankle::dist(rankle), rknee::z);
    this->lankle3d giveDepth(lankle, metric::knee_to_ankle, lankle::dist(lankle), lknee::z);
}

std::vector<float> calculateAngles(Point3f newp, Point3f newref, Point3f oldp, Point3f oldref) {
        float oldx = oldref.x - oldp.x;
        float oldy = oldref.y - oldp.y;
        float oldz = oldref.z - oldp.z;
        float newx = newref.x - newp.x;
        float newy = newref.y - newp.y;
        float newz = newref.z - newp.z;
    return {atan2(newx, newz) - atan2(oldx, oldz),
            atan2(newy, newz) - atan2(oldy, oldz)}
}

std::vector<std::vector<float>> UserTracked3d::getAngles(UserTracked3d next) {
    return {
        calculateAngles(next.threedeeparts.rwrist.pos, next.threedeeparts.rshoulder.pos, this->threedeeparts.rwrist.pos, this->threedeeparts.rshoulder.pos);
        calculateAngles(next.threedeepalts.lwlist.pos, next.threedeepalts.lshoulder.pos, this->threedeepalts.lwrist.pos, this->threedeepalts.lshoulder.pos);
        calculateAngles(next.threedeeparts.rankle.pos, next.threedeeparts.rhip.pos, this->threedeeparts.rankle.pos, this->threedeeparts.rhip.pos);
        calculateAngles(next.threedeeparts.lankle.pos, next.threedeeparts.lhip.pos, this->threedeeparts.lankle.pos, this->threedeeparts.lhip.pos);
    }
}

UserTracked2d::UserTracked2d(std::vector<float> keypoints) {
    int numParts = 18;
    ConfidencePoint2f* parts[numParts] = {&nose, &chest, &rshoulder, &relbow, &rwrist,
                                 &lshoulder, &lelbow, &lwrist, &lhip, &rknee, &lankle,
                                 &lhip, &lknee, &lankle, &reye, &leye, &rear, &lear};
    int offset = 0;
    for (int i = 0; i < numParts; i++) {
        ConfidencePoint2f* part = parts[i];
        *part = ConfidencePoint2f(keypoints[offset], keypoints[offset + 1],
                                  keypoints[offset + 2]);
        std::vector<float> floatArray();
        offset += 3;
    }
}
