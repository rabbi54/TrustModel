#include "packet-data.h"

namespace ns3{


    // NS_LOG_COMPONENT_DEFINE("PacketData");
    // NS_OBJECT_ENSURE_REGISTERED(PacketData);

    // uint8_t REQUEST_FOR_TRUST = 1;
    // uint8_t RESPONSE_FOR_TRUST = 2;
    // uint8_t REQUEST_FOR_RECOMM = 3;
    // uint8_t RESPONSE_FOR_RECOMM = 4;
    // uint8_t REQUEST_FOR_SERVICE = 5;
    // uint8_tRESPONSE_FOR_SERVICE = 6;

    PacketData::PacketData()
    {
        len = 0;
    }

    PacketData::PacketData(uint8_t buff[], uint16_t len)
    {

    }

    PacketData::~PacketData()
    {

    }

    // TypeId PacketData::GetTypeId(void)
    // {
    //     static TypeId tid = TypeId ("ns3::PacketData")
    //         .AddConstructor<PacketData> ();
    //     return tid;
    // }

    // TypeId PacketData::GetInstanceTypeId (void) const
    // {
    //     return PacketData::GetTypeId ();
    // }


    void
    PacketData::ClearBuffer()
    {
        for(int i = 0; i < 255; i++)
        {
            buffer[0] = 0;
        }
    }

   
    uint8_t*
    PacketData::GetBuffer()
    {
        return buffer;
    }

    void PacketData::SetBuffer(uint8_t data[], uint16_t l = 255)
    {
        for(uint16_t i = 0; i < 255; i++)
        {
            buffer[i] = 0;
        }
        
        for(uint16_t i = 0; i < l; i++)
        {
            buffer[i]=data[i];
        }
    }
    uint8_t
    PacketData::GetType()
    {
        return type;
    }

    uint16_t
    PacketData::GetFromAddress()
    {
        return from_address;
    }

    uint16_t
    PacketData::GetToAddress()
    {
        return to_address;
    }

    double 
    PacketData::GetTrust()
    {
        return trust;
    }

    Time
    PacketData::GetTime()
    {
        return time;
    }

    uint16_t
    PacketData::GetSuccessful()
    {
        return nSuccess;
    }

    uint16_t
    PacketData::GetUnsuccessful()
    {
        return nUnsuccess;
    }

    // 110011


    void 
    PacketData::SerializeDouble(double a, int start)
    {
        uint64_t casted_num = (uint64_t) (a * PRESICION_TERM);

        buffer[start] = ((casted_num >> 0) & 0xff);
        buffer[start+1] = ((casted_num >> 8) & 0xff);

        buffer[start+2] = ((casted_num >> 16) & 0xff); 
        buffer[start+3] = ((casted_num >> 24) & 0xff);

        buffer[start+4] = ((casted_num >> 32) & 0xff); 
        buffer[start+5] = ((casted_num >> 40) & 0xff);

        buffer[start+6] = ((casted_num >> 48) & 0xff); 
        buffer[start+7] = ((casted_num >> 56) & 0xff);

     

        
    }


    double
    PacketData::DeserializeDouble(int start){
        uint8_t byte0 = buffer[start];
        uint8_t byte1 = buffer[start+1];

        uint8_t byte2 = buffer[start+2];
        uint8_t byte3 = buffer[start+3];
        uint8_t byte4 = buffer[start+4];
        uint8_t byte5 = buffer[start+5];

        uint8_t byte6 = buffer[start+6];
        uint8_t byte7 = buffer[start+7];
        uint64_t data = (uint64_t)byte7;


        data <<= 8;
        
        data |= (uint64_t)byte6;
        data <<= 8;
        data |= (uint64_t)byte5;

        data <<= 8;
        data |= (uint64_t)byte4;
        data <<= 8;
        data |= (uint64_t)byte3;

        data <<= 8;
        data |= (uint64_t)byte2;
        data <<= 8;
        data |= (uint64_t)byte1;

        data <<= 8;
        data |= (uint64_t)byte0;
        double temp = (double)data/PRESICION_TERM;

        return temp;

    }


    void
    PacketData::SerializeLenForRequestForTrust()
    {
        buffer[1] = ((len >> 0) & 0xff);
        buffer[2] = ((len >> 8) & 0xff);
    }


    
    void
    PacketData::SerializeRequestForTrust(int index, uint8_t type, uint16_t from_addr, uint16_t to_addr, uint16_t nSuccess, uint16_t nUnsuccess, Time time)
    {
        if(index > 15)
        {
            std::cout<<"*******************Something is wrong in the window ***************************\n";
            return;
        }
        if(index == 0){
            len = 1+2+2+2+2+2+8;
            buffer[0] = type;

        }
        else{
            len += 16;
        }
        buffer[index*16+3] = ((from_addr >> 0) & 0xff); 
        buffer[index*16+4] = ((from_addr >> 8) & 0xff);

        buffer[index*16+5] = ((to_addr >> 0) & 0xff); 
        buffer[index*16+6] = ((to_addr >> 8) & 0xff);

        buffer[index*16+7] = ((nSuccess >> 0) & 0xff);
        buffer[index*16+8] = ((nSuccess >> 8) & 0xff);

        buffer[index*16+9] = ((nUnsuccess >> 0) & 0xff);
        buffer[index*16+10] = ((nUnsuccess >> 8) & 0xff);
        
        SerializeDouble(time.GetSeconds(), index*16+11);
    }

    uint16_t
    PacketData::GetLength()
    {
        return len;
    }

    uint8_t 
    PacketData::GetPacketType()
    {
        return buffer[0];
    }

    uint16_t
    PacketData::ReadU16(int start)
    {
        uint8_t byte0 = buffer[start];
        uint8_t byte1 = buffer[start+1];
        uint16_t data = byte1;
        data <<= 8;
        data |= byte0;
        return data;
    }

    void 
    PacketData:: DeserializeRequestForTrust(int index)
    {
        if(index > 15)
        {
            std::cout<<"*******************Something is wrong in the window ***************************\n";
            return;
        }
        if(index == 0)
        {
            type = buffer[0];
            len = ReadU16(1);
        }
        // type = buffer[16*index+0];
        from_address = ReadU16(16*index+3);
        to_address = ReadU16(16*index+5);
        nSuccess = ReadU16(16*index+7);

        nUnsuccess = ReadU16(16*index+9);

        time = Time::FromDouble (DeserializeDouble(16*index+11), Time::S);

    }

    void
    PacketData::SerializeResponseForTrust(uint8_t type, uint16_t address, double trust)
    {
        len = 1 + 2 + 2 + 8;
        buffer[0] = type;
        
        buffer[1] = ((len >> 0) & 0xff);
        buffer[2] = ((len >> 8) & 0xff);

        buffer[3] = ((address >> 0) & 0xff); 
        buffer[4] = ((address >> 8) & 0xff);

        SerializeDouble(trust, 5);

    }

    void
    PacketData::DeserializeResponseForTrust()
    {
        type = buffer[0];
        from_address = ReadU16(3);
        trust = DeserializeDouble(5); 
    }

    void 
    PacketData::SerializeRequestForRecomm(uint8_t type, uint16_t address)
    {
        len = 1 + 2 + 2;
        buffer[0] = type;
        
        buffer[1] = ((len >> 0) & 0xff);
        buffer[2] = ((len >> 8) & 0xff);

        buffer[3] = ((address >> 0) & 0xff); 
        buffer[4] = ((address >> 8) & 0xff);   
    }

    void
    PacketData::DeserializeRequestForRecomm()
    {
        type = buffer[0];
        from_address = ReadU16(3);
    }

    void 
    PacketData::SerializeResponseForRecomm(int index, uint8_t type, uint16_t address_from, uint16_t address_to, double trust)
    {
        
        if(index > 15)
        {
            std::cout<<"*******************Something is wrong in request ***************************\n";
            return;
        }
        if(index == 0){
            len = 1+2+2+2+8;
            buffer[0] = type;

        }
        else{
        len += 12;
        }
        buffer[index*12+3] = ((address_from >> 0) & 0xff); 
        buffer[index*12+4] = ((address_from >> 8) & 0xff);

        buffer[index*12+5] = ((address_to >> 0) & 0xff); 
        buffer[index*12+6] = ((address_to >> 8) & 0xff);

        
        SerializeDouble(trust, index*12+7);
    }

    void 
    PacketData::DeserializeResponseForRecomm(int index)
    {
        
        if(index > 15)
        {
            std::cout<<"*******************Something is wrong in the request ***************************\n";
            return;
        }
        if(index == 0)
        {
            type = buffer[0];
            len = ReadU16(1);
        }
        from_address = ReadU16(12*index+3);
        to_address = ReadU16(12*index+5);
        trust = DeserializeDouble(12*index+7);
    }

    void 
    PacketData::SerializeRequestForService(uint8_t type, Time initialize_time, uint16_t address)
    {
        len = 1+2+2+8;
        buffer[0] = type;
        
        buffer[1] = ((len >> 0) & 0xff);
        buffer[2] = ((len >> 8) & 0xff);

        buffer[3] = ((address >> 0) & 0xff); 
        buffer[4] = ((address >> 8) & 0xff);
        SerializeDouble(initialize_time.GetSeconds(), 5);
        
    }

    void 
    PacketData::DeserializeRequestForService()
    {
        from_address = ReadU16(3);
        time = Time::FromDouble (DeserializeDouble(5), Time::S);
    }

    void 
    PacketData::SerializeResponseForService(uint8_t type, Time initialize_time, uint16_t address)
    {
        len = 1+2+2+8;
        buffer[0] = type;
        
        buffer[1] = ((len >> 0) & 0xff);
        buffer[2] = ((len >> 8) & 0xff);

        buffer[3] = ((address >> 0) & 0xff); 
        buffer[4] = ((address >> 8) & 0xff);
        SerializeDouble(initialize_time.GetSeconds(), 5);
    }

    void 
    PacketData::DeserializeResponseForService()
    {
        type = buffer[0];
        from_address = ReadU16(3);
        time = Time::FromDouble (DeserializeDouble(5), Time::S);
    }




    void 
    PacketData::SerializeTrustRequestPacket(uint8_t type, uint16_t from_address, uint16_t to_address)
    {
        len = 1+2+2+2;
        buffer[0] = type;
        
        buffer[1] = ((len >> 0) & 0xff);
        buffer[2] = ((len >> 8) & 0xff);

        buffer[3] = ((from_address >> 0) & 0xff); 
        buffer[4] = ((from_address >> 8) & 0xff);

        buffer[5] = ((to_address >> 0) & 0xff); 
        buffer[6] = ((to_address >> 8) & 0xff);
        
    }

    void 
    PacketData::DeserializeTrustRequestPacket()
    {
        from_address = ReadU16(3);
        to_address = ReadU16(5);
    }
}