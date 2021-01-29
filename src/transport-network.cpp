#include <network-monitor/transport-network.h>

#include <algorithm>
#include <stdexcept>

namespace NetworkMonitor
{

std::vector<TransportNetwork::GraphEdge>::const_iterator
TransportNetwork::GraphNode::FindEdgeForRoute(const std::shared_ptr<RouteInternal>& route)
{
    return std::find_if(std::begin(edges), std::end(edges), [&route](const auto& edge) {
        return edge.route == route;
    });
}

TransportNetwork::TransportNetwork() = default;

TransportNetwork::TransportNetwork(const TransportNetwork& copied) = default;

TransportNetwork::TransportNetwork(TransportNetwork&& moved) = default;

TransportNetwork& TransportNetwork::operator=(const TransportNetwork& copied) = default;

TransportNetwork& TransportNetwork::operator=(TransportNetwork&& moved) = default;

bool TransportNetwork::AddStation(const Station& station)
{
    if (getStation(station.id) != nullptr)
    {
        return false;
    }

    auto node{std::make_shared<GraphNode>(GraphNode{station.id, station.name, 0, {}})};
    m_stations.emplace(node->id, std::move(node));
    return true;
}

bool TransportNetwork::AddLine(const Line& line)
{
    if (getLine(line.id) != nullptr)
    {
        return false;
    }

    auto lineInternal{std::make_shared<LineInternal>(LineInternal{line.id, line.name, {}})};
    for (const auto& route : line.routes)
    {
        if (!addRouteToLine(route, lineInternal))
        {
            return false;
        }
    }

    m_lines.emplace(lineInternal->id, lineInternal);

    return true;
}

bool TransportNetwork::RecordPassengerEvent(const PassengerEvent& event)
{
    const auto node{getStation(event.stationId)};
    if (node == nullptr)
    {
        return false;
    }

    switch (event.type)
    {
    case PassengerEvent::Type::In:
        node->passengerCount++;
        return true;
    case PassengerEvent::Type::Out:
        node->passengerCount--;
        return true;
    default:
        return false;
    };
}

long long int TransportNetwork::GetPassengerCount(const Id& station) const
{
    const auto node{getStation(station)};
    if (nullptr == node)
    {
        throw std::runtime_error("Could not find station in the network: " + station);
    }

    return node->passengerCount;
}

std::vector<Id> TransportNetwork::GetRoutesServingStation(const Id& station) const
{
    const auto node{getStation(station)};
    if (nullptr == node)
    {
        throw std::runtime_error("Could not find station in the network: " + station);
    }

    const auto& edges{node->edges};
    std::vector<Id> routes{};
    routes.reserve(edges.size());
    const auto size = edges.size();
    for (const auto& edge : edges)
    {
        routes.push_back(edge.route->id);
    }

    // FIXME: In the worst case, we are iterating over all routes
    // for all lines in the network. We may want to optimize this.
    for (const auto& [_, line] : m_lines)
    {
        for (const auto& [_, route] : line->routes)
        {
            const auto endStop{route->stops.back()};
            if (endStop == node)
            {
                routes.push_back(route->id);
            }
        }
    }

	return routes;
}

bool TransportNetwork::SetTravelTime(const Id& stationA,
                                     const Id& stationB,
                                     const unsigned int travelTime)
{
	const auto stationANode{getStation(stationA)};
	const auto stationBNode{getStation(stationB)};
	if (stationANode == nullptr || stationBNode == nullptr)
	{
		return false;
	}

	bool foundAnyEdge{false};
	auto setTravelTime {[&foundAnyEdge, &travelTime](auto from, auto to){
		for (auto& edge: from->edges)
		{
			if (edge.nextStop == to)
			{
				edge.travelTime = travelTime;
				foundAnyEdge = true;
			}
		}
	}};

	setTravelTime(stationANode, stationBNode);
	setTravelTime(stationBNode, stationANode);

	return foundAnyEdge;
}

unsigned int TransportNetwork::GetTravelTime(const Id& stationA, const Id& stationB)
{
	if (stationA == stationB)
	{
		return 0;
	}

    const auto stationANode{getStation(stationA)};
    const auto stationBNode{getStation(stationB)};
    if (stationANode == nullptr || stationBNode == nullptr)
    {
        return 0;
    }

    for (const auto& edge : stationANode->edges)
    {
        if (edge.nextStop == stationBNode)
        {
            return edge.travelTime;
        }
    }

    for (const auto& edge : stationBNode->edges)
    {
        if (edge.nextStop == stationANode)
        {
            return edge.travelTime;
        }
    }

	return 0;
}

unsigned int TransportNetwork::GetTravelTime(const Id& line,
                                             const Id& route,
                                             const Id& stationA,
                                             const Id& stationB)
{
    auto routeInternal{getRoute(line, route)};
    if (routeInternal == nullptr)
    {
        return 0;
    }

    const auto& stationANode{getStation(stationA)};
    const auto& stationBNode{getStation(stationB)};
    if (stationANode == nullptr || stationBNode == nullptr)
    {
        return 0;
    }

    unsigned int travelTime{0};
    bool found{false};
    for (const auto& stop : routeInternal->stops)
    {
        if (stop == stationANode)
        {
            found = true;
        }

        if (stop == stationBNode)
        {
            return travelTime;
        }

        if (found)
        {
            auto edgeIt{stop->FindEdgeForRoute(routeInternal)};
            if (edgeIt == stop->edges.end())
            {
                return 0;
            }

            travelTime += edgeIt->travelTime;
        }
    }

    return travelTime;
}

std::shared_ptr<TransportNetwork::GraphNode> TransportNetwork::getStation(const Id& id) const
{
    auto itStation = m_stations.find(id);
    return (itStation != m_stations.end() ? itStation->second : nullptr);
}

std::shared_ptr<TransportNetwork::LineInternal> TransportNetwork::getLine(const Id& id) const
{
    auto itLine = m_lines.find(id);
    return (itLine != m_lines.end() ? itLine->second : nullptr);
}

bool TransportNetwork::addRouteToLine(
    const Route& route,
    const std::shared_ptr<TransportNetwork::LineInternal>& lineInternal)
{
    if (lineInternal->routes.find(route.id) != lineInternal->routes.end())
    {
        return false;
    }

    std::vector<std::shared_ptr<GraphNode>> stops{};
    stops.reserve(route.stops.size());
    for (const auto& stop : route.stops)
    {
        const auto& station{getStation(stop)};
        if (station == nullptr)
        {
            return false;
        }

        stops.push_back(station);
    }

    auto routeInternal{std::make_shared<RouteInternal>(
        RouteInternal{route.id, route.name, lineInternal, std::move(stops)})};

    for (size_t idx{0}; idx < routeInternal->stops.size() - 1; ++idx)
    {
        const auto& thisStop{routeInternal->stops[idx]};
        const auto& nextStop{routeInternal->stops[idx + 1]};
        GraphEdge edge{routeInternal, nextStop, 0};

        thisStop->edges.emplace_back(std::move(edge));
    }

    lineInternal->routes[route.id] = std::move(routeInternal);

    return true;
}

std::shared_ptr<TransportNetwork::LineInternal> TransportNetwork::getLine(const Id& lineId)
{
    auto lineIt = m_lines.find(lineId);
    if (lineIt == m_lines.end())
    {
        return nullptr;
    }

    return lineIt->second;
}

std::shared_ptr<TransportNetwork::RouteInternal> TransportNetwork::getRoute(const Id& lineId,
                                                                            const Id& routeId)
{
    auto line{getLine(lineId)};
    if (!line)
    {
        return nullptr;
    }

    const auto& routes{line->routes};
    auto routeIt = routes.find(routeId);
    if (routeIt == routes.end())
    {
        return nullptr;
    }

    return routeIt->second;
}

} // namespace NetworkMonitor
