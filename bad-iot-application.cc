#include "bad-iot-application.h"

#define PURPLE_CODE "\033[95m"
#define CYAN_CODE "\033[96m"
#define TEAL_CODE "\033[36m"
#define BLUE_CODE "\033[94m"
#define GREEN_CODE "\033[32m"
#define YELLOW_CODE "\033[33m"
#define LIGHT_YELLOW_CODE "\033[93m"
#define RED_CODE "\033[91m"
#define BOLD_CODE "\033[1m"
#define END_CODE "\033[0m"


using namespace std;
namespace ns3{

    NS_LOG_COMPONENT_DEFINE ("BadIoTApplication");
    NS_OBJECT_ENSURE_REGISTERED (BadIoTApplication);

    TypeId
    BadIoTApplication::GetTypeId ()
    {
        static TypeId tid =
            TypeId ("ns3::BadIoTApplication")
                .SetParent<Application> ()
                .AddConstructor<BadIoTApplication> ()
                .AddAttribute ("Interval", "Broadcast Interval", TimeValue (MilliSeconds (100)),
                                MakeTimeAccessor (&BadIoTApplication::m_broadcast_time),
                                MakeTimeChecker ());
        return tid;
    }

    TypeId
    BadIoTApplication::GetInstanceTypeId () const
    {
        return BadIoTApplication::GetTypeId ();
    }

    BadIoTApplication::BadIoTApplication():
        m_broadcast_time(2)
    {

    }

    BadIoTApplication::~BadIoTApplication()
    {

    }

    void
    BadIoTApplication::SetAttackType(int attack)
    {
        attack_type = attack;
    }

    void
    BadIoTApplication::Setup(Ptr<Socket> _speak_socket, Ptr<Socket> _list_socket, uint32_t pktSize, uint32_t nPkts, DataRate dataRate, uint16_t _port)
    {
        speaker_socket = _speak_socket;
        listener_socket = _list_socket;
        m_packetSize = pktSize;
        m_nPackets = nPkts;
        m_dataRate = dataRate;
        port = _port;

    }


    void
    BadIoTApplication::StartApplication ()
    {
        NS_LOG_FUNCTION (this);

        m_running = true;
        m_packetSent = 0;

        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
        if (listener_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        NS_LOG_INFO(RED_CODE<<"IoT app started successfully"<<END_CODE);
        listener_socket->Listen();
        listener_socket->ShutdownSend();

        listener_socket->SetRecvCallback(MakeCallback( &BadIoTApplication::ReceivePacket, this));

        listener_socket->SetAcceptCallback (
        MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
        MakeCallback (&BadIoTApplication::HandleAccept, this));
        
        NS_LOG_INFO(RED_CODE<<InetSocketAddress::ConvertFrom(iot[selfAddrIndex]).GetIpv4()<<END_CODE);

        
       
    //We will periodically (every 1 second) check the list of neighbors, and remove old ones (older than 5 seconds)
     if(find(sp_indexs.begin(), sp_indexs.end(), selfAddrIndex) == sp_indexs.end())
        {
            Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
            Time random_offset = MilliSeconds (rand->GetValue (50, 3000));
            Simulator::Schedule (Seconds(service_taking_period) + random_offset, &BadIoTApplication::ScheduleService, this);
            Simulator::Schedule (Seconds (trust_request_period) + random_offset, &BadIoTApplication::RequestForTrust, this);
        }
    }



    void 
    BadIoTApplication::StopApplication (void)
    {
        m_running = false;

        
        if(listener_socket)
            {
            listener_socket->Close ();
            }
            if(speaker_socket)
            {
                speaker_socket->Close();
            }
        for(auto x : m_socketList)
        {
            x->Close();
        }
        m_socketList.clear();
    }

    void 
    BadIoTApplication::HandleAccept (Ptr<Socket> s, const Address& from)
    {
        s->SetRecvCallback (MakeCallback (&BadIoTApplication::ReceivePacket, this));
        m_socketList.push_back (s);
    }

    void
    BadIoTApplication::AddSPIndex(int i)
    {
        sp_indexs.push_back(i);
    }
    void
    BadIoTApplication::AddTTPAddr(Address a){
        ttp.push_back(a);
    }

    void 
    BadIoTApplication::AddSPAddr(Address a)
    {
        sp.push_back(a);
    }

    void 
    BadIoTApplication::AddIoTAddr(Address a)
    {
        iot.push_back(a);
    }
    void 
    BadIoTApplication::AddTTPPos(Vector a){
        ttp_pos.push_back(a);
    }

    void
    BadIoTApplication::SetPos(Vector a){
        pos = a;
    }
    
    void 
    BadIoTApplication::SetSelfAddrIndex(uint16_t i)
    {
        selfAddrIndex = i;
    }

    void
    BadIoTApplication::RequestForTrust()
    {
        // Get the closest ttp from this node;

        // total_service_taken = 0;
        uint32_t nodeId = 0;
        int i = 0;
        double dis = DBL_MAX;
        double d;
        m_packetSent += 1;
        for( ;i < (int) ttp_pos.size(); i++){
            d = (pos-ttp_pos[i]).GetLength();
            if(d < dis)
            {
                nodeId = (uint32_t) i;
            }
        }

        // closet ttp 
        Address ttpAddress = ttp[nodeId];

        uint16_t s = honest_iot;
        cout<<"######################################################\n\n";
        cout<<"Talking "<<InetSocketAddress::ConvertFrom(iot[selfAddrIndex]).GetIpv4()<<endl;
        cout<<"My window size is \t"<<window.size()<<endl;

        for(auto timeUnit : window)
            {
                for(auto rec : timeUnit)
                {
                    cout<<InetSocketAddress::ConvertFrom(rec.first).GetIpv4()<<"\t"<<rec.second.first<<"  "<<rec.second.second<<endl;
                }

                cout<<"***********************\n";
            }

        cout<<"Printing time window\n";
        for(auto x : timeSlot)
        {
            cout<<x.GetSeconds()<<"    ";
        }
        cout<<"\nFinished sharing my window record\n";

        cout<<"######################################################\n\n";
        // take service request to other iot
        // Address idr = iot[s];
        
        // Always maintain from(iot)  -> to(sp)
        PacketData dt = PacketData();

        int counter = 0;
        if((int) window.size() <= 0)
        {
            dt.SerializeRequestForTrust(
                    counter,
                    PacketData::REQUEST_FOR_TRUST,
                    selfAddrIndex,
                    s,
                    (uint16_t)0,
                    (uint16_t)0,
                    Now()
                );
            counter++;

        }
        else
        {
            // send all the current window data to ttp
            for(int i = 0; i < (int) window.size(); i++)
            {
                    
                for (auto const& p : window[i])
                {
                    
                    if(counter > 14)
                    {   

                        dt.SerializeLenForRequestForTrust();
                        Ptr<Packet> packet = Create<Packet>(dt.GetBuffer(), 255);
                        Ptr<Socket> speak = Socket::CreateSocket (GetNode(), TcpSocketFactory::GetTypeId ());
                        speak->Bind();
                        speak->Connect(ttpAddress);
                        speak->Send(packet);


                        NS_LOG_INFO(BLUE_CODE<< "This is first packet. Successfully sent trust request for " << InetSocketAddress::ConvertFrom(iot[s]).GetIpv4() << "\t to" << InetSocketAddress::ConvertFrom(ttpAddress).GetIpv4() <<"\t packet size is " <<dt.GetLength()<<END_CODE);


                        dt.ClearBuffer();
                        counter = 0;

                    }
                    int idx = find(iot.begin(), iot.end(), p.first) - iot.begin();

                    dt.SerializeRequestForTrust(
                        counter,
                        PacketData::REQUEST_FOR_TRUST,
                        selfAddrIndex,
                        (uint16_t) idx,
                        (uint16_t)p.second.first,
                        (uint16_t)p.second.second,
                        timeSlot[i]
                    );
                    counter++;

                }

            }
        }
        
        dt.SerializeLenForRequestForTrust();
        Ptr<Packet> packet = Create<Packet>(dt.GetBuffer(),255);
        
        Ptr<Socket> speak = Socket::CreateSocket (GetNode(), TcpSocketFactory::GetTypeId ());
        speak->Bind();
        speak->Connect(ttpAddress);
        speak->Send(packet);



        NS_LOG_INFO(BLUE_CODE<< "This is second packet. Successfully sent trust request for " << InetSocketAddress::ConvertFrom(iot[s]).GetIpv4() << "\t to" << InetSocketAddress::ConvertFrom(ttpAddress).GetIpv4() <<"\t packet size is " <<dt.GetLength()<<END_CODE);


        Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
        Time random_offset = MilliSeconds (rand->GetValue (50, 3000));

        Simulator::Schedule ( Seconds(trust_request_period) + random_offset,
        &BadIoTApplication::RequestForTrust, this);
         Ptr<UniformRandomVariable> rand2 = CreateObject<UniformRandomVariable> ();
        Time offset = MilliSeconds (rand2->GetValue (100, 700));

        Simulator::Schedule ( Seconds(3.0) + offset,
        &BadIoTApplication::ScheduleRequestForTrust, this);
    }

    void
    BadIoTApplication::ScheduleRequestForTrust()
    {
        Address ttpAddress = ttp[0];
        uint16_t s = honest_iot;
        PacketData ddd = PacketData();
        ddd.SerializeTrustRequestPacket(PacketData::TRUST_REQUEST_GLOBAL, selfAddrIndex ,s);
        Ptr<Packet> req_packet = Create<Packet>(ddd.GetBuffer(), 255);
        Ptr<Socket> speak2 = Socket::CreateSocket (GetNode(), TcpSocketFactory::GetTypeId ());
        speak2->Bind();
        speak2->Connect(ttpAddress);
        speak2->Send(req_packet);
    }
    void
    BadIoTApplication::ScheduleReply(Time ts, uint16_t my_adrs, Address to_adrs)
    {
        PacketData sendData = PacketData();
        sendData.SerializeResponseForService(
            PacketData::RESPONSE_FOR_SERVICE,
            ts,
            my_adrs
        );

        Ptr<Packet> sendpacket = Create<Packet> (sendData.GetBuffer(), 255);
        
        Ptr<Socket> speak = Socket::CreateSocket (GetNode(), TcpSocketFactory::GetTypeId ());
        speak->Bind();
        speak->Connect(to_adrs);
        speak->Send(sendpacket);

    }
    

    void
    BadIoTApplication::ScheduleService()
    {
        
        Ptr<UniformRandomVariable> r = CreateObject<UniformRandomVariable> ();
        uint16_t s = (uint16_t)r -> GetValue (0, iot.size());

        // if(total_service_taken % (int)(trust_request_period/service_taking_period) == 13 || total_service_taken % (int)(trust_request_period/service_taking_period) == 23 || total_service_taken % (int)(trust_request_period/service_taking_period) == 11)
        // {
        //     s = honest_iot;
        // }
        int window_cell = total_service_taken % (int)(trust_request_period/service_taking_period);

        window_cell = window_cell % 5;

        if(window_cell == 2 || window_cell == 3)
        {
            s = (uint16_t)r-> GetValue(0, sp_indexs.size());
            s = (uint16_t)sp_indexs[s];
            // cout<<"Taking service from a sp "<<s<<"  IP is   "<<InetSocketAddress::ConvertFrom(iot[s]).GetIpv4()<<endl;
        }

        while(s==selfAddrIndex)
        {
            s = (uint16_t)r -> GetValue (0, iot.size());
        }
        Address idr = iot[s];
        CallForService(idr);

        Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
            Time random_offset = MilliSeconds (rand->GetValue (5, 50));

        Simulator::Schedule ( Seconds(service_taking_period) + random_offset,
            &BadIoTApplication::ScheduleService, this);
    }


    void 
    BadIoTApplication::CallForService(Address a)
    {

        
        PacketData pd = PacketData();

        int i = find(iot.begin(), iot.end(), a) - iot.begin();

        pd.SerializeRequestForService(
            PacketData::REQUEST_FOR_SERVICE,
            Now(),
            (uint16_t)i

        );
        
        Ptr<Packet> sendpacket = Create<Packet> (pd.GetBuffer(), 255);

        // uint8_t *buff = pd.GetBuffer();
        // for(int i = 0; i < 20; i++)
        // {
        //     cout<<(int)buff[i]<<" ";
        // }
        // cout<<endl;
        Ptr<Socket> speak = Socket::CreateSocket (GetNode(), TcpSocketFactory::GetTypeId ());
        speak->Bind();
        speak->Connect(a);
        speak->Send(sendpacket);
        total_service_taken++;
        NS_LOG_INFO(GREEN_CODE<<"Packt sent to iot from \t"<<InetSocketAddress::ConvertFrom(iot[selfAddrIndex]).GetIpv4()<<" for service."<<END_CODE);
    }


    void
    BadIoTApplication::ReceivePacket (Ptr<Socket> sock)
    {
        Address from;
        Ptr<Packet> packet;
        while( (packet = sock->RecvFrom (from)) )
        {
            NS_LOG_INFO(YELLOW_CODE << "CODE RECEIVED AT " << Now().GetSeconds() << END_CODE);
            if(packet->GetSize()==0)break;
            InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
            NS_LOG_INFO(RED_CODE<<"Incoming information from "<<address.GetIpv4()<<END_CODE);

            
            uint8_t data[255];
            packet->CopyData(data, 255);
            uint8_t type = data[0];


            if(type == PacketData::RESPONSE_FOR_SERVICE)
            {
                PacketData pd = PacketData();
                pd.SetBuffer(data, 255); 

                pd.DeserializeResponseForService();
                Time send_time = pd.GetTime();
                bool trst = IsSuccessFulTrx(Simulator::Now(), send_time);

                NS_LOG_INFO(GREEN_CODE<<"Received a service from \t"<<address.GetIpv4()<<"\t and provided service is"<<trst<<END_CODE);
                Address adrs = InetSocketAddress(address.GetIpv4(), port);

                
                UpdateTrustWindow(adrs, trst);
            }

       

            if(type == PacketData::RESPONSE_FOR_TRUST)
            {
                
                PacketData pd = PacketData();
                pd.SetBuffer(data, 255); 
                pd.DeserializeResponseForTrust();
                Address adr = iot[pd.GetFromAddress()];
                double trust = pd.GetTrust();

                NS_LOG_INFO(PURPLE_CODE<<"Trust value of "<<InetSocketAddress::ConvertFrom(adr).GetIpv4()<<"is  "<<trust<<END_CODE);

             

            }

            if(type == PacketData::REQUEST_FOR_SERVICE)
            {
                PacketData pd = PacketData();

                Address adrs = InetSocketAddress(address.GetIpv4(), port);
                
                pd.SetBuffer(data, 255);
                pd.DeserializeRequestForService();
                Time ts = pd.GetTime();
                uint16_t addrss_ind = pd.GetFromAddress();

                NS_LOG_INFO(YELLOW_CODE<<"Incoming request for service from "<<address.GetIpv4()<<" "<<address.GetPort() <<END_CODE);

                
                Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
                Time service_rtime = MilliSeconds (rand->GetValue (18, max_service_time_delay));
                // cout<<"Service from bad "<<InetSocketAddress::ConvertFrom(iot[selfAddrIndex]).GetIpv4()<<" at delay is "<<service_rtime.GetMilliSeconds()<<endl;
                Simulator::Schedule(service_rtime, &BadIoTApplication::ScheduleReply, this, service_rtime, addrss_ind, adrs);
                
                NS_LOG_INFO(TEAL_CODE<<"Service packet send to "<<address.GetIpv4()<<" after (mili secs) \t"<<service_rtime.GetMilliSeconds()<<END_CODE);
            }
           
        }
    }

    bool
    BadIoTApplication::IsSuccessFulTrx(Time rcvT, Time sndT)
    {
        if(attack_type == BAD_MOUTH)
        {
            return false;
        }
        else if (attack_type == BALLOT_STUFF){
            return true;
        }
        else{
            double diff = sndT.GetMilliSeconds();
            return (diff <= delay_threshold) ; 
        }
    }


    void 
    BadIoTApplication::UpdateTrustWindow(Address a, bool isSuccessFul)
    {
        int sz = (int)window.size();
        map<Address, pair<int, int>> m = map<Address, pair<int, int>>();

        

        if(sz == 0){
            timeSlot.push_back(Now());


            if(isSuccessFul)
            {
                m[a] = make_pair(1, 0);
            }
            else{
                m[a] = make_pair(0, 1);
            }

            window.push_back(m);
        }
        
        else if(sz < 5){
            
            Time t = timeSlot[sz-1];
            double dif = (Simulator::Now() - t).GetSeconds();

            // as the time slot is still valid so do things on the current window don't need to add new one
            if(dif < time_window_size)
            {
               
                //  check if there is any trnx with a?
                if (window[sz-1].find(a) == window[sz-1].end()) {
                    // no trnx in the current window :'()
                    if(isSuccessFul)
                    {
                        window[sz-1][a] = make_pair(1, 0);
                    }
                    else{
                        window[sz-1][a] = make_pair(0, 1);
                     }   

                } else {
                    if(isSuccessFul)
                    {
                        window[sz-1][a].first += 1;
                    }
                    else{
                        window[sz-1][a].second +=1;
                     }  
                }
            }
            else
            {
                // new window :)
                timeSlot.push_back(Now());
                if(isSuccessFul)
                {
                    m[a] = make_pair(1, 0);
                }
                else{
                    m[a] = make_pair(0, 1);
                }

                window.push_back(m);
            }

        }
        else if(sz == 5)
        {
            Time t = timeSlot[sz-1];
            double dif = (Simulator::Now() - t).GetSeconds();

            // as the time slot is still valid so do things on the current window don't need to add new one
            if(dif < time_window_size)
            {
               
                if (window[sz-1].find(a) == window[sz-1].end()) {
                    // no trnx in the current window :'()
                    if(isSuccessFul)
                    {
                        window[sz-1][a] = make_pair(1, 0);
                    }
                    else{
                        window[sz-1][a] = make_pair(0, 1);
                     }   

                } else {
                    if(isSuccessFul)
                    {
                        window[sz-1][a].first += 1;
                    }
                    else{
                        window[sz-1][a].second +=1;
                     }  
                }
            }
            else{
                window.erase(window.begin());
                timeSlot.erase(timeSlot.begin());
                timeSlot.push_back(Now());
                    if(isSuccessFul)
                    {
                        m[a] = make_pair(1, 0);
                    }
                    else{
                        m[a] = make_pair(0, 1);
                    }

                    window.push_back(m);
            }
        }
        

        
    }


}