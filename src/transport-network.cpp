#include <network-monitor/transport-network.h>

namespace NetworkMonitor
{

TransportNetwork::TransportNetwork()
{
}

TransportNetwork::TransportNetwork(const TransportNetwork& copied)
{
}

TransportNetwork::TransportNetwork(TransportNetwork&& moved)
{
}

TransportNetwork& TransportNetwork::operator=(const TransportNetwork& copied)
{
}

TransportNetwork& TransportNetwork::operator=(TransportNetwork&& moved)
{
}

bool TransportNetwork::AddStation(const Station& station)
{
	return false;
}

bool TransportNetwork::AddLine(const Line& line)
{
}

bool TransportNetwork::RecordPassengerEvent(const Id& station, const PassengerEvent& event)
{
}

long long int TransportNetwork::GetPassengerCount(const Id& station) const
{
}

std::vector<Id> TransportNetwork::GetRoutesServingStation(const Id& station) const
{
}

bool TransportNetwork::SetTravelTime(const Id& stationA,
                                     const Id& stationB,
                                     const unsigned int travelTime)
{
}

unsigned int TransportNetwork::GetTravelTime(const Id& stationA, const Id& stationB)
{
}

unsigned int TransportNetwork::GetTravelTime(const Line& line,
                                             const Route& route,
                                             const Id& stationA,
                                             const Id& stationB)
{
}

} // namespace NetworkMonitor
