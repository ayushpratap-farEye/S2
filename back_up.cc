#include <iomanip>
#include <iostream>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include "s2/base/commandlineflags.h"
#include "s2/s2earth.h"
#include "s2/s1chord_angle.h"
#include "s2/s2closest_point_query.h"
#include "s2/s2point_index.h"
#include "s2/s2testing.h"
#include <pqxx/pqxx>
#include <map>

using namespace std;
using namespace std::chrono;

S2_DEFINE_int32(num_index_points, 50000000, "Number of points to index");


int main(int argc, char **argv) {
    //  Create a map
    map <int, S2PointIndex<int>*> m;

    cout<< "Starting the application"<<endl;
    auto startIndex = high_resolution_clock::now();
    
    //  Build an index from random points anywehre on earth
    S2PointIndex <int> index;
    for(auto i = 1; i < FLAGS_num_index_points; i++) {
        index.Add(S2Testing::RandomPoint(), i);
    }

    auto stopIndex = high_resolution_clock::now();
    double indexDuration = (duration_cast<microseconds>(stopIndex - startIndex).count())/1000000;

    cout<< "Index creation time  for "<< FLAGS_num_index_points << " points : " << indexDuration << " seconds" << endl;

    
    //  create a query to search within the given radius of a target point
    cout<<"Starting the loop"<<endl;
    cout<<"----------------------------------------------------------------------" << endl;

    //  Create query
    S2ClosestPointQuery<int> query(&index);

    while(true) {
        //  Ask for radius 
        double radius;
        double latitude;
        double longitude;

        //  Ask for laittude
        cout<<"Please input latitude : ";
        cin>>latitude;
        
        //  Ask fot longitude
        cout<<"Please input longitude : ";
        cin>>longitude;

        //  Ask for radius
        cout<<"Please input search radius : ";
        cin>>radius;

        //  Set radius to query
        query.mutable_options()->set_max_distance(S1Angle::Radians(S2Earth::KmToRadians(radius)));

        auto start = high_resolution_clock::now();
        
        //  Create point
        auto prePoint = S2LatLng::FromDegrees(latitude, longitude);
        S2Point point(prePoint);

        //  Create target
        S2ClosestPointQuery<int>::PointTarget target(point);

        auto result = query.FindClosestPoint(&target);
        if (result.is_empty()) {
            cout<<endl<<"No closest point found in given radius" << endl;
        } else {
            auto pointResult = S2LatLng(result.point());
            auto distanceResult = S2Earth::ToMeters(result.distance());
            cout<<endl;
            cout<<"Searched Point:"<< std::setw(29)<<"[" << prePoint.lat() << ", " << prePoint.lng() << "]" << endl;
            cout<<"Closest Point:"<<std::setw(30)<< "[" << pointResult.lat() << ", " << pointResult.lng() << "]" << endl;
            cout<<"Distance between points:"<< std::setw(26) << distanceResult<< " m"<<endl;
        }
        auto stop =  high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop-start).count();
        cout<<"Total execution for search:"<< std::setw(20) << duration << " microseconds" << endl;
        cout<<"----------------------------------------------------------------------" << endl;
    }
    return 0;
}