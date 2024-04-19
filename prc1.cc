/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/netanim-module.h"

// wifi includes
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/qos-utils.h"


// Default Network Topology
//
//       10.1.1.0
// n0 -------------- n1   n2   n3   n4
//    point-to-point  |    |    |    |
//                    ================
//                      LAN 10.1.2.0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SecondScriptExample");

int
main(int argc, char* argv[])
{
    bool verbose = true;
    uint32_t nCsma = 2;


    // Passing user defined data
    CommandLine cmd(__FILE__);
    cmd.AddValue("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
    cmd.AddValue("verbose", "Tell echo applications to log if true", verbose);
    
    cmd.Parse(argc, argv);

    std::cout<<nCsma<<"CSMA value\n";


    if (verbose)
    {
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    nCsma = nCsma == 0 ? 1 : nCsma;

    

    NodeContainer csmaNodes;
    csmaNodes.Create(nCsma);

    NodeContainer p2pNodes;
    p2pNodes.Create(2);
    csmaNodes.Add(p2pNodes.Get(1));

    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("5Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("2ms"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint.Install(p2pNodes);

    CsmaHelper csma;
    csma.SetChannelAttribute("DataRate", StringValue("100Mbps"));
    csma.SetChannelAttribute("Delay", TimeValue(NanoSeconds(6560)));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma.Install(csmaNodes);

    InternetStackHelper stack;
    stack.Install(p2pNodes.Get(0));
    stack.Install(csmaNodes);

    Ipv4AddressHelper address;
    address.SetBase("10.1.1.0", "255.255.255.0");
    Ipv4InterfaceContainer p2pInterfaces;
    p2pInterfaces = address.Assign(p2pDevices);

    address.SetBase("10.1.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces;
    csmaInterfaces = address.Assign(csmaDevices);

    UdpEchoServerHelper echoServer(9);

    ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(nCsma));
    serverApps.Start(Seconds(1.0));
    serverApps.Stop(Seconds(10.0));

    UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(nCsma-1), 9);
    echoClient.SetAttribute("MaxPackets", UintegerValue(1));
    echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));
    echoClient.SetAttribute("PacketSize", UintegerValue(1024));

    ApplicationContainer clientApps = echoClient.Install(p2pNodes.Get(0));
    clientApps.Start(Seconds(2.0));
    clientApps.Stop(Seconds(10.0));



    // WIFI Codes

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (4);
  NodeContainer wifiApNode;
  wifiApNode.Create(1);



  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
    //channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
    //channel.AddPropagationLoss ("ns3::FriisPropagationLossModel", "Frequency", DoubleValue(5e9));
  YansWifiPhyHelper phy;
  phy.SetErrorRateModel ("ns3::NistErrorRateModel");
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::MinstrelHtWifiManager");

  WifiMacHelper mac;
  //mac.SetType ("ns3::QosWifiMacHelper", "QosSupported", BooleanValue (true));
  //NqosWifiMacHelper mac= NqosWifiMacHelper::Default();
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac","Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));
  //mac.SetType ("ns3::NqstaWifiMac","Ssid", SsidValue (ssid),"ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  //mac.SetType ("ns3::NqapWifiMac", "Ssid", SsidValue (ssid));
  mac.SetType ("ns3::ApWifiMac", "Ssid", SsidValue (ssid));

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
  //mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);


// Set up the nodes in a star topology
// MobilityHelper mobility;

// Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
// for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i)
// {
//   // Arrange the nodes in a star topology around the point (0,0)
//   double angle = (double)i/wifiStaNodes.GetN() * 2 * M_PI;
//   positionAlloc->Add (Vector (10.0 * cos(angle), 10.0 * sin(angle), 0.0));
// }

// mobility.SetPositionAllocator (positionAlloc);

// // Set the mobility model to RandomWalk2dMobilityModel with bounds encompassing the star
// mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel", "Bounds", RectangleValue (Rectangle (-10, 10, -10, 10)));
// mobility.Install (wifiStaNodes);

// // Position the AP node at the center of the star
// positionAlloc = CreateObject<ListPositionAllocator> ();
// positionAlloc->Add (Vector (0.0, 0.0, 0.0));
// mobility.SetPositionAllocator (positionAlloc);
// mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
// mobility.Install (wifiApNode);





  InternetStackHelper stack1;
  //stack1.Install (csmaNodes);
  stack1.Install (wifiApNode);
  stack1.Install (wifiStaNodes);

  Ipv4AddressHelper address1;


  address1.SetBase ("10.1.3.0", "255.255.255.0");
  address1.Assign (apDevices);
  Ipv4InterfaceContainer staInterfaces = address1.Assign (staDevices);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    AnimationInterface anim("mycode.xml");
    anim.SetConstantPosition(p2pNodes.Get(0), 50.0, 80.0);
    anim.SetConstantPosition(p2pNodes.Get(1), 40.0, 90.0);
    anim.SetConstantPosition(csmaNodes.Get(0), 10.0, 50.0);
    anim.SetConstantPosition(csmaNodes.Get(1), 20.0, 50.0);

    AsciiTraceHelper ascii;
    pointToPoint.EnableAsciiAll(ascii.CreateFileStream("mycode.tr"));
    csma.EnableAsciiAll(ascii.CreateFileStream("mycode.tr"));

    Simulator::Run();
    Simulator::Destroy();
    return 0;
}
