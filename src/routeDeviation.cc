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
#include <typeinfo>

using namespace std;
using namespace std::chrono;

#define CONNECTION_STRING "dbname=gps_development_postgres user=postgres password=user@123 hostaddr=127.0.0.1 port=5432"
#define SELECT_QUERY "SELECT route_mapping_id,latitude,longitude FROM ideal_routes LIMIT 10"

pqxx::result query() {
    pqxx::connection dbConn(CONNECTION_STRING);
    pqxx::work txn{dbConn};
    pqxx::result r{txn.exec(SELECT_QUERY)};
    txn.commit();
    return r;
}

int main(int argc, char **argv) {
    //  Create a map
    map <int, S2PointIndex<int>*> m;
    int row_nums = 0;

    std::cout<< "Starting the application"<<endl;
    auto startIndex = high_resolution_clock::now();
    try
    {
        //  Get data from DB
        pqxx::result dbData{query()};

        std::cout << "Totals rows found in db : " << dbData.size() << endl;

        for(auto row : dbData){
            // std::cout << "row : " << row. << endl;
            ++row_nums;
            auto t = (row[1].c_str());
            int routeId = atoi(row[0].c_str());
            double latitude = atof(row[1].c_str());
            double longitude = atof(row[2].c_str());  
            
            std::cout<<
            "routeId : " 
            << routeId << " | latitude : " 
            << latitude << " | longitude : " 
            << longitude << endl;

            //  Check if routeid exists in 
            if(m.find(routeId) == m.end()) {
                //  Key does not exits
                //  Create new S2Point index
                S2PointIndex <int> tmpIndex;
                auto tmp = &tmpIndex;

                //  Convert lat/long to S2Point
                S2Point point(S2LatLng::FromDegrees(latitude, longitude));
                    
                //  Add point to index
                tmp->Add(point, row_nums);

                std::cout<<"1. Index with routeId " << routeId << " contains " << tmp->num_points() << " points.\n";
                
                m.insert(pair<int, S2PointIndex<int>*>(routeId, tmp));
            } 
            else 
            {
                //  Key exists in 
                S2PointIndex<int>* tmpIndex = m.at(routeId);

                //  Convert lat/long to S2Point
                S2Point point(S2LatLng::FromDegrees(latitude, longitude));
                
                //  Add point to index
                tmpIndex->Add(point, row_nums);
                std::cout<<"2. Index with routeId " << routeId << " contains " << tmpIndex->num_points() << " points.\n";
                exit(0);

                //  Reinsert index to map
                m[routeId] = tmpIndex;

                std::cout<<"2. Index with routeId " << routeId << " contains " << tmpIndex->num_points() << " points.\n";
            }
            // std::cout << std::endl;
        }
    }
    catch(pqxx::sql_error const &e)
    {
        std::cerr << "SQL error: " << e.what() << std::endl;
        std::cerr << "Query was: " << e.query() << std::endl;
        return 2;
    }
    catch (std::exception const &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    auto stopIndex = high_resolution_clock::now();
    auto indexDuration = duration_cast<seconds>(stopIndex - startIndex).count();
    std::cout<< "Index creation time  for "<< row_nums << " points : " << indexDuration << " seconds" << endl;


    while(true) {
        int routeMapId; 
        double radius;
        double latitude;
        double longitude;

        //  Ask for route map id
        std::cout<<"Please input route map id : ";
        std::cin>>routeMapId;

        //  Ask for laittude
        std::cout<<"Please input latitude : ";
        cin>>latitude;
        
        //  Ask fot longitude
        std::cout<<"Please input longitude : ";
        cin>>longitude;

        //  Ask for radius
        std::cout<<"Please input search radius : ";
        cin>>radius;

        auto start = high_resolution_clock::now();

        //  Check if routeMapId exists in map
        if(m.find(routeMapId) == m.end()) {
            //  route does not exists in map
            std::cout<<"Sorry the route map id you provided does not exists\n";
        } else {
            //  Route exists in map
            S2PointIndex<int> *index = m[routeMapId];

            //  Create query
            S2ClosestPointQuery <int> query(&(*index));
            
            //  Set options in quert
            query.mutable_options()->set_max_distance(S1Angle::Radians(S2Earth::KmToRadians(radius)));

            //  Create point
            S2Point point(S2LatLng::FromDegrees(latitude, longitude));

            //  Create target
            S2ClosestPointQuery<int>::PointTarget target(point);

            //  Get result
            auto result = query.FindClosestPoint(&target);

            if(result.is_empty()) {
                //  Result is empty
                std::cout<<"No closest point found in given radius on the route.\n";
            } else {
                auto centerResult = target.GetCapBound().center();
                auto pointResult = result.point();
                auto distanceResult = result.distance();

                std::cout<<"Searched point (ploted) : "  << centerResult << endl;
                std::cout<<"Closest point (ploted) : " << pointResult << endl;
                std::cout<<"Distance between points : " << distanceResult<<endl;
            }
        }

        auto stop =  high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(stop-start).count();
        std::cout<<"Total execution for search : " << duration << " microseconds" << endl;
        std::cout<<"----------------------------------------------------------------------" << endl;
    }
    return 0;
}