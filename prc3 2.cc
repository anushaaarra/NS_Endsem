#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"
#include "ns3/names.h" // Add Names module header

// wifi includes
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/qos-utils.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("WifiExample");

int main ()
{
    bool verbose = true;
    uint32_t nCsma = 2;


    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    nCsma = nCsma == 0 ? 1 : nCsma;

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (5);
  NodeContainer wifiApNode;
  wifiApNode.Create(1);
  
  NodeContainer p2pNodes;
  p2pNodes.Create(1);
  p2pNodes.Add(wifiApNode.Get(0));
  
  NodeContainer p2pNodes1;
  p2pNodes1.Create(2);
  p2pNodes1.Add(p2pNodes.Get(0));
  
  NodeContainer csmaNodes;
  csmaNodes.Add(p2pNodes1.Get(1)); // point to point ke dusre node ko csma me add kiya
  csmaNodes.Create(nCsma);
  
  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install(p2pNodes);
	
  PointToPointHelper pointToPoint1;
  pointToPoint1.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
  pointToPoint1.SetChannelAttribute("Delay", StringValue("2ms"));
  pointToPoint1.SetQueue ("ns3::DropTailQueue","MaxSize", StringValue ("50p"));
  NetDeviceContainer p2pDevices1, p2pDevices2;
  p2pDevices1 = pointToPoint1.Install(p2pNodes.Get(0),p2pNodes1.Get(0));
  p2pDevices2 = pointToPoint1.Install(p2pNodes1.Get(0),p2pNodes1.Get(1));
  
   CsmaHelper csma;
   csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
   csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);
   
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetErrorRateModel ("ns3::NistErrorRateModel");
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);
  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (50.0),
                                 "MinY", DoubleValue (50.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel","Bounds", RectangleValue (Rectangle (40, 60, 40, 60)));
  mobility.Install (wifiStaNodes);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack1;
  stack1.Install (wifiApNode);
  stack1.Install (wifiStaNodes);
  stack1.Install(p2pNodes.Get(0));
  stack1.Install(p2pNodes1.Get(0));
  stack1.Install(p2pNodes1.Get(1));
  stack1.Install(csmaNodes);

  
  
  Ipv4AddressHelper address1;
  address1.SetBase ("10.1.3.0", "255.255.255.0");
  address1.Assign (apDevices);
  Ipv4InterfaceContainer staInterfaces = address1.Assign (staDevices);
 
  Ipv4AddressHelper address2;
  address2.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address2.Assign(p2pDevices);
 
  Ipv4AddressHelper address3;
  address3.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces1;
  p2pInterfaces1 = address3.Assign(p2pDevices1);
 
  Ipv4AddressHelper address4;
  address4.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer p2pInterfaces2;
  p2pInterfaces2 = address4.Assign(p2pDevices2);
  
  Ipv4AddressHelper address5;
  address5.SetBase ("10.1.4.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address5.Assign(csmaDevices);
	
  UdpEchoServerHelper echoServer(9);
  ApplicationContainer serverApps = echoServer.Install(wifiApNode.Get(0));
  serverApps.Start(Seconds(1.0));
  serverApps.Stop(Seconds(10.0));

  UdpEchoClientHelper echoClient(p2pInterfaces.GetAddress(1), 9);
  echoClient.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient.SetAttribute("PacketSize", UintegerValue(1024));
  ApplicationContainer clientApps = echoClient.Install(p2pNodes.Get(0));
  clientApps.Start(Seconds(2.0));
  clientApps.Stop(Seconds(10.0));
    
  UdpEchoServerHelper echoServer1(9);
  ApplicationContainer serverApps1 = echoServer1.Install(p2pNodes1.Get(1));
  serverApps1.Start(Seconds(1.0));
  serverApps1.Stop(Seconds(10.0));

  UdpEchoClientHelper echoClient1(p2pInterfaces2.GetAddress(1), 9);
  echoClient1.SetAttribute("MaxPackets", UintegerValue(1));
  echoClient1.SetAttribute("Interval", TimeValue(Seconds(1.0)));
  echoClient1.SetAttribute("PacketSize", UintegerValue(1024));
  ApplicationContainer clientApps1 = echoClient1.Install(p2pNodes1.Get(0));
  clientApps1.Start(Seconds(2.0));
  clientApps1.Stop(Seconds(10.0));
  
   UdpEchoServerHelper echoServer2(9);

    ApplicationContainer serverApps2 = echoServer2.Install(csmaNodes.Get(nCsma));
    serverApps2.Start(Seconds(1.0));
    serverApps2.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient2(csmaInterfaces.GetAddress(nCsma), 9);
    echoClient2.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient2.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient2.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps2 = echoClient2.Install(p2pNodes1.Get(1));
    clientApps2.Start(Seconds(2.0));
    clientApps2.Stop(Seconds(10.0));
    
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Animation setup
  AnimationInterface anim("prc3.xml");
  anim.SetConstantPosition(p2pNodes.Get(0), 50.0, 80.0);
  anim.SetConstantPosition(wifiApNode.Get(0), 40.0, 90.0);
  anim.SetConstantPosition(p2pNodes1.Get(0), 60.0, 60.0);
  anim.SetConstantPosition(p2pNodes1.Get(1), 70.0, 80.0);
  anim.SetConstantPosition(csmaNodes.Get(0), 73.0, 80.0);
  anim.SetConstantPosition(csmaNodes.Get(1), 76.0, 80.0);
  
 
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll(ascii.CreateFileStream("prc3.tr"));
  pointToPoint1.EnableAsciiAll(ascii.CreateFileStream("prc3.tr"));
  csma.EnableAsciiAll(ascii.CreateFileStream("prc3.tr"));

  // Run simulation
  Simulator::Run ();
  Simulator::Destroy ();
  
  return 0;
}

