#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/csma-module.h"
#include "ns3/netanim-module.h"
#include "ns3/bulk-send-helper.h"
//
//
// n0   n1   n2   n3   n4
//  |    |    |    |    |
// =======================
//
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Lab5_1Script");

int main (int argc, char *argv[])
{ 

  double bandwidths[] = {0.5, 1, 4, 10, 24, 100};
  uint32_t delays[] = {5000, 10000, 15000, 20000, 25000,30000};//in ms
  LogComponentEnable("Lab5_1Script", LOG_LEVEL_INFO);
  for(uint32_t j=0;j<6;j++)
  {
  double bandwidth=bandwidths[j];
  uint32_t delay=delays[j];
  NodeContainer csmaNodes;
  csmaNodes.Create (5);
  
  CsmaHelper csma; 
  csma.SetChannelAttribute("DataRate", DataRateValue(DataRate(bandwidth * 1000000)));// Convert to bps
  csma.SetChannelAttribute("Delay", TimeValue(MicroSeconds(delay)));

  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);

  InternetStackHelper stack;
  stack.Install (csmaNodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  
  Ipv4InterfaceContainer interfaces = address.Assign (csmaDevices);
  
  BulkSendHelper source("ns3::TcpSocketFactory",InetSocketAddress(interfaces.GetAddress(1),9));
  source.SetAttribute("MaxBytes",UintegerValue(1000000));
  
  ApplicationContainer sourceApps;
  sourceApps.Add(source.Install(csmaNodes.Get(0)));
  sourceApps.Start (Seconds (0.0));
  sourceApps.Stop (Seconds (10.0));
  
  PacketSinkHelper sink ("ns3::TcpSocketFactory",InetSocketAddress (Ipv4Address::GetAny (),9));
  ApplicationContainer sinkApps = sink.Install (csmaNodes.Get (1));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (10.0));

  char abc[20];
  sprintf(abc, "Lab5_1trace%d.tr", j);
  AsciiTraceHelper ascii;
  Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (abc);
  csma.EnableAsciiAll (stream);
  AnimationInterface anim("Lab5_1.xml");
  for (uint32_t i = 0; i < 5; i++) {
        anim.SetConstantPosition(csmaNodes.Get(i), 100, 10*i);
    }
  Simulator::Run();
  Simulator::Destroy();
  
  Ptr<PacketSink> sink1 = DynamicCast<PacketSink>(sinkApps.Get(0));
  std::cout << "Bandwidth: " << bandwidth << " Mbps, Delay: " << delay / 1000.0 << " ms, Total Bytes Received: " << sink1->GetTotalRx() << std::endl;
  }
  

  return 0;
}

