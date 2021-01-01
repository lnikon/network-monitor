#ifndef TRANSPORT_NETWORK_H
#define TRANSPORT_NETWORK_H

#include <memory>
#include <string>
#include <vector>

namespace NetworkMonitor
{

using Id = std::string;
using Name = std::string;

struct Station
{
    Id id{};
    Name name{};

    bool operator==(const Station& other);
};

struct Route
{
    Id id{};
    Name name{};
    Id lineId{};
    Id startStationId{};
    Id endStationId{};
    std::vector<Id> stops{};

    bool operator==(const Station& other);
};

struct Line
{
    Id id;
    Name name;
    std::vector<Route> routes;
};

class TransportNetwork
{
public:
    enum class PassengerEvent
    {
        In,
        Out
    };

    TransportNetwork();
    TransportNetwork(const TransportNetwork& copied);
    TransportNetwork(TransportNetwork&& moved);
    TransportNetwork& operator=(const TransportNetwork& copied);
    TransportNetwork& operator=(TransportNetwork&& moved);

    bool AddStation(const Station& station);
    bool AddLine(const Line& line);

    bool RecordPassengerEvent(const Id& station, const PassengerEvent& event);

    long long int GetPassengerCount(const Id& station) const;

    std::vector<Id> GetRoutesServingStation(const Id& station) const;

    bool SetTravelTime(const Id& stationA, const Id& stationB, const unsigned int travelTime);

    unsigned int GetTravelTime(const Id& stationA, const Id& stationB);

    unsigned int
    GetTravelTime(const Line& line, const Route& route, const Id& stationA, const Id& stationB);
};

} // namespace NetworkMonitor

#endif // TRANSPORT_NETWORK_H
