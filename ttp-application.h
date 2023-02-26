#ifndef WAVETEST_TTP_APPLICATION_H
#define WAVETEST_TTP_APPLICATION_H
#include "ns3/application.h"
#include "ns3/core-module.h"
#include "ns3/packet.h"
#include "ns3/wave-net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include <iostream>
#include <iterator>
#include <map>
#include <utility>
#include <vector>
#include "ns3/tcp-header.h"
#include "packet-data.h"
#include <bits/stdc++.h>
#include <fstream>
#include "ns3/gnuplot.h"


using namespace std;



namespace ns3
{
    /**
     * TTP maintains a feedback repo where it saves all pair trust value
     * <127.168.0.1, 127.168.0.2>, 0.96 -> means 127.168.0.1 gives 0.96 feedback to 127.168.0.2
     * 
     * 
     * These feedbacks are collected from the nodes
     * Nodes send time window full of transactions to ttp
     * 
     * From that it generates direct trust based on time decay and negative and positive feedback
     * then it full up the repository
     * 
     * 
     * When any node wants trust rating of a sp
     * it get feedback from the direct feedback repository
     * 
     * if that pair is not present it request for recommendation to all ttp about that sp
     * and collect indirect transaction from it's repository
     *
     * 
     * 
     * basic works:
     *      1. maintain a repository
     *      2. collect feedbacks from nodes and fill up the reposityr
     *      3. request to other ttp for recomm
     *      4. send feedback to node 
     * 
     * 
     * Need to have all address of ttp
    */

    class TTPApplication : public ns3::Application
    {
        public: 
            
            static TypeId GetTypeId (void);
            virtual TypeId GetInstanceTypeId (void) const;

            TTPApplication();
            ~TTPApplication();

            // Let the iot know about ttp and sp
            // SetUp(Ptr<NodeContainer> _ttp, Ptr<NodeContainer> _sp);

            /** periodically send trust value to ttp 
             */ 
            // void BroadcastInformation();

            // // it sends service request packet to sp. if the trust value is higher than threshold
            // void CallForService();

            // // it sends trust reqeust for a sp
            // void RequestForTrust();

            // /** \brief This function is called when a net device receives a packet. 
            //  * I connect to the callback in StartApplication. This matches the signiture of NetDevice receive.
            //  */

            void Setup (Ptr<Socket> _speaker_socket, Ptr<Socket> _listener_socket, uint32_t packetSize, uint32_t nPackets, 
            DataRate dataRate,uint16_t _port);

            void ReceivePacket (Ptr<Socket> soc);

            void Create2DPlotFile();

            // requested_addr node recommendation is needed to search in all ttp

            
            // send a to that addr
            void SendPkt(Address addr, Ptr<Packet> pkt);

            void AddTTPAddr(Address a);
            void AddSPAddr(Address a);
            void AddIoTAddr(Address a);

            void AddToRepo(pair<Address, Address> p, double d);

            void AddToAddrRepo(Address a, pair<Address, double> p);


            vector<Address> GetFilteredRecommenders(Address from, Address to);
            void ComputeTrust(Address from, Address to);

            double calcDistance(pair<double, double> a, pair<double, double> b);

            void SetSelfAddressIndex(int index);

            void updateDataset(double trust);

            double DirectTrustCalculator(Address from, Address to);

            vector<Address> GetRecommenders(Address from, Address to);

            double GetSimilarityValue(Address i, Address rk);

            double GetConfidence(Address rk, Address j);

            double RecommendationTrustCalculator(Address i, Address j, vector<Address>rks);
            // void SetBroadcastInterval (Time interval);

            // // void AddTrustToWindow(Address& sender, double trust);

            // /** \brief Update a neighbor's last contact time, or add a new neighbor
            //  */
    
            // void SetWifiMode (WifiMode mode);

            // /*
            //     From the window in the node it generates the direct trust value to give periodic feed back to its ttp
            //     It generates a vector of pair of address and trust value
            //     [<192.0.0.1, 0.5>, <192.0.0.2, 0.9> .......]
            // */
            // std::vector<std::pair<Address,double>>   GetPeriodicFeedBack();

            /** \brief Remove neighbors you haven't heard from after some time.
             */
            Gnuplot2dDataset dataset;

            
        private:
            /** \brief This is an inherited function. Code that executes once the application starts
             */
            void StartApplication();
            void HandleAccept (Ptr<Socket> s, const Address& from);
            virtual void StopApplication (void);

            void printWindow();           
            // std::unordered_map<Address, std::vector<std::pair<Time, pair<int, int>>>> feedback_window;

            Time m_broadcast_time =  MilliSeconds (100); /**< How often do you broadcast messages */ 
            Ptr<NetDevice> m_waveDevice; /**< A WaveNetDevice that is attached to this device */  
            
            int selfAddressIndex;

            map<Address, vector<map<Address, pair<int, int>>>> direct_trust_repo;

            map<Address, vector<Time>> timeSlot_repo;

            map<Address, vector<Address>> transaction_records;

            vector <Address> ttps;
            vector<ns3::Address> sp_addrs;
            vector<ns3::Address> iot_addrs;
            vector<Ptr<Socket>>m_socketList;
            // pair <from, to>
            map< pair<Address, Address>, double> repo;

            map<Address, vector<pair<Address, double>>> addr_repo;

            Time m_time_limit = Seconds(5); /**< Time limit to keep neighbors in a list */
            
            WifiMode m_mode; /**< data rate used for broadcasts */
            //You can define more stuff to record statistics, etc.
            
            

            Ptr<Socket> listener_socket;
            Ptr<Socket> speaker_socket;

            uint32_t m_packetSize;
            uint32_t m_nPackets;
            DataRate m_dataRate;
            uint16_t port;

            bool m_running;
            uint32_t m_packetsSent;


            double trust_computation_delay = 2.0;
            

    };
}

#endif