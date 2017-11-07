#include <opencv2/opencv.hpp>
#include <opencv2/tracking.hpp>
#include <opencv2/core/ocl.hpp>
#include <deque>

using namespace cv;
using namespace std;

struct sTracker{
    Ptr<Tracker> tracker;
    Rect2d bbox;
    bool ok;
};


int main(int argc, char **argv)
{
    // List of tracker types in OpenCV 3.2
    // NOTE : GOTURN implementation is buggy and does not work.
    string trackerTypes[6] = {"BOOSTING", "MIL", "KCF", "TLD","MEDIANFLOW", "GOTURN"};
    // vector <string> trackerTypes(types, std::end(types));

    // Create a tracker
    string trackerType = trackerTypes[2];

    Ptr<Tracker> tracker;

    deque<sTracker> allTrackers;

    // Read video
    VideoCapture video("Videos/original.avi");

    // Exit if video is not opened
    if(!video.isOpened())
    {
        cout << "Could not read video file" << endl;
        return 1;

    }

    ifstream input("Videos/detected_vehicles_output.txt");
    string line;
    Rect2d tempFromFile;
    sTracker temp;

    // Read first frame
    Mat frame;
    bool ok = video.read(frame);

    int numOfFrame = 0;

    bool flag = false;

    while(video.read(frame))
    {
            getline(input, line);
            getline(input, line);

            while(line != ""){
              istringstream cub(line);
              cub >> tempFromFile.x >> tempFromFile.y >> tempFromFile.width >> tempFromFile.height;

              for (sTracker &tr : allTrackers){
                  if ((tr.bbox & tempFromFile).area() > 0 && tr.ok){
                      flag = true;
                  }
              }

              if (!flag) {
                  temp.bbox = tempFromFile;

                  if (trackerType == "BOOSTING")
                      temp.tracker = TrackerBoosting::create();
                  if (trackerType == "MIL")
                      temp.tracker = TrackerMIL::create();
                  if (trackerType == "KCF")
                      temp.tracker = TrackerKCF::create();
                  if (trackerType == "TLD")
                      temp.tracker = TrackerTLD::create();
                  if (trackerType == "MEDIANFLOW")
                      temp.tracker = TrackerMedianFlow::create();
                  if (trackerType == "GOTURN")
                      temp.tracker = TrackerGOTURN::create();

                  //temp.tracker = TrackerKCF::create();
                  temp.tracker->init(frame, tempFromFile);
                  temp.ok = true;
                  allTrackers.push_back(temp);
              }

              flag = false;

              getline(input, line);

        }

        // Update the tracking result

        auto tr = allTrackers.begin();
        for(;tr != allTrackers.end();){
          (*tr).ok = (*tr).tracker->update(frame, (*tr).bbox);

          if ((*tr).ok) {
              rectangle(frame, (*tr).bbox, Scalar( 255, 0, 0 ), 2, 1 );
              tr++;
          } else {
              tr = allTrackers.erase(tr);
          }
        }

        // Display tracker type on frame
        putText(frame, trackerType + " Tracker NumOfFrame " + to_string(numOfFrame), Point(100,20), FONT_HERSHEY_SIMPLEX, 0.75, Scalar(50,170,50),2);

        // Display frame.
        imshow("Tracking", frame);

        numOfFrame++;

        // Exit if ESC pressed.
        int k = waitKey(30);
        if(k == 27)
        {
            break;
        }

    }
}
