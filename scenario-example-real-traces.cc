/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 This software was developed at the National Institute of Standards and
 Technology by employees of the Federal Government in the course of
 their official duties. Pursuant to titleElement 17 Section 105 of the United
 States Code this software is not subject to copyright protection and
 is in the public domain.
 NIST assumes no responsibility whatsoever for its use by other parties,
 and makes no guarantees, expressed or implied, about its quality,
 reliability, or any other characteristic.

 We would appreciate acknowledgement if the software is used.

 NIST ALLOWS FREE USE OF THIS SOFTWARE IN ITS "AS IS" CONDITION AND
 DISCLAIM ANY LIABILITY OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING
 FROM THE USE OF THIS SOFTWARE.
 */

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/point-to-point-epc-helper.h"
#include "ns3/config-store.h"
#include <ns3/buildings-helper.h>
#include <ns3/constant-position-mobility-model.h>
#include "ns3/random-variable-stream.h"
#include <ns3/lte-load-profile-management.h>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ScenarioExampleRealTraces");

int
main (int argc, char *argv[])
{

  // Set the run number of simulation and the value of the seed
  //RngSeedManager::SetSeed (1);
  //RngSeedManager::SetRun (1);  

  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", StringValue ("50"));
  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", StringValue ("50"));

  // Set the UEs power in dBm 
  Config::SetDefault ("ns3::LteUePhy::TxPower", DoubleValue (23.0));
  Config::SetDefault ("ns3::LteUePowerControl::Pcmax", DoubleValue (24.0));
  // Set the eNBs power in dBm 
  Config::SetDefault ("ns3::LteEnbPhy::TxPower", DoubleValue (46.0));
  // Allowed values : 2, 5, 10, 20, 40, 80, 160 and 320
  Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (160));

  // The data rate to be used for the S1-U link to be created
  Config::SetDefault ("ns3::PointToPointEpcHelper::S1uLinkDataRate", DataRateValue(DataRate ("100Gb/s")));//default is 10Gb/s
  
  // Time interval to display EPC radio bearer stats
  Config::SetDefault ("ns3::RadioBearerStatsCalculator::EpochDuration", TimeValue (MilliSeconds (5)));
  
  // Max Packets dropped in the queue
  Config::SetDefault ("ns3::DropTailQueue::MaxPackets", UintegerValue (10000000));

  // Initialize some values
  double simTime = 15;
  double startTime = 2;
  double intPack = 1;
  uint32_t maxPack = 1000000;
  //uint32_t sizePack = 1280;
  uint32_t nEnbs = 3;
  std::vector<uint32_t> maxUesPerEnb;
  ///*
  Ptr<UniformRandomVariable> ueRnd = CreateObject<UniformRandomVariable> ();
  ueRnd->SetAttribute ("Min", DoubleValue(5));
  ueRnd->SetAttribute ("Max", DoubleValue(10));
  for (uint32_t j=0; j < nEnbs; j++)
  {
    maxUesPerEnb.push_back (ueRnd->GetInteger());
  }
  //*/
    
  /*
  maxUesPerEnb.push_back (50); //1
  maxUesPerEnb.push_back (55); //2
  maxUesPerEnb.push_back (60); //3
  maxUesPerEnb.push_back (65); //4
  maxUesPerEnb.push_back (70); //5
  maxUesPerEnb.push_back (75); //6
  maxUesPerEnb.push_back (80); //7
  maxUesPerEnb.push_back (85); //8
  maxUesPerEnb.push_back (90); //9
  maxUesPerEnb.push_back (95); //10
  maxUesPerEnb.push_back (100); //11
  maxUesPerEnb.push_back (105); //12
  maxUesPerEnb.push_back (110); //13
  */
  uint32_t nUes = 0;
  for (std::vector<uint32_t>::iterator it = maxUesPerEnb.begin() ; it != maxUesPerEnb.end(); ++it)
     nUes+=*it;
  uint32_t antennaHeight = 30.0;


  // Command line arguments
  CommandLine cmd;
  cmd.AddValue ("time", "Simulation time", simTime);
  cmd.AddValue ("packetStart", "Application start time", startTime);
  //cmd.AddValue ("packetSize", "Packets size in bytes", sizePack);
  cmd.AddValue ("packetMax", "maximum number of packets per UE", maxPack); 
  cmd.AddValue ("UEs", "number of UEs in the simulation", nUes);
  cmd.AddValue ("eNBs", "number of eNBs in the simulation", nEnbs);
  cmd.Parse(argc, argv);

  // Create the helpers
  Ptr<LteHelper> lteHelper = CreateObject< LteHelper> ();

  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject< PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
 
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults();

  // Set pathloss model
  //lteHelper->SetPathlossModelType ("ns3::FriisPropagationLossModel"); // free space
  lteHelper->SetPathlossModelType ("ns3::Cost231PropagationLossModel"); // default pathloss for LTE in NS-3
  
  // Create nodes (eNbs + UEs)
  NodeContainer enbNodes;
  enbNodes.Create (nEnbs);
  NodeContainer ueNodes;
  ueNodes.Create (nUes);

  // Install mobility : constant position
  MobilityHelper mobilityeNodeB;
  ///*
  mobilityeNodeB.SetPositionAllocator("ns3::GridPositionAllocator", // in meters
                                      "MinX", DoubleValue (0.0), 
                                      "MinY", DoubleValue (0.0),
                                      "DeltaX", DoubleValue (200.0),
                                      "DeltaY", DoubleValue (200.0),
                                      "GridWidth", UintegerValue (2),
                                      "LayoutType", StringValue ("RowFirst"));
  //*/
  /*
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (1440, 653, 30)); // enb1
  positionAlloc->Add (Vector (787, 766, 30)); // enb2
  positionAlloc->Add (Vector (979, 958, 30)); // enb3
  positionAlloc->Add (Vector (725, 1169, 30)); // enb4
  positionAlloc->Add (Vector (1455, 995, 30)); // enb5
  positionAlloc->Add (Vector (1206, 1258, 30)); // enb6
  positionAlloc->Add (Vector (1638, 1439, 30)); // enb7
  positionAlloc->Add (Vector (1409, 1530, 30)); // enb8
  positionAlloc->Add (Vector (891, 1578, 30)); // enb9
  positionAlloc->Add (Vector (1723, 1767, 30)); // enb10
  positionAlloc->Add (Vector (1151, 1879, 30)); // enb11
  positionAlloc->Add (Vector (1802, 1843, 30)); // enb12
  positionAlloc->Add (Vector (1450, 2066, 30)); // enb13
  mobilityeNodeB.SetPositionAllocator (positionAlloc);
  */

  mobilityeNodeB.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobilityeNodeB.Install (enbNodes);

  Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < nEnbs; i++)
  {
    Ptr<MobilityModel> enbMobility = enbNodes.Get (i)->GetObject<MobilityModel> ();
    double enbX = enbMobility->GetPosition ().x;
    double enbY = enbMobility->GetPosition ().y;
    enbMobility->SetPosition (Vector (enbX, enbY, antennaHeight));
    Ptr<RandomDiscPositionAllocator> discPos = CreateObject<RandomDiscPositionAllocator> ();
    discPos->SetX (enbX);
    discPos->SetY (enbY);
    Ptr<UniformRandomVariable> urv = CreateObject<UniformRandomVariable> ();
    urv->SetAttribute ("Min", DoubleValue (10.0));
    urv->SetAttribute ("Max", DoubleValue (400.0));
    discPos->SetRho (urv);

    for (uint16_t j = 0; j < maxUesPerEnb.at(i); j++)
    {
      Vector uePos = discPos->GetNext();
      uePositionAlloc->Add (Vector (uePos.x, uePos.y, 1.5));
    }
  }
  
  MobilityHelper mobilityUe;
  mobilityUe.SetPositionAllocator(uePositionAlloc);
  mobilityUe.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobilityUe.Install(ueNodes);
    
  //Install LTE devices to the eNBs
  NetDeviceContainer enbDevs;
  enbDevs = lteHelper->InstallEnbDevice (enbNodes);

  // Create UE devices and install them to UE Nodes
  NetDeviceContainer ueDevs;
  ueDevs = lteHelper->InstallUeDevice (ueNodes);

  // Add X2 inteface
  lteHelper->AddX2Interface (enbNodes);

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));//1500
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueDevs));
 
  // Assign IP address to UEs, and install applications
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
  {
    Ptr<Node> ueNode = ueNodes.Get (u);
    // Set the default gateway for the UE
    Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
    ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
  }

  // Attach each UE to the best available eNB
  lteHelper->Attach (ueDevs);

  /* Load Management */
  Ptr<LteLoadProfileManagement> profileManagement = CreateObject< LteLoadProfileManagement> ();
  profileManagement->SetLteHelper (lteHelper);
  std::vector<uint32_t> loadProfile;
  Ptr<UniformRandomVariable> lpRnd = CreateObject<UniformRandomVariable> ();
  lpRnd->SetAttribute ("Min", DoubleValue(1));
  lpRnd->SetAttribute ("Max", DoubleValue(3));
  uint32_t prev = 0;
  for (uint32_t j=0; j < nEnbs; j++)
  {
    uint8_t pf = 2;
    loadProfile.push_back (pf);
    prev = prev + maxUesPerEnb.at(j);
  }
  
  profileManagement->SetDevices (enbDevs, ueDevs, maxUesPerEnb, loadProfile);

  /*
  // Attach each UE to the corresponding eNB
  for (uint32_t u = 0; u < ueDevs.GetN (); ++u)
  {
	  lteHelper->Attach (ueDevs.Get (u), enbDevs.Get (u));
  }
  */
  // Attach UEs automatically
  //lteHelper->Attach (ueDevs);
  
  ///*** Configure applications ***///
  
  // randomize a bit start times to avoid simulation artifacts
  // (e.g., buffer overflows due to packet transmissions happening
  // exactly at the same time) 
  Ptr<UniformRandomVariable> startTimeSeconds = CreateObject<UniformRandomVariable> ();
  startTimeSeconds->SetAttribute ("Min", DoubleValue (0));
  startTimeSeconds->SetAttribute ("Max", DoubleValue (intPack/1000.0));


  uint16_t dlPort = 1234;
  uint16_t ulPort = 2000;
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      ++ulPort;
      ApplicationContainer clientApps;
      ApplicationContainer serverApps;


      PacketSinkHelper dlPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), dlPort));
      PacketSinkHelper ulPacketSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), ulPort));
      serverApps.Add (dlPacketSinkHelper.Install (ueNodes.Get(u)));
      serverApps.Add (ulPacketSinkHelper.Install (remoteHost));

      UdpClientHelper dlClient (ueIpIface.GetAddress (u), dlPort);
      dlClient.SetAttribute ("Interval", TimeValue (MilliSeconds(intPack)));
      dlClient.SetAttribute ("MaxPackets", UintegerValue(maxPack));

      UdpClientHelper ulClient (remoteHostAddr, ulPort);
      ulClient.SetAttribute ("Interval", TimeValue (MilliSeconds(intPack)));
      ulClient.SetAttribute ("MaxPackets", UintegerValue(maxPack));

      clientApps.Add (dlClient.Install (remoteHost));
      clientApps.Add (ulClient.Install (ueNodes.Get(u)));

      serverApps.Start (Seconds (startTime));
      clientApps.Start (Seconds (startTime));  
    }


  // Enable traces
  //lteHelper->EnableTraces();

  Simulator::Stop(Seconds(simTime));
  
  Simulator::Run();
  Simulator::Destroy();
  return 0;

}

