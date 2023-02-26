/**
 * Packet send types:
 * 
 * 1. Request for trust :
 *      From: IoT       To: TTP
 *      params: type: uint8_t, from_address: uint16_t, to_address: uint16_t, nSuccess : uint16_t, nUnsuccess : uint16_t, time : double
 * 
 * 
 * 2. Response for trust:
 *      From: TTP        To: IoT
 *      params: type: uint8_t, address : uint16_t, trust_val: double
 * 
 * 
 * 3. Request for recommendation:
 *      From: TTP        To: TTP
 *      params: type: uint8_t, address: uint16_t
 * 
 * 
 * 4. Response for recommendation:
 *      From: TTP        To: TTP
 *      params: type: uint8_t, address_from: uint16_t, address_to : uint16_t, trust: double
 * 
 * 
 * 5. Request for service:
 *      From: IoT        To: SP
 *      params: type: uint8_t, time_to_initialize: double, address: uint16_t
 * 
 * 
 * 6. Response for service:
 *      From: SP        To: IoT
 *      params: type: uint8_t, time_to_initialize: double, address: uint16_t
 * 
 * 
 * 
*/

#ifndef PACKET_DATA_H
#define PACKET_DATA_H

#include "ns3/vector.h"
#include "ns3/nstime.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"

namespace ns3
{
    class PacketData
    {
        public:

            // static TypeId GetTypeId(void);
	        // TypeId GetInstanceTypeId(void) const;
            PacketData();

            PacketData(uint8_t buff[], uint16_t len);
	       ~PacketData();
            // provide a buffer now i can access the values
            // provide the values as fucntion return buffer

            /**
             * \brief serialize all the params to the buffer the parameters are passed to provide window data
             * \param index where the writing in buffer starts
             * \param type to recognize the data
             * \param from_addr service requester address index
             * \param to_addr service responser address index
             * \param nSuccess number of successfull transaction in that time window
             * \param nUnsuccess number of unsuccessfull transaction in that time window
             * \param time unit of a timeslot
             * 
             * 
            */
           void SerializeRequestForTrust(int index, uint8_t type, uint16_t from_addr, uint16_t to_addr, uint16_t nSuccess, uint16_t nUnsuccess, Time time);

            void SerializeLenForRequestForTrust();
           
            /**
             * \brief This will read from the buffer and update the value accordingly 
             * params: type: uint8_t, from_address: uint16_t, to_address: uint16_t, nSuccess : uint16_t, nUnsuccess : uint16_t, time : Time
            */
            void DeserializeRequestForTrust(int index);


            void SerializeResponseForTrust(uint8_t type, uint16_t address, double trust);

            void DeserializeResponseForTrust();

            void SerializeRequestForRecomm(uint8_t type, uint16_t address);

            void DeserializeRequestForRecomm();

            void SerializeResponseForRecomm(int index, uint8_t type, uint16_t address_from, uint16_t address_to, double trust);

            void DeserializeResponseForRecomm(int indx);

            void SerializeRequestForService(uint8_t type, Time initialize_time, uint16_t address);

            void DeserializeRequestForService();

            void SerializeResponseForService(uint8_t type, Time initialize_time, uint16_t address);
            void DeserializeResponseForService();


            void SerializeTrustRequestPacket(uint8_t type, uint16_t from_address, uint16_t to_address);
            void DeserializeTrustRequestPacket();

            uint8_t GetPacketType();
            uint16_t ReadU16(int start);

            uint8_t* GetBuffer();

            void SetBuffer(uint8_t data[], uint16_t len);
            
            uint8_t GetType();
            uint16_t GetFromAddress();
            uint16_t GetToAddress();
            Time GetTime();
            uint16_t GetSuccessful();
            uint16_t GetUnsuccessful();

            uint16_t GetLength();
            double GetTrust();

            void ClearBuffer();
            
            static const uint8_t REQUEST_FOR_TRUST = 1;
            static const uint8_t RESPONSE_FOR_TRUST = 2;
            static const uint8_t REQUEST_FOR_RECOMM = 3;
            static const uint8_t RESPONSE_FOR_RECOMM = 4;
            static const uint8_t REQUEST_FOR_SERVICE = 5;
            static const uint8_t RESPONSE_FOR_SERVICE = 6;

            static const uint8_t TRUST_REQUEST_GLOBAL = 13;


            
        
        private:
            /**
             * \brief takes a double value and a starting index and write it to the buffer
             * \param a a dobule value to write in buffer
             * \param start starting index
            */
            void SerializeDouble(double a, int start);

            double DeserializeDouble(int start);

            // void SerializeTime(Time a, int start);

            // Time DeserializeTime(int start);

            uint64_t PRESICION_TERM = 10000000000000;
            uint8_t buffer[255];

            uint8_t type;
            uint16_t from_address;
            uint16_t to_address;
            uint16_t nSuccess;
            uint16_t nUnsuccess;
            Time time;
            uint16_t len;
            double trust;

    };

}


#endif