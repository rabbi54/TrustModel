#ifndef WAVETEST_BAD_IOT_APPLICATION_H
#define WAVETEST_BAD_IOT_APPLICATION_H
#include "ns3/application.h"
#include "ns3/core-module.h"
#include "ns3/wave-net-device.h"
#include "ns3/wifi-phy.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include <vector>
#include <utility>
#include <bits/stdc++.h>
#include <map>
#include<float.h>
#include "ns3/tcp-header.h"
#include "ns3/ipv4.h"
#include "packet-data.h"


using namespace std;
using namespace ns3;
namespace ns3
{
    /**
     * Main three function
     * 
     * 1. Request for trust to its neighbour ttp
     * 2. Request for service to a service provider
     * 3. Send feedback window when ask for trust info to its neighbour ttp
     * 4. Manage the trust window haha
     * 
     * 
    */

    class BadIoTApplication : public ns3::Application
    {
        public: 
            
            static TypeId GetTypeId (void);
            virtual TypeId GetInstanceTypeId (void) const;

            BadIoTApplication();
            ~BadIoTApplication();

            
            // to request for trust it also sends it's window info to ttp


            void Setup (Ptr<Socket> _speaker_socket, Ptr<Socket> _listener_socket, uint32_t packetSize, uint32_t nPackets, DataRate dataRate, uint16_t _port);


            void RequestForTrust();

            void AddTTPAddr(Address a);
            void AddSPAddr(Address a);
            void AddIoTAddr(Address a);

            void AddTTPPos(Vector a);
            void AddSPIndex(int i);
            bool IsSuccessFulTrx(Time rcvT, Time sndT);
            void UpdateTrustWindow(Address a, bool isSucess);

        
            void ReceivePacket (Ptr<Socket> sock);
            void SetSelfAddrIndex(uint16_t i);
            // void SetWifiMode (WifiMode mode);
            void SetPos(Vector a);
            void CallForService(Address a);

            void ScheduleService();

            void ScheduleRequestForTrust();

            void SetAttackType(int attack);
            void ScheduleReply(Time ts, uint16_t my_adrs, Address to_adrs);

            static const uint8_t BAD_MOUTH = 1;
            static const uint8_t BALLOT_STUFF = 2;

        private:
            /** \brief This is an inherited function. Code that executes once the application starts
             */
            void HandleAccept (Ptr<Socket> s, const Address& from);
            Ptr<Socket>     speaker_socket;
            Ptr<Socket>     listener_socket;

            vector<ns3::Address> ttp;
            vector<ns3::Address> sp;
            vector<ns3::Address> iot;
            Vector pos;
            vector<int>sp_indexs;

            vector<Vector> ttp_pos;

            void StartApplication();
            void StopApplication();
            
            vector<map<Address, pair<int, int>>> window;

            vector<Time> timeSlot;

            Time m_broadcast_time; /**< How often do you broadcast messages */ 
            uint32_t m_packetSize = 1000; /**< Packet size in bytes */
            Ptr<NetDevice> m_waveDevice; /**< A WaveNetDevice that is attached to this device */  
            
            Time m_time_limit = Seconds(5); /**< Time limit to keep neighbors in a list */
            uint32_t        m_nPackets;
            DataRate        m_dataRate;

            uint16_t port;
            bool m_running;
            uint32_t m_packetSent;
            vector<Ptr<Socket> > m_socketList;
            WifiMode m_mode; /**< data rate used for broadcasts */
            //You can define more stuff to record statistics, etc.

            uint16_t selfAddrIndex;


            /******Upload config ******/
            // int total_service_taken = 0;

            // double delay_threshold = 19.0; // into miliseconds
            // double time_window_size = 20.0;
            // double trust_threshold = 0.5;

            // uint16_t honest_iot = 94;

            // double service_taking_period = 4.0;
            // double trust_request_period = 100.0;

            // double max_service_time_delay = 38; // into miliseconds

            // int attack_type = 19;


            /***************LOCAL */
            int total_service_taken = 0;

            double delay_threshold = 19.0; // into miliseconds
            double time_window_size = 5.0;
            double trust_threshold = 0.5;

            uint16_t honest_iot = 20;

            double service_taking_period = 1.0;
            double trust_request_period = 25.0;

            double max_service_time_delay = 38; // into miliseconds
            int attack_type = 19;
    };
}

#endif