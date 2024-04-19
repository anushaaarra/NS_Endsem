/*

 * This program is free software; you can redistribute it and/or modify

 * it under the terms of the GNU General Public License version 2 as

 * published by the Free Software Foundation;

 *

 * This program is distributed in the hope that it will be useful,

 * but WITHOUT ANY WARRANTY; without even the implied warranty of

 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the

 * GNU General Public License for more details.

 *

 * You should have received a copy of the GNU General Public License

 * along with this program; if not, write to the Free Software

 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */



#include <iostream>

#include <fstream>

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



using namespace ns3;



NS_LOG_COMPONENT_DEFINE("SecondScriptExample");



int main(int argc, char* argv[]) {

	bool verbose = true;

	uint32_t nCsma = 3;







	std::cout << nCsma << " CSMA value\n";



	if (verbose) {

    	LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);

    	LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

	}



	nCsma = nCsma == 0 ? 1 : nCsma;



	NodeContainer csmaNodes;

	csmaNodes.Create(3);



	NodeContainer p2pNodes;

	p2pNodes.Add(csmaNodes.Get(2));

	p2pNodes.Create(1); // ERROR, made change



	CsmaHelper csma;

	csma.SetChannelAttribute("DataRate", StringValue("200Mbps"));

	csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(12)));

	NetDeviceContainer csmaDevices;

	csmaDevices = csma.Install(csmaNodes);



	PointToPointHelper pointToPoint;

	pointToPoint.SetDeviceAttribute("DataRate", StringValue("8Mbps"));

	pointToPoint.SetChannelAttribute("Delay", StringValue("5ms"));

	NetDeviceContainer p2pDevices;

	p2pDevices = pointToPoint.Install(p2pNodes.Get(0), p2pNodes.Get(1));



	NodeContainer wifiStaNodes, wifiAPNodes;

	wifiStaNodes.Create(5);

	wifiAPNodes.Add(p2pNodes.Get(1));



	NodeContainer p2pNodes1;

	p2pNodes1.Create(2);

 	NetDeviceContainer p2pDevices1, csmaDevices1;

	p2pDevices1= pointToPoint.Install(p2pNodes1.Get(0), p2pNodes1.Get(1));



	wifiStaNodes.Add(p2pNodes1.Get(0));



	NodeContainer csmaNodes1;

	csmaNodes1.Add(p2pNodes1.Get(1));

	csmaNodes1.Create(2);





	csmaDevices1 = csma.Install(csmaNodes1);



	InternetStackHelper stack;

	// Install Internet stack on the CSMA nodes

	stack.Install(csmaNodes);

	// Install Internet stack on the point-to-point nodes

	stack.Install(wifiAPNodes);

	stack.Install(wifiStaNodes);

	stack.Install(csmaNodes1);



	//wifi setting up

 	WifiMacHelper wifiMac;

 	WifiHelper wifiHelper;

 	wifiHelper.SetStandard(WIFI_STANDARD_80211n_5GHZ);



 	/* Set up Legacy Channel */

    YansWifiChannelHelper wifiChannel;

    wifiChannel.SetPropagationDelay("ns3::ConstantSpeedPropagationDelayModel");

    wifiChannel.AddPropagationLoss("ns3::FriisPropagationLossModel", "Frequency",    		 DoubleValue(5e9));



  /* Setup Physical Layer */

  // YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();

  YansWifiPhyHelper wifiPhy;

  wifiPhy.SetChannel(wifiChannel.Create());

  wifiPhy.SetErrorRateModel("ns3::YansErrorRateModel");

  wifiHelper.SetRemoteStationManager("ns3::ConstantRateWifiManager",

                                 	"DataMode", StringValue("HtMcs7"),

                                 	"ControlMode", StringValue("HtMcs0"));



   Ssid ssid = Ssid("network");

   //Configure AP

	wifiMac.SetType("ns3::ApWifiMac",

              	"Ssid", SsidValue(ssid));



      NetDeviceContainer apDevice = wifiHelper.Install(wifiPhy, wifiMac, wifiAPNodes);



   /* Configure STA */



  wifiMac.SetType("ns3::StaWifiMac",

              	"Ssid", SsidValue(ssid));



  NetDeviceContainer staDevices, devices;

  staDevices = wifiHelper.Install(wifiPhy, wifiMac, wifiStaNodes);



  //mobility

	MobilityHelper mobility;

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();



  positionAlloc->Add(Vector(70.0, 70.0, 0.0));

  positionAlloc->Add(Vector(110.0, 20.0, 0.0));

  positionAlloc->Add(Vector(110, 70, 0.0));

  positionAlloc->Add(Vector(110, 120, 0.0));

  positionAlloc->Add(Vector(160, 70, 0.0));

  positionAlloc->Add(Vector(190, 80, 0.0));



  mobility.SetPositionAllocator(positionAlloc);

  mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");

  mobility.Install(wifiStaNodes);

  Ptr<ListPositionAllocator> positionAlloc2 = CreateObject<ListPositionAllocator>();

  positionAlloc2->Add(Vector(50, 80, 0.0));



  mobility.SetPositionAllocator(positionAlloc2);

  mobility.Install(wifiAPNodes);



	Ipv4AddressHelper address;

	address.SetBase("10.1.1.0", "255.255.255.0");

	Ipv4InterfaceContainer csmaInterfaces, csmaInterfaces1;

	csmaInterfaces = address.Assign(csmaDevices);



	address.SetBase("10.1.2.0", "255.255.255.0");

	Ipv4InterfaceContainer p2pInterfaces,p2pInterfaces1;

	p2pInterfaces = address.Assign(p2pDevices);



	address.SetBase("10.1.3.0", "255.255.255.0");

  Ipv4InterfaceContainer apInterface, staInterface;

  apInterface = address.Assign(apDevice);

  staInterface = address.Assign(staDevices);



	address.SetBase("10.1.4.0", "255.255.255.0");

	p2pInterfaces1 = address.Assign(p2pDevices1);



	address.SetBase("10.1.5.0", "255.255.255.0");

	csmaInterfaces1 = address.Assign(csmaDevices1);











	UdpEchoServerHelper echoServer(9);

	ApplicationContainer serverApps = echoServer.Install(csmaNodes.Get(0));

	serverApps.Start(Seconds(1.0));

	serverApps.Stop(Seconds(10.0));



	UdpEchoClientHelper echoClient(csmaInterfaces.GetAddress(0), 9);

	echoClient.SetAttribute("MaxPackets", UintegerValue(1));

	echoClient.SetAttribute("Interval", TimeValue(Seconds(1.0)));

	echoClient.SetAttribute("PacketSize", UintegerValue(1024));

	ApplicationContainer clientApps = echoClient.Install(csmaNodes1.Get(0));

	clientApps.Start(Seconds(2.0));

	clientApps.Stop(Seconds(10.0));



	// InternetStackHelper stack1; WTF WHY IS THIS THERE???



	// stack1.Install(csmaNodes);



	Ipv4GlobalRoutingHelper::PopulateRoutingTables();



	AnimationInterface anim("mycode2.xml");

 	anim.SetConstantPosition(p2pNodes.Get(0), 30.0, 120.0);

	anim.SetConstantPosition(p2pNodes.Get(1), 50.0, 80.0);

	anim.SetConstantPosition(csmaNodes.Get(0), 30.0, 20.0);

	anim.SetConstantPosition(csmaNodes.Get(1), 40.0, 20.0);

	anim.SetConstantPosition(wifiStaNodes.Get(2), 110.0, 70.0);

	anim.SetConstantPosition(wifiStaNodes.Get(0), 70.0, 70.0);

	anim.SetConstantPosition(wifiStaNodes.Get(1), 110.0, 20.0);

	anim.SetConstantPosition(wifiStaNodes.Get(3), 110.0, 120.0);

	anim.SetConstantPosition(wifiStaNodes.Get(4), 160.0, 70.0);

	anim.SetConstantPosition(p2pNodes1.Get(1), 200.0, 90.0);

	anim.SetConstantPosition(csmaNodes1.Get(1), 195.0, 30.0);

	anim.SetConstantPosition(csmaNodes1.Get(2), 205.0, 30.0);



	AsciiTraceHelper ascii;

	pointToPoint.EnableAsciiAll(ascii.CreateFileStream("endsem1.tr"));

	csma.EnableAsciiAll(ascii.CreateFileStream("endsem1.tr"));



	Simulator::Run();

	Simulator::Destroy();



	return 0;

}
