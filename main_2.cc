#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/netanim-module.h"
#include <bits/stdc++.h>
#include "ns3/wave-net-device.h"
#include "ns3/wave-helper.h"
#include "ns3/pointer.h"
#include "sp-application.h"
#include "iot-application.h"
#include "ttp-application.h"
#include "ns3/tcp-header.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "bad-iot-application.h"

using namespace std;
using namespace ns3;

// NS_LOG_COMPONENT_DEFINE ("Test1");


// void init(Ptr<IoTApplication> iot){
//   iot->RequestForTrust();
// }

int
main (int argc, char *argv[])
{

  // LogComponentEnable("IoTApplication",LOG_LEVEL_ALL);
  // LogComponentEnable("SPApplication",LOG_LEVEL_ALL);
  // LogComponentEnable("TTPApplication",LOG_LEVEL_ALL);
 
  /******* Local */
 
  int nGoodNode = 14;
  int nBadNode = 10;
  int nGoodSP = 0;
  int nBadSP = 0;
  int nGSP = 0;
  int nBSP = 5;
  // int128_t total_nodes = 0;
  uint32_t simTime = 150;
  int nTTP = 1;
  // double radius = 100.0;


  /****** UPLOAD *****/

  // int nGoodNode = 105;

  // // 0->91 these 92 iot device are good
  // // 92->99 these 8 are good sp

  // int nBadNode = 45;

  // // 100-137 these 38 iot device are bad
  // // 138-139 these 2 are bad sp
  // int nGoodSP = 0;
  // int nBadSP = 0;
  // // int128_t total_nodes = 0;
  // int nGSP = 5;
  // int nBSP = 2;
  // uint32_t simTime = 5000;
  // int nTTP = 1;

  CommandLine cmd (__FILE__);
  cmd.AddValue ("nGoodNode", "Number of \"Good\" IoT nodes/devices", nGoodNode);
  cmd.AddValue ("nBadNode", "Number of \"Bad\" IoT nodes/devices", nBadNode);
  cmd.AddValue ("nGoodServiceProvider", "Number of \"Good\" Service provider nodes/devices",
                nGoodSP);
  cmd.AddValue ("nBadServiceProvider", "Number of \"Bad\" Service provider nodes/devices", nBadSP);

  cmd.Parse (argc, argv);

  // total_nodes = nGoodNode + nBadNode + nGoodSP + nBadSP;
  ns3::PacketMetadata::Enable ();
  uint16_t sinkPort = 8080;

    // The one and only ap of wifi
  NodeContainer wifiApNode;
  wifiApNode.Create (1);


  NodeContainer ttpContainer;
  ttpContainer.Create (nTTP);

  NodeContainer goodNodeContainer;
  goodNodeContainer.Create (nGoodNode);
   //   node container for the bad nodes
  NodeContainer badNodeContainer;
  badNodeContainer.Create (nBadNode);

  NodeContainer goodSPContainer;
  goodSPContainer.Create (nGoodSP);
  //   node container for the good nodes
  NodeContainer badSPContainer;
  badSPContainer.Create (nBadSP);

  NodeContainer allNodes;
  allNodes.Add(goodNodeContainer);
  allNodes.Add(badNodeContainer);
  allNodes.Add(goodSPContainer);
  allNodes.Add(badSPContainer);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy;
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
  wifi.SetStandard(WIFI_STANDARD_80211a);
  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  

  NetDeviceContainer ttpDevice = wifi.Install(phy, mac, ttpContainer);
  NetDeviceContainer goodNodeDevice = wifi.Install(phy, mac, goodNodeContainer);
  NetDeviceContainer badNodeDevice = wifi.Install(phy, mac, badNodeContainer);
  NetDeviceContainer goodSPDevice = wifi.Install(phy, mac, goodSPContainer);
  NetDeviceContainer badSPDevice = wifi.Install(phy, mac, badSPContainer);


  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);
  

  MobilityHelper mobility;

  mobility.SetPositionAllocator (
      "ns3::GridPositionAllocator", "MinX", 
      DoubleValue (0.0), "MinY", 
      DoubleValue (0.0), "DeltaX",
      DoubleValue (1.0), "DeltaY", 
      DoubleValue (1.0), "GridWidth",
      UintegerValue (25), 
      "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (allNodes);
  mobility.Install (ttpContainer);
  mobility.Install(wifiApNode);


  // setting the internet protocol stack
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (ttpContainer);
  stack.Install (goodNodeContainer);
  stack.Install (badNodeContainer);
  stack.Install (goodSPContainer);
  stack.Install (badSPContainer);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface = address.Assign (apDevices);
  Ipv4InterfaceContainer ttpInterface = address.Assign (ttpDevice);
  Ipv4InterfaceContainer goodDeviceInterface =  address.Assign (goodNodeDevice);
  Ipv4InterfaceContainer badDeviceInterface = address.Assign (badNodeDevice);
  Ipv4InterfaceContainer goodSPInterface = address.Assign (goodSPDevice);
  Ipv4InterfaceContainer badSPInterface = address.Assign (badSPDevice);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();




  Ptr<Socket> ttp_listener[nTTP];
  Ptr<Socket> ttp_speaker[nTTP];

  Ptr<Socket> good_iot_listener[nGoodNode];
  Ptr<Socket> good_iot_speaker[nGoodNode];

  Ptr<Socket> bad_iot_listener[nBadNode];
  Ptr<Socket> bad_iot_speaker[nBadNode];

  Ptr<Socket> good_sp_listener[nGoodSP];
  Ptr<Socket> good_sp_speaker[nGoodSP];

  Ptr<Socket> bad_sp_listener[nBadSP];
  Ptr<Socket> bad_sp_speaker[nBadSP];

  for(int j=0;j<nTTP;j++)
  {
    ttp_listener[j] = Socket::CreateSocket ( ttpContainer.Get(j),TcpSocketFactory::GetTypeId ());
    ttp_speaker[j]  = Socket::CreateSocket ( ttpContainer.Get(j),TcpSocketFactory::GetTypeId ());
  }

  for(int j=0;j<nGoodNode;j++)
  {
    good_iot_listener[j] = Socket::CreateSocket ( goodNodeContainer.Get(j),TcpSocketFactory::GetTypeId ());
    good_iot_speaker[j]  = Socket::CreateSocket ( goodNodeContainer.Get(j),TcpSocketFactory::GetTypeId ());
  }

  for(int j=0;j<nBadNode;j++)
  {
    bad_iot_listener[j] = Socket::CreateSocket ( badNodeContainer.Get(j),TcpSocketFactory::GetTypeId ());
    bad_iot_speaker[j]  = Socket::CreateSocket ( badNodeContainer.Get(j),TcpSocketFactory::GetTypeId ());
  }


  for(int j=0;j<nGoodSP;j++)
  {
    good_sp_listener[j] = Socket::CreateSocket ( goodSPContainer.Get(j),TcpSocketFactory::GetTypeId ());
    good_sp_speaker[j]  = Socket::CreateSocket ( goodSPContainer.Get(j),TcpSocketFactory::GetTypeId ());
  }

  for(int j=0;j<nBadSP;j++)
  {
    bad_sp_listener[j] = Socket::CreateSocket ( badSPContainer.Get(j),TcpSocketFactory::GetTypeId ());
    bad_sp_speaker[j]  = Socket::CreateSocket ( badSPContainer.Get(j),TcpSocketFactory::GetTypeId ());
  }


  Address goodIoTAddr[nGoodNode];
  Address badIoTAddr[nBadNode];
  Address ttpAddr[nTTP];
  Address goodSPAddr[nGoodSP];
  Address badSPAddr[nBadSP];
  Address wifiAPAddr;
  AsciiTraceHelper as;

  wifiAPAddr = InetSocketAddress(apInterface.GetAddress(0), sinkPort);

  for(int j = 0; j < nGoodNode; j++){
    goodIoTAddr[j] = InetSocketAddress(goodDeviceInterface.GetAddress(j), sinkPort);
  }

  for(int j = 0; j < nBadNode; j++){
    badIoTAddr[j] = InetSocketAddress(badDeviceInterface.GetAddress(j), sinkPort);
  }

  for(int j = 0; j < nGoodSP; j++){
    goodSPAddr[j] = InetSocketAddress(goodSPInterface.GetAddress(j), sinkPort);
  }

  for(int j = 0; j < nBadSP; j++){
    badSPAddr[j] = InetSocketAddress(badSPInterface.GetAddress(j), sinkPort);
  }

  for(int j = 0; j < nTTP; j++){
    ttpAddr[j] = InetSocketAddress(ttpInterface.GetAddress(j), sinkPort);
  }



  Ptr<IoTApplication> iotApps[nGoodNode];
  Ptr<BadIoTApplication> badIoT[nBadNode];
  Ptr<TTPApplication> ttpApps[nTTP];
  Ptr<SPApplication> spApps[nGoodSP];

  for (int i = 0; i < nTTP; i++)
  {
    ttpApps[i] = CreateObject<TTPApplication> ();
    ttpApps[i]->SetStartTime (Seconds (1.0));
    ttpApps[i]->SetStopTime (Seconds (simTime));
    ttpApps[i]->Setup (ttp_speaker[i],ttp_listener[i] , 1040, 1000, DataRate ("1Mbps"),sinkPort);


    for(int j = 0; j < nTTP; j++){
        ttpApps[i]->AddTTPAddr(ttpAddr[j]);
      }

    for(int j = 0; j < nGoodSP; j++){
        ttpApps[i]->AddSPAddr(goodSPAddr[j]);
      }
    for(int j = 0; j < nBadSP; j++){
        ttpApps[i]->AddSPAddr(badSPAddr[j]);
      }
    
    for(int j = 0; j < nGoodNode; j++){
        ttpApps[i]->AddIoTAddr(goodIoTAddr[j]);
      }
    for(int j = 0; j < nBadNode; j++){
        ttpApps[i]->AddIoTAddr(badIoTAddr[j]);
      }

    ttpApps[i]->SetSelfAddressIndex(i);
    ttpContainer.Get (i)-> AddApplication(ttpApps[i]);
  }

  // print all infos
  cout<<"TTP \n";
  for(int i = 0; i < nTTP; i++)
    { 
      cout<<InetSocketAddress::ConvertFrom(ttpAddr[i]).GetIpv4()<<endl;
    }
  
  cout<<"\n\n\nGoodNode:\n";
  for(int i = 0; i < nGoodNode; i++)
    {
      if(i == nGoodNode - nGSP)
      {
        cout<<"\n\nGoodSP:\n";
      }
      cout<<InetSocketAddress::ConvertFrom(goodIoTAddr[i]).GetIpv4()<<endl;
    }
  
  cout<<"\n\n\nBadNode:\n";
  for(int i = 0; i < nBadNode; i++)
    {
      if(i == nBadNode - nBSP)
      {
        cout<<"\n\nBadSP:\n";
      }
      cout<<InetSocketAddress::ConvertFrom(badIoTAddr[i]).GetIpv4()<<endl;
    }



    

  for (int i = 0; i < nGoodNode; i++)
    {
      iotApps[i] = CreateObject<IoTApplication> ();
      // Ptr<IoTApplication> app_i = CreateObject<IoTApplication> ();
      for(int j = 0; j < nTTP; j++){
        iotApps[i]->AddTTPAddr(ttpAddr[j]);
        iotApps[i]->AddTTPPos(ttpContainer.Get(j)->GetObject<MobilityModel>()->GetPosition());
      }

      for(int j = 0; j < nGoodSP; j++){
        iotApps[i]->AddSPAddr(goodSPAddr[j]);
      }
      // for(int j = 0; j < nBadSP; j++){
      //   iotApps[i]->AddSPAddr(badSPAddr[j]);
      // }

      for(int j = 0; j < nGoodNode; j++){
        iotApps[i]->AddIoTAddr(goodIoTAddr[j]);
        
      }
      for(int j = 0; j < nBadNode; j++){
        iotApps[i]->AddIoTAddr(badIoTAddr[j]);
      }

      for(int j = (nGoodNode-nGSP); j < nGoodNode; j++)
      {
          iotApps[i]->AddSPIndex(j);
      }
      for(int j = (nGoodNode + nBadNode - nBSP); j < nGoodNode + nBadNode; j++)
      {
          iotApps[i]->AddSPIndex(j);
      }

      iotApps[i] -> SetSelfAddrIndex((uint16_t)i);
      iotApps[i]->SetStartTime (Seconds (1.0));
      iotApps[i]->SetStopTime (Seconds (simTime));
      iotApps[i]->Setup(good_iot_speaker[i], good_iot_listener[i], 1040, 1000, DataRate("1Mbps"), sinkPort);
      iotApps[i]->SetPos (goodNodeContainer.Get (i) -> GetObject<MobilityModel>()->GetPosition());
      goodNodeContainer.Get (i)->AddApplication (iotApps[i]);
    }



    for (int i = 0; i < nBadNode; i++)
    {
      badIoT[i] = CreateObject<BadIoTApplication> ();
      // Ptr<IoTApplication> app_i = CreateObject<IoTApplication> ();
      for(int j = 0; j < nTTP; j++){
        badIoT[i]->AddTTPAddr(ttpAddr[j]);
        badIoT[i]->AddTTPPos(ttpContainer.Get(j)->GetObject<MobilityModel>()->GetPosition());
      }

      for(int j = 0; j < nGoodSP; j++){
        badIoT[i]->AddSPAddr(goodSPAddr[j]);
      }
      // for(int j = 0; j < nBadSP; j++){
      //   iotApps[i]->AddSPAddr(badSPAddr[j]);
      // }

      for(int j = 0; j < nGoodNode; j++){
        badIoT[i]->AddIoTAddr(goodIoTAddr[j]);
        
      }
      for(int j = 0; j < nBadNode; j++){
        badIoT[i]->AddIoTAddr(badIoTAddr[j]);
      }


      for(int j = (nGoodNode-nGSP); j < nGoodNode; j++)
      {
          badIoT[i]->AddSPIndex(j);
      }
      for(int j = (nGoodNode + nBadNode - nBSP); j < nGoodNode + nBadNode; j++)
      {
          badIoT[i]->AddSPIndex(j);
      }

      badIoT[i] -> SetSelfAddrIndex((uint16_t)nGoodNode+i);
      badIoT[i]->SetStartTime (Seconds (1.0));
      // badIoT[i]->SetAttackType(BadIoTApplication::BAD_MOUTH);
      badIoT[i]->SetStopTime (Seconds (simTime));
      badIoT[i]->Setup(bad_iot_speaker[i], bad_iot_listener[i], 1040, 1000, DataRate("1Mbps"), sinkPort);
      badIoT[i]->SetPos (badNodeContainer.Get (i) -> GetObject<MobilityModel>()->GetPosition());
      badNodeContainer.Get (i)->AddApplication (badIoT[i]);
    }
    
   

 
  Ptr<FlowMonitor> flowMonitor;
  FlowMonitorHelper flowHelper;

  flowMonitor = flowHelper.InstallAll();

  Simulator::Stop (Seconds (simTime));

  AnimationInterface anim ("testz.xml");
  uint32_t server_img = anim.AddResource("./scratch/server.png");
  anim.SetConstantPosition (wifiApNode.Get (0), 25, 25);
  anim.UpdateNodeColor (wifiApNode.Get (0), 3, 255, 200);

  for(int i = 0; i < nTTP; i++)
  {
     anim.UpdateNodeDescription (ttpContainer.Get (i), "TTP");
     anim.UpdateNodeImage(i, server_img);
  }

  for (int i = 0; i < nGoodNode; i++)
    {
      anim.UpdateNodeColor (goodNodeContainer.Get (i), 0, 255, 0);
      anim.UpdateNodeDescription (goodNodeContainer.Get (i), "Good Node");
    }

  for (int i = 0; i < nBadNode; i++)
    {
      anim.UpdateNodeColor (badNodeContainer.Get (i), 255, 0, 255);
      anim.UpdateNodeDescription (badNodeContainer.Get (i), "Bad Node");
    }

  for (int i = 0; i < nGoodSP; i++)
    {
      anim.UpdateNodeColor (goodSPContainer.Get (i), 0, 200, 155);
      anim.UpdateNodeDescription (goodSPContainer.Get (i), "Good SP");
    }
  Simulator::Run ();

  flowMonitor -> SerializeToXmlFile("flow.xml", true, true);
  Simulator::Destroy ();
  return 0;
}


// New example for trust time calculation
// Chapter one contribution all (literature drawback)
// Relative trust 
// chapter one
// chapter 6