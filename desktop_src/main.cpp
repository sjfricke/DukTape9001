//Waifyou -- make anime real
//Here, for now, we're just going to read a video file and attempt to back
//out the 3d pose data from that

// ------------------------- OpenPose Library Tutorial - Pose - Example 1 - Extract from Image -------------------------
// This first example shows the user how to:
    // 1. Load an image (`filestream` module)
    // 2. Extract the pose of that image (`pose` module)
    // 3. Render the pose on a resized copy of the input image (`pose` module)
    // 4. Display the rendered pose (`gui` module)
// In addition to the previous OpenPose modules, we also need to use:
    // 1. `core` module: for the Array<float> class that the `pose` module needs
    // 2. `utilities` module: for the error & logging functions, i.e. op::error & op::log respectively

// 3rdparty dependencies
// GFlags: DEFINE_bool, _int32, _int64, _uint64, _double, _string
#include <gflags/gflags.h>
// Allow Google Flags in Ubuntu 14
#ifndef GFLAGS_GFLAGS_H_
    namespace gflags = google;
#endif
// OpenPose dependencies
#include <openpose/core/headers.hpp>
#include <openpose/filestream/headers.hpp>
#include <openpose/gui/headers.hpp>
#include <openpose/pose/headers.hpp>
#include <openpose/utilities/headers.hpp>
#include "userdimensions.hpp"
#include "unistd.h"

// See all the available parameter options withe the `--help` flag. E.g. `build/examples/openpose/openpose.bin --help`
// Note: This command will show you flags for other unnecessary 3rdparty files. Check only the flags for the OpenPose
// executable. E.g. for `openpose.bin`, look for `Flags from examples/openpose/openpose.cpp:`.
// Debugging/Other
DEFINE_int32(logging_level,             3,              "The logging level. Integer in the range [0, 255]. 0 will output any log() message, while"
                                                        " 255 will not output any. Current OpenPose library messages are in the range 0-4: 1 for"
                                                        " low priority messages and 4 for important ones.");
// Producer
DEFINE_string(image_path,               "examples/media/COCO_val2014_000000000192.jpg",     "Process the desired image.");
// OpenPose
DEFINE_string(model_pose,               "COCO",         "Model to be used. E.g. `COCO` (18 keypoints), `MPI` (15 keypoints, ~10% faster), "
                                                        "`MPI_4_layers` (15 keypoints, even faster but less accurate).");
DEFINE_string(model_folder,             "models/",      "Folder path (absolute or relative) where the models (pose, face, ...) are located.");
DEFINE_string(net_resolution,           "-1x128",       "Multiples of 16. If it is increased, the accuracy potentially increases. If it is"
                                                        " decreased, the speed increases. For maximum speed-accuracy balance, it should keep the"
                                                        " closest aspect ratio possible to the images or videos to be processed. Using `-1` in"
                                                        " any of the dimensions, OP will choose the optimal aspect ratio depending on the user's"
                                                        " input value. E.g. the default `-1x368` is equivalent to `656x368` in 16:9 resolutions,"
                                                        " e.g. full HD (1980x1080) and HD (1280x720) resolutions.");
DEFINE_string(output_resolution,        "-1x-1",        "The image resolution (display and output). Use \"-1x-1\" to force the program to use the"
                                                        " input image resolution.");
DEFINE_int32(num_gpu_start,             0,              "GPU device start number.");
DEFINE_double(scale_gap,                0.3,            "Scale gap between scales. No effect unless scale_number > 1. Initial scale is always 1."
                                                        " If you want to change the initial scale, you actually want to multiply the"
                                                        " `net_resolution` by your desired initial scale.");
DEFINE_int32(scale_number,              1,              "Number of scales to average.");
// OpenPose Rendering
DEFINE_bool(disable_blending,           false,          "If enabled, it will render the results (keypoint skeletons or heatmaps) on a black"
                                                        " background, instead of being rendered into the original image. Related: `part_to_show`,"
                                                        " `alpha_pose`, and `alpha_pose`.");
DEFINE_double(render_threshold,         0.05,           "Only estimated keypoints whose score confidences are higher than this threshold will be"
                                                        " rendered. Generally, a high threshold (> 0.5) will only render very clear body parts;"
                                                        " while small thresholds (~0.1) will also output guessed and occluded keypoints, but also"
                                                        " more false positives (i.e. wrong detections).");
DEFINE_double(alpha_pose,               0.6,            "Blending factor (range 0-1) for the body part rendering. 1 will show it completely, 0 will"
                                                        " hide it. Only valid for GPU rendering.");

//TODO: Change from video source to webcam source (though this is easy with OpenCV!)
int waifYou(std::string videoSource)
{
    op::log("WaifYou v0.1", op::Priority::High);
    // ------------------------- INITIALIZATION -------------------------
    // Step 1 - Set logging level
        // - 0 will output all the logging messages
        // - 255 will output nothing
    op::check(0 <= FLAGS_logging_level && FLAGS_logging_level <= 255, "Wrong logging_level value.",
              __LINE__, __FUNCTION__, __FILE__);
    op::ConfigureLog::setPriorityThreshold((op::Priority)FLAGS_logging_level);
    op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);
    // Step 2 - Read Google flags (user defined configuration)
    // outputSize
    const auto outputSize = op::flagsToPoint(FLAGS_output_resolution, "-1x-1");
    // netInputSize
    const auto netInputSize = op::flagsToPoint(FLAGS_net_resolution, "-1x128");
    // poseModel
    const auto poseModel = op::flagsToPoseModel(FLAGS_model_pose);
    // Check no contradictory flags enabled
    if (FLAGS_alpha_pose < 0. || FLAGS_alpha_pose > 1.)
        op::error("Alpha value for blending must be in the range [0,1].", __LINE__, __FUNCTION__, __FILE__);
    if (FLAGS_scale_gap <= 0. && FLAGS_scale_number > 1)
        op::error("Incompatible flag configuration: scale_gap must be greater than 0 or scale_number = 1.",
                  __LINE__, __FUNCTION__, __FILE__);
    // Logging
    op::log("", op::Priority::Low, __LINE__, __FUNCTION__, __FILE__);

    // Step 3 - Initialize all required classes
    op::ScaleAndSizeExtractor scaleAndSizeExtractor(netInputSize, outputSize, FLAGS_scale_number, FLAGS_scale_gap);
    op::CvMatToOpInput cvMatToOpInput;
    op::PoseExtractorCaffe poseExtractorCaffe{poseModel, FLAGS_model_folder, FLAGS_num_gpu_start};

    // Step 4 - Initialize resources on desired thread (in this case single thread, i.e. we init resources here)
    poseExtractorCaffe.initializationOnThread();
    
    //INIT COMPLETE
    //Now, read in the video file
    cv::VideoCapture video(videoSource);
    if (!video.isOpened()) {
        op::error("Could not load the video file " + videoSource, __LINE__, __FUNCTION__, __FILE__);
    }

    int num_calibration_frames = 30;

    op::log("----------------------------------", op::Priority::High);
    op::log("------CALIBRATION START-----------", op::Priority::High);
    op::log("----------------------------------", op::Priority::High);
    op::log("------HIT ENTER TO CONTINUE-------", op::Priority::High);
    op::log("------YOU WILL HAVE 5 SECONDS-----", op::Priority::High);
    op::log("------TO GET INTO T POSITION------", op::Priority::High);
    op::log("----------------------------------", op::Priority::High);
    getchar();
    sleep(5);

    
    std::vector<UserTracked2d> calibrationData = std::vector<UserTracked2d>();

    UserMetrics userMetrics;

    //MAIN LOOP
    //TODO: Termination condition
    for (;;) {

        cv::Mat frame;
        video >> frame;
        if (frame.empty()) {
            op::error("Could not load some video frame from " + videoSource, __LINE__, __FUNCTION__, __FILE__);
        }
        const op::Point<int> imageSize{frame.cols, frame.rows};

        //Get image scales
        std::vector<double> scaleInputToNetInputs;
        std::vector<op::Point<int>> netInputSizes;
        double scaleInputToOutput;
        op::Point<int> outputResolution;
        std::tie(scaleInputToNetInputs, netInputSizes, scaleInputToOutput, outputResolution)
            = scaleAndSizeExtractor.extract(imageSize);

        // Step 3 - Format input image to OpenPose input and output formats
        const auto netInputArray = cvMatToOpInput.createArray(frame, scaleInputToNetInputs, netInputSizes);

        // Step 4 - Estimate poseKeypoints
        poseExtractorCaffe.forwardPass(netInputArray, imageSize, scaleInputToNetInputs);
        const auto poseKeypoints = poseExtractorCaffe.getPoseKeypoints();

        std::vector<float> keypoints = to_nice_float_array(poseKeypoints);

        //Okay, great, now we have the pose keypoints. Time to do our magic!
        UserTracked2d trackedUser(keypoints);

        if (calibrationData.size() < num_calibration_frames) {
            //Still calibrating
            calibrationData.push_back(trackedUser);
            if (calibrationData.size() == num_calibration_frames) {
                //Generate the metric data
                userMetrics = UserMetrics(calibrationData);
                op::log("----------------------------------", op::Priority::High);
                op::log("--------CALIBRATION COMPLETE------", op::Priority::High);
                op::log("----------------------------------", op::Priority::High);
                op::log("------------USER METRICS----------", op::Priority::High);
                op::log(userMetrics.to_string(), op::Priority::High);
                op::log("----------------------------------", op::Priority::High);
            }
        }
        else {
            //Done calibrating, do stuff
        }

    }
    // Return successful message
    return 0;
}

int main(int argc, char *argv[])
{
    // Parsing command line flags
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    //TODO: read in from argv the video file name

    // Running openPoseTutorialPose1
    return waifYou("../repo/DukTape9001/test_data/testmovie.avi");
}
