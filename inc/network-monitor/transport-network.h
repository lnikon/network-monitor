#ifndef TRANSPORT_NETWORK_H
#define TRANSPORT_NETWORK_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Utility
{
template <typename IdType, typename ObjectType> IdType getId(const ObjectType& object)
{
    return object.id;
}

template <typename IdType, typename ValueType>
std::unordered_map<IdType, ValueType> vectorToUnorderedMap(const std::vector<ValueType>& vec)
{
    std::unordered_map<IdType, ValueType> result;
    for (const auto& item : vec)
    {
        result.emplace(getId(item), item);
    }

    return result;
}
} // namespace Utility

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

private:
    struct GraphNode;
    struct GraphEdge;
    struct RouteInternal;
    struct LineInternal;

    struct GraphNode
    {
        Id id{};
        Name name{};
        long long int passengerCount{0};
        std::vector<GraphEdge> edges{};
    };

    struct GraphEdge
    {
        std::shared_ptr<RouteInternal> route{nullptr};
        std::shared_ptr<GraphNode> nextStop{nullptr};
        unsigned int travelTime{0};
    };

    struct RouteInternal
    {
        Id id{};
        Name name{};
        std::shared_ptr<LineInternal> line{nullptr};
        std::vector<std::shared_ptr<GraphNode>> stops{};
    };

    struct LineInternal
    {
        Id id{};
        Name name{};
        std::unordered_map<Id, std::shared_ptr<RouteInternal>> routes{};
    };

    std::unordered_map<Id, std::shared_ptr<GraphNode>> m_stations{};
    std::unordered_map<Id, std::shared_ptr<LineInternal>> m_lines{};

    // Helper functions
    std::shared_ptr<GraphNode> getStation(const Id& id) const;
    std::shared_ptr<LineInternal> getLine(const Id& id) const;

    bool addRouteToLine(const Route& route, const std::shared_ptr<LineInternal>& lineInternal);
};

} // namespace NetworkMonitor

#endif // TRANSPORT_NETWORK_H
