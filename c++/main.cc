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

using namespace std::chrono;

S2_DEFINE_int32(num_index_points, 10000000, "Number of points to index");
S2_DEFINE_int32(num_queries, 100000, "Number of queries");
S2_DEFINE_double(query_radius_km, 1, "Query radius in kilometers");

int main(int argc, char ** argv) {
    auto indexTimerStart = high_resolution_clock::now();
    // Build an index containing random points anywhere on the Earth.
    S2PointIndex < int > index;
    for (int i = 0; i < FLAGS_num_index_points; ++i) {
      index.Add(S2Testing::RandomPoint(), i);
    }
    auto indexTimerStop = high_resolution_clock::now();
    auto indexDuration = duration_cast<microseconds>(indexTimerStop - indexTimerStart);

    std::cout<< "Index creation time : " << indexDuration.count() << std::endl;

    // Create a query to search within the given radius of a target point.
    S2ClosestPointQuery < int > query( & index);
    query.mutable_options()->set_max_distance(
      S1Angle::Radians(S2Earth::KmToRadians(FLAGS_query_radius_km)));

    // Repeatedly choose a random target point, and count how many index points
    // are within the given radius of that point.
    int64_t num_found = 0;
    auto startQuery = high_resolution_clock::now();
    for (int i = 0; i < FLAGS_num_queries; ++i) {
      S2ClosestPointQuery < int > ::PointTarget target(S2Testing::RandomPoint());
      // int count= query.FindClosestPoints(&target).size();
      auto result = query.FindClosestPoint(&target);
      // auto resultV = query.FindClosestPoints( & target);
      // for (auto result: resultV) {
        if (!result.is_empty()) {
          num_found += 1;
          auto tmp1 = target.GetCapBound();
          auto center = tmp1.center();
          auto tmp = tmp1.GetCentroid();
          auto x = tmp.x();
          auto y = tmp.y();
          auto z = tmp.z();
          // std::cout << "center : " << center << ", point : " << result.point() << ", distance : " << result.distance() << std::endl;
        // std::cout << "------------------------------------------------------------------------------------------" << std::endl;
        }
        else {
          // std::cout<<"result is empty" << std::endl;
        }
      }
    // }

    auto stopQuery = high_resolution_clock::now();
    auto queryDuration = duration_cast<microseconds>(stopQuery - startQuery);
    std::cout<< "Total query duration : " << queryDuration.count() << std::endl;

    std::printf("Found %"
      PRId64 " points in %d queries\n",
      num_found, FLAGS_num_queries); return  0;
}