#include "ttp-application.h"



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

namespace ns3{

    NS_LOG_COMPONENT_DEFINE ("TTPApplication");
    NS_OBJECT_ENSURE_REGISTERED (TTPApplication);

    TypeId
    TTPApplication::GetTypeId ()
    {
        static TypeId tid =
            TypeId ("ns3::TTPApplication")
                .SetParent<Application> ()
                .AddConstructor<TTPApplication> ()
                .AddAttribute ("Interval", "Broadcast Interval", TimeValue (MilliSeconds (100)),
                                MakeTimeAccessor (&TTPApplication::m_broadcast_time),
                                MakeTimeChecker ());
        return tid;
    }

    TypeId
    TTPApplication::GetInstanceTypeId () const
    {
        return TTPApplication::GetTypeId ();
    }

    TTPApplication::TTPApplication()
    {

    }

    TTPApplication::~TTPApplication()
    {

    }

    void 
    TTPApplication::Setup (Ptr<Socket> _speaker_socket, Ptr<Socket> _listener_socket, uint32_t packetSize, uint32_t nPackets, 
            DataRate dataRate,uint16_t _port)
    {
        listener_socket = _listener_socket;
        speaker_socket = _speaker_socket;
        m_packetSize = packetSize;
        m_nPackets = nPackets;
        m_dataRate = dataRate;
        port = _port;
    }

    void
    TTPApplication::StartApplication ()
    {
        NS_LOG_FUNCTION (this);

        m_running = true;
        m_packetsSent = 0;
        InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), port);
        if (listener_socket->Bind(local) == -1)
        {
            NS_FATAL_ERROR("Failed to bind socket");
        }
        
        dataset.Add(0.0, 0.5);
   

        NS_LOG_INFO(TEAL_CODE<<"TTP started successfully"<<END_CODE);
        listener_socket->Listen();
        listener_socket->ShutdownSend();


        listener_socket->SetRecvCallback(MakeCallback( &TTPApplication::ReceivePacket, this));

        listener_socket->SetAcceptCallback (
        MakeNullCallback<bool, Ptr<Socket>, const Address &> (),
        MakeCallback (&TTPApplication::HandleAccept, this));




        // Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable> ();
        //     Time random_offset = MilliSeconds (rand->GetValue (0, 200));
        Simulator :: Schedule(Seconds(26.5),  &TTPApplication::printWindow, this);

    }


    void TTPApplication::Create2DPlotFile ()
    {
        std::string fileNameWithNoExtension = "plot-2d";
        std::string graphicsFileName        = fileNameWithNoExtension + ".png";
        std::string plotFileName            = fileNameWithNoExtension + ".plt";
        std::string plotTitle               = "Trust value for Honest Service Provider";
        std::string dataTitle               = "2-D Data";

        // Instantiate the plot and set its title.
        Gnuplot plot (graphicsFileName);
        plot.SetTitle (plotTitle);

     
        // Make the graphics file, which the plot file will create when it
        // is used with Gnuplot, be a PNG file.
        plot.SetTerminal ("png");

        // Set the labels for each axis.
        plot.SetLegend ("Simulation Time in Seconds", "Domain Trust score");

        // Set the range for the x axis.
        plot.AppendExtra ("set xrange [0:+15]");
        plot.AppendExtra ("set yrange [+0.0:+1.0]");
        dataset.SetTitle (dataTitle);
        dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

        plot.AddDataset (dataset);

        // Open the plot file.
        std::ofstream plotFile (plotFileName.c_str());

        // Write the plot file.
        plot.GenerateOutput (plotFile);

        // Close the plot file.
        plotFile.close ();
        // cout<<"\n\n plot created \n\n"; 
    }


    void 
    TTPApplication::StopApplication (void)
    {
        m_running = false;

        // Create2DPlotFile(dataset);
        
        if (listener_socket)
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

    

    

    void TTPApplication::HandleAccept (Ptr<Socket> s, const Address& from)
    {
        NS_LOG_INFO(CYAN_CODE << "Client connected" <<END_CODE );
        s->SetRecvCallback (MakeCallback (&TTPApplication::ReceivePacket, this));
        Ptr<Socket> ss = Socket::CreateSocket ( GetNode(),TcpSocketFactory::GetTypeId ());
        m_socketList.push_back (s);
    }

    void
    TTPApplication::SendPkt(Address addr, Ptr<Packet> pkt){
        Ptr<Socket> speak = Socket::CreateSocket (GetNode(), TcpSocketFactory::GetTypeId ());
        speak->Bind();
        speak->Connect(addr);
        speak->Send(pkt);
        // m_waveDevice->Send (pkt, addr, 0x88dc);

    }

    // a ttp has all ttp address to communicate for recommendation feedback
    void
    TTPApplication::AddTTPAddr (Address a)
    {
        ttps.push_back(a);
    }

    // a ttp has all sp to get the address and make repository
    void 
    TTPApplication::AddSPAddr(Address a)
    {
        sp_addrs.push_back(a);
    }

    // a ttp has all iot to get the address and make repository
    void 
    TTPApplication::AddIoTAddr(Address a)
    {
        iot_addrs.push_back(a);
    }

    void 
    TTPApplication::SetSelfAddressIndex(int index)
    {
        selfAddressIndex = index;
    }
    // this node and sp don't have any direct communication in this ttp
    //  so ttp will request others to get the feedback fo a.
    /*
        From a ttp
    */


    void TTPApplication::AddToRepo(pair<Address, Address> p , double d)
    {
        repo[p] = d;
        AddToAddrRepo(p.first, make_pair(p.second, d));
        AddToAddrRepo(p.second, make_pair(p.first, d));
        NS_LOG_INFO(RED_CODE<<"Trust info addred to repo"<<END_CODE);
        
    }

    void TTPApplication::AddToAddrRepo(Address a, pair<Address, double>p){
        addr_repo[a].push_back(p);
    }


    double
    TTPApplication::DirectTrustCalculator(Address from, Address to)
    {
        int inx = 0;
        double p=0;
        double n = 0;

        Time ref = Simulator::Now();
        double time_diff = 0;

        for(auto window : direct_trust_repo[from])
        {
            time_diff = (ref - timeSlot_repo[from][inx]).GetMinutes();
            if(window.find(to) != window.end())
            {
                p += (exp(-(0.7)*time_diff*3) * p + window[to].first);
                n += (exp(-(0.7)*time_diff*3) * n + window[to].second);
               
            }
            inx++;
        }
        double res = (p+1)/(p+n*1.5+2);
        return res;
    }

    
    void
    TTPApplication:: updateDataset(double trust)
    {
        // cout<<Now().GetSeconds()<<"\t"<<trust<<endl;
        // cout<<"\n\n\n\n";
        Time simTime  = Simulator::Now();
        double tm = simTime.GetSeconds();
        double tr_rating = trust;
        dataset.Add(tm, tr_rating);
    }

    double TTPApplication::calcDistance (pair<double, double> a, pair<double, double> b)
    {
        return sqrt(pow((a.first - b.first), 2) + pow((a.second - b.second), 2));
    }

    // also sends trust to the iot
    void TTPApplication::ComputeTrust(Address from, Address to)
    {
        // NS_LOG_INFO(PURPLE_CODE<<"Hi there here to send trust value and i'm scheduled to send "<<directTrust<<"\to "<<InetSocketAddress::ConvertFrom(node).GetIpv4()<<" of "<<InetSocketAddress::ConvertFrom(sp).GetIpv4()<<END_CODE);
        
        
        double directTrust = DirectTrustCalculator(from, to);

        vector<Address> recommenders = GetFilteredRecommenders(from, to);

        auto end = recommenders.end();
        for (auto it = recommenders.begin(); it != end; ++it) {
            end = remove(it + 1, end, *it);
        }
    
        recommenders.erase(end, recommenders.end());


        double trust_score = 0;
        if(recommenders.size() > 1)
        {
            double recommendation_trust = RecommendationTrustCalculator(from, to, recommenders);

            double avg_dt_rk = 0;

            for(auto rk : recommenders)
            {
                avg_dt_rk += DirectTrustCalculator(from, rk);
            }

            avg_dt_rk = avg_dt_rk / (recommenders.size() + 1);

            

            if(avg_dt_rk >= 0.5)
            {
                int inx = 0;
                
                int interaction = 0;
                Time last_int_time = timeSlot_repo[from][0];
                double time_diff = 0;

                for(auto window : direct_trust_repo[from])
                {
                    if(window.find(to) != window.end())
                    {
                        if(timeSlot_repo[from][inx].GetSeconds() > last_int_time.GetSeconds())
                        {
                            last_int_time = timeSlot_repo[from][inx];
                        }
                        interaction++;
                    }
                    inx++;
                }
                time_diff = (Simulator::Now() - last_int_time).GetMinutes();
                // cout<<time_diff<<"\t"<<interaction<<endl;
                double omega = 1 - pow(0.1, exp(-(time_diff * 3) * interaction));

                trust_score = omega * directTrust + (1-omega) * recommendation_trust;

                cout<<"Recommendation details: "<<InetSocketAddress::ConvertFrom(to).GetIpv4()<<" ";
                cout<<Now().GetSeconds()<<"\t"<<trust_score<<"\t"<<omega<<"\t"<<directTrust<<"\t"<<recommendation_trust<<endl;

                cout<<"Recommender lists: ";
                for(auto ixxxx : recommenders)
                {
                    cout<<InetSocketAddress::ConvertFrom(ixxxx).GetIpv4()<<'\t';
                }
                cout<<"\n\n";
            }
            else{
                trust_score = directTrust;
                // cout<<Now().GetSeconds()<<"\t"<<trust_score<<endl;
            }

        }
        else{
            trust_score = directTrust;
        }
        // double recomm = ComputeRecommendationTrust(sp);
        // double tt = (0.7) * directTrust + (0.3) * recommendation_trust;

    //    TODO::
        // just write the syntesis trust
        // updateDataset(tt);

        // int fr = find(iot_addrs.begin(), iot_addrs.end(), from) - iot_addrs.begin();
        
        cout<<InetSocketAddress::ConvertFrom(from).GetIpv4()<<"\t"<<Now().GetSeconds()<<"\t"<<trust_score<<endl;
        int i = find(iot_addrs.begin(), iot_addrs.end(), to) - iot_addrs.begin();
        // for(i =0; i< (int)sp_addrs.size(); i++)
        // {
        //     if(sp_addrs[i] == sp)
        //     {
        //         break;
        //     }
        // }
        PacketData dt = PacketData();
        dt.SerializeResponseForTrust(
            PacketData::RESPONSE_FOR_TRUST,
            (uint16_t) i,
            trust_score
        );

        Ptr<Packet> packet = Create<Packet>(dt.GetBuffer(),255);

            
        Ptr<Socket> speak = Socket::CreateSocket (GetNode(), TcpSocketFactory::GetTypeId ());
        speak->Bind();
        speak->Connect(from);
        speak->Send(packet);
        NS_LOG_INFO(BLUE_CODE<< "Successfully sent trust value for " << InetSocketAddress::ConvertFrom(iot_addrs[i]).GetIpv4() << "\t to" << InetSocketAddress::ConvertFrom(from).GetIpv4() <<"\t trust value is " <<trust_score<<END_CODE);
    }


    double
    TTPApplication::GetConfidence(Address rk, Address j)
    {
        double p=0;
        double n = 0;

        Time ref = Simulator::Now();
        double time_diff = 0;
        int inx = 0;

        for(auto window : direct_trust_repo[rk])
        {
            time_diff = (ref - timeSlot_repo[rk][inx]).GetSeconds();
            if(window.find(j) != window.end())
            {


                p += (exp(-(0.7)*time_diff) * window[j].first);
                n += (exp(-(0.7)*time_diff) * window[j].second);
               
            }
        }

        double rightPart = (12*(p+1)*(n+1))/pow((p+n+2), 2)*(p+n+3);
        double res = 1 - sqrt(rightPart);
        return res;
    }

    double
    TTPApplication::GetSimilarityValue(Address i, Address rk)
    {
        vector<Address> common = vector<Address>();

        for(auto addr : transaction_records[i])
        {
            if(find(transaction_records[rk].begin(),transaction_records[rk].end(), addr) != transaction_records[rk].end())
            {
                common.push_back(addr);
            }
        }

        double diff = 0;
        for(auto com : common)
        {
           diff += abs(DirectTrustCalculator(i, com) - DirectTrustCalculator(rk, com));
        }
        
        double res = 1 - (diff/(common.size() + 1));
        return res;
    }

    vector<Address>
    TTPApplication::GetRecommenders(Address from, Address to)
    {
        vector<Address> rk = vector<Address>();
        for(auto addr : transaction_records[from])
        {
            if(find(transaction_records[addr].begin(), transaction_records[addr].end(), to) != transaction_records[addr].end())
            {
                rk.push_back(addr);
            }
        }
        // cout<<"Communication btw \t"<<InetSocketAddress::ConvertFrom(from).GetIpv4() <<"\t"<<InetSocketAddress::ConvertFrom(to)<<"Recommenders size is\t"<<rk.size()<<endl;
        return rk;
    }


    double
    TTPApplication::RecommendationTrustCalculator(Address i, Address j, vector<Address> rks)
    {
        double dT_i_rk, S_i_rk, C_rk_j, dT_rk_j;
        vector<double> dT_i_rk_vec;
        vector<double> S_i_rk_vec;
        vector<double> C_rk_j_vec;
        vector<double> dT_rk_j_vec;

        for(auto rk : rks)
        {
            dT_i_rk = DirectTrustCalculator(i, rk);
            S_i_rk = GetSimilarityValue(i, rk);
            C_rk_j = GetConfidence(rk, j);
            dT_rk_j = DirectTrustCalculator(rk, j);

            dT_i_rk_vec.push_back(dT_i_rk);
            S_i_rk_vec.push_back(S_i_rk);
            C_rk_j_vec.push_back(C_rk_j);
            dT_rk_j_vec.push_back(dT_rk_j);
        }


        double denominator = 0;
        for(int itr = 0; itr < (int)rks.size(); itr++)
        {
            denominator += (dT_i_rk_vec[itr]*S_i_rk_vec[itr]*C_rk_j_vec[itr]);
        }

        double res = 0;

        for(int itr = 0; itr < (int)rks.size(); itr++)
        {
            res += (dT_i_rk_vec[itr]*S_i_rk_vec[itr]*C_rk_j_vec[itr] * dT_rk_j_vec[itr]) / denominator;
        }

        return res;
    }



    vector<Address>
    TTPApplication::GetFilteredRecommenders(Address from, Address to)
    {
        // Function to perform K-means clustering
        // vector<vector<double>> kmeans (vector<double> points, int K)

        vector<Address> recommenders = GetRecommenders(from, to);

        if(recommenders.size() >= 2)
        {
            vector<pair<double, double>> points;

            map<pair<double, double>, Address> reContainer;
            double p1=0, p2=0;
            
            for(int i = 0; i < (int)recommenders.size(); i++)
            {

                p1 = DirectTrustCalculator(from, recommenders[i]);
                p2 = DirectTrustCalculator(recommenders[i], to);

                points.push_back(make_pair(p1, p2));
                reContainer[make_pair(p1, p2)] = recommenders[i];

            }

            vector<pair<double, double>> clusters[2];


            Ptr<UniformRandomVariable> r = CreateObject<UniformRandomVariable> ();
            int fst = (int)r -> GetValue (0, points.size());
            int secd = (int)r -> GetValue (0, points.size());
            while(secd == fst)
            {
                secd = (int)r -> GetValue (0, points.size());
            }

            pair<double, double> center1 = points[fst];
            pair<double, double> center2 = points[secd];
            


            int number_max_iter = 50;
            // Loop until the clusters converge
            while (number_max_iter--)
            {
                // Assign each point to the nearest cluster
                for (auto point : points)
                {
                double dist1 = calcDistance (point, center1);
                double dist2 = calcDistance (point, center2);

                if (dist1 < dist2)
                {
                    clusters[0].push_back (point);
                }
                else
                {
                    clusters[1].push_back (point);
                }
                }

                // Calculate the new cluster centers
                pair<double, double> newCenter1 = make_pair(0, 0);
                for (auto point : clusters[0])
                {
                    newCenter1.first += point.first;
                    newCenter1.second += point.second;
                }
                newCenter1.first /= (clusters[0].size()+1);
                newCenter1.second /= (clusters[0].size()+1);

                pair<double, double> newCenter2 = make_pair(0, 0);
                for (auto point : clusters[1])
                {
                    newCenter2.first += point.first;
                    newCenter2.second += point.second;
                }
                newCenter2.first /= (clusters[1].size()+1);
                newCenter2.second /= (clusters[1].size()+1);


                // Check for convergence
                if (calcDistance(newCenter1, center1) < 1e-6 && calcDistance(newCenter2, center2) < 1e-6)
                {
                    break;
                }

                // Update the cluster centers
                center1 = newCenter1;
                center2 = newCenter2;

                if(number_max_iter > 1)
                {
                    clusters[0].clear();
                    clusters[1].clear();
                }
                // Clear the clusters
                
            }

            vector<pair<double, double>> right_clstr;

            
            if(center1.first >= center2.first)
            {
                right_clstr = clusters[0];
            }
            else{
                right_clstr = clusters[1];
            }


            vector<Address> res;

            for(auto rk : right_clstr)
            {
                res.push_back(reContainer[rk]);
            }
            return res;
        }
        else{
            return recommenders;
        }
    }


    void
    TTPApplication::printWindow()
    {
        cout<<"Starting Print Window <<<<<<<<<<<<<<<<<    \n\n";
        for(auto win : direct_trust_repo)
        {
            cout<<"Printing window of address \t"<<InetSocketAddress::ConvertFrom(win.first).GetIpv4()<<endl;
            cout<<"Window length is "<<win.second.size()<<endl;
            for(auto timeUnit : win.second)
            {
                for(auto rec : timeUnit)
                {
                    cout<<InetSocketAddress::ConvertFrom(rec.first).GetIpv4()<<"\t"<<rec.second.first<<"  "<<rec.second.second<<endl;
                }

                cout<<"***********************\n";
            }

            cout<<"Printing time window\n";
            for(auto x : timeSlot_repo[win.first])
            {
                cout<<x.GetSeconds()<<"    ";
            }
            cout<<"\n";
        }

        cout<<"\n**********************************************\n\n\n";
        Simulator :: Schedule(Seconds(26.5),  &TTPApplication::printWindow, this);
    }

    void
    TTPApplication::ReceivePacket (Ptr<Socket> sock)
    {
       
        Address from;
        Ptr<Packet> packet;
        while( (packet = sock->RecvFrom (from)) )
        {
            
            InetSocketAddress address = InetSocketAddress::ConvertFrom (from);
            NS_LOG_INFO(RED_CODE<<"Incoming information from "<<address.GetIpv4()<<"\t to "<<InetSocketAddress::ConvertFrom( ttps[selfAddressIndex]).GetIpv4()<<END_CODE);

            uint8_t data[255];
            packet->CopyData(data,sizeof(data));

            uint8_t type = data[0];
            // Address adrs = InetSocketAddress(address.GetIpv4(), port);

            if(type == PacketData::REQUEST_FOR_TRUST)
            {
                // i should receive two packets from the iot device 
                NS_LOG_INFO(RED_CODE<<"Incoming information from "<<address.GetIpv4()<<"about "<< int (type)<<END_CODE);

                PacketData pd = PacketData();
                pd.SetBuffer(data, 255);

                

                pd.DeserializeRequestForTrust(0);
                // Address iot_addr_2 = iot_addrs[pd.GetToAddress()];
                
                uint16_t len = pd.GetLength();

                
                int receive_data = len/16;

                
                // The direct trust window only contains last 5 time unit
                if(timeSlot_repo[iot_addrs[pd.GetFromAddress()]].size() >= 5 && timeSlot_repo[iot_addrs[pd.GetFromAddress()]].back().GetDouble() != pd.GetTime().GetDouble())
                {
                    timeSlot_repo[iot_addrs[pd.GetFromAddress()]].clear();
                    direct_trust_repo[iot_addrs[pd.GetFromAddress()]].clear();
                    transaction_records[iot_addrs[pd.GetFromAddress()]].clear();
                }




                // time slot is empty so add the current time
                if(timeSlot_repo[iot_addrs[pd.GetFromAddress()]].size() == 0)
                {
                    timeSlot_repo[iot_addrs[pd.GetFromAddress()]].push_back(pd.GetTime());
                }

                map<Address, pair<int, int>> unit_window;
                if(direct_trust_repo[iot_addrs[pd.GetFromAddress()]].size() > 0 && (timeSlot_repo[iot_addrs[pd.GetFromAddress()]].back().GetDouble() == pd.GetTime().GetDouble()))
                {

                    // int len = (int)direct_trust_repo[iot_addrs[pd.GetFromAddress()]].size();
                    // if(len >= 5)
                    // {
                    //     direct_trust_repo[iot_addrs[pd.GetFromAddress()]].erase(direct_trust_repo[iot_addrs[pd.GetFromAddress()]].begin(), direct_trust_repo[iot_addrs[pd.GetFromAddress()]].begin() + (len - 2)); 
                    //     timeSlot_repo[iot_addrs[pd.GetFromAddress()]].erase(
                    //         timeSlot_repo[iot_addrs[pd.GetFromAddress()]].begin(),
                    //         timeSlot_repo[iot_addrs[pd.GetFromAddress()]].begin() + (len - 2)
                    //     );
                    // }
                    unit_window = direct_trust_repo[iot_addrs[pd.GetFromAddress()]].back();
                    direct_trust_repo[iot_addrs[pd.GetFromAddress()]].pop_back();

                    

                }

                // if the latest time is not equal to pd time then add new window
                if(timeSlot_repo[iot_addrs[pd.GetFromAddress()]].back().GetDouble() != pd.GetTime().GetDouble())
                {
                    timeSlot_repo[iot_addrs[pd.GetFromAddress()]].push_back(pd.GetTime());

                }


                Time initial_time = timeSlot_repo[iot_addrs[pd.GetFromAddress()]].back();



                map<Time, pair<int, int>> m = map<Time, pair<int, int>>();

                 

             

                for(int i = 0; i < receive_data; i++)
                {
                     pd.DeserializeRequestForTrust(i);
                     if(pd.GetSuccessful() == 0 && pd.GetUnsuccessful() == 0)
                     {
                        continue;
                     }
                     m[pd.GetTime()] = make_pair(
                        pd.GetSuccessful(),
                        pd.GetUnsuccessful()
                     );


                     uint16_t iot_1 = pd.GetFromAddress();
                     uint16_t iot_2 = pd.GetToAddress();
                     uint16_t success = pd.GetSuccessful();
                     uint16_t unsucess = pd.GetUnsuccessful();
                     Time t = pd.GetTime();


                     if(initial_time.GetDouble() != t.GetDouble())
                     {
                        direct_trust_repo[iot_addrs[iot_1]].push_back(unit_window);

                        timeSlot_repo[iot_addrs[iot_1]].push_back(t);
                        unit_window.clear();
                        initial_time = t;
                     }
                     transaction_records[iot_addrs[pd.GetFromAddress()]].push_back(iot_addrs[pd.GetToAddress()]);

                     unit_window[iot_addrs[iot_2]] = make_pair(success, unsucess);
                     
                     
                }

                direct_trust_repo[iot_addrs[pd.GetFromAddress()]].push_back(unit_window);

                // Simulator :: Schedule(Seconds(1.0),  &TTPApplication::ComputeTrust, this, iot_addrs[pd.GetFromAddress()], iot_addrs[pd.GetToAddress()]);

            }

            else if(type == PacketData::TRUST_REQUEST_GLOBAL)
            {
                PacketData pd = PacketData();
                pd.SetBuffer(data, 255);
                pd.DeserializeTrustRequestPacket();
                Simulator :: Schedule(Seconds(trust_computation_delay),  &TTPApplication::ComputeTrust, this, iot_addrs[pd.GetFromAddress()], iot_addrs[pd.GetToAddress()]);
            }

            else if(type == PacketData::REQUEST_FOR_RECOMM)
            {
                // PacketData dt = PacketData();
                // dt.SetBuffer(data, 255);
                // dt.DeserializeRequestForRecomm();
                // uint16_t sp_index = dt.GetFromAddress();
                // Address a = sp_addrs[sp_index];
            }
            else if(type == PacketData::RESPONSE_FOR_RECOMM)
            {
                PacketData dt = PacketData();
                dt.SetBuffer(data, 255);
                dt.DeserializeResponseForRecomm(0);
                uint16_t len = dt.GetLength();

                int receive_data = len/12;


                for(int i = 0; i < receive_data; i++)
                {
                     dt.DeserializeResponseForRecomm(i);
                     AddToRepo(make_pair(iot_addrs[dt.GetFromAddress()],sp_addrs[dt.GetToAddress()]), dt.GetTrust());

                     
                }
            }
           
            
        }
    }


}