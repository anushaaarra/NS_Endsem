#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/netanim-module.h"

using namespace ns3;

int main (int argc, char *argv[])
{
  //CommandLine cmd (__FILE__);
  //cmd.Parse (argc, argv);

  //Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create(10);

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("2ms"));

  InternetStackHelper stack;
  stack.Install(nodes);

  NetDeviceContainer devices[45];
  int count=0;
  for (uint32_t i = 0; i < 10; i++) {
    for (uint32_t j = i + 1; j < 10; j++) {
      devices[count].Add(p2p.Install(nodes.Get(i), nodes.Get(j)));
      count++;
    }
  }

  Ipv4AddressHelper address[45];
  for(uint32_t i=0;i<45;i++)
  {
    char abc[10];
    sprintf(abc, "10.1.%d.0", i+1);
    address[i].SetBase(abc,"255.255.255.0");
  }
  
  Ipv4InterfaceContainer interfaces;
  for(uint32_t i=0;i<45;i++)
  {
    interfaces.Add(address[i].Assign(devices[i]));
  }

  for (uint32_t i = 0; i < 10; i++) {
    UdpEchoServerHelper echoServer(9);
    ApplicationContainer serverApps = echoServer.Install(nodes.Get(i));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));
  }

  for (uint32_t i = 0; i < 10; i++) {
    for (uint32_t j = 0; j < 10; j++) {
        if (i != j) { // Exclude the current node
            UdpEchoClientHelper echoClient(interfaces.GetAddress(j), 9);
            echoClient.SetAttribute("MaxPackets", UintegerValue(1));
            echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
            echoClient.SetAttribute("PacketSize", UintegerValue(1024));

            ApplicationContainer clientApps = echoClient.Install(nodes.Get(i));
            clientApps.Start(Seconds(2.0));
            clientApps.Stop(Seconds(10.0));
        }
    }
}

  AnimationInterface anim("Lab5_2.xml");

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

