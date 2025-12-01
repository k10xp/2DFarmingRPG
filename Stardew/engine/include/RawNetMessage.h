#ifndef RAWNETMESSAGE_H
#define RAWNETMESSAGE_H

#include "IntTypes.h"

enum NetRawMessageType
{
    UnreliableDataMessageComplete,
    ReliableDataMessageComplete,
    ReliableDataMessageFragment, /* part of a packet that's too big to send all at once */
    ReliableDataMessageAck,
};

struct NetReliableMessageHeader
{
    u32 messageIdentifier;
};

struct NetFragmentMessageHeader
{
    u16 numFragments; /* how many fragments in total */
    u16 sequenceNum;  /* which message in the sequence is thos one */
};

void NetMsg_Parse(u8* data, enum NetRawMessageType* pOutType, u8** outBody);

struct NetReliableMessageHeader* NetMsg_GetReliableHeader(u8* data);

struct NetFragmentMessageHeader* NetMsg_GetFragmentHeader(u8* data);

u32 NetMsg_GetAckedIdentifier(u8* data);

/*these all return the number of bytes written*/

int NetMsg_WriteUnreliableCompleteDataPacket(u8* dataOut, u8* dataIn, int dataSize);

u32 NetMsg_GetReliableMessageIdentifier();

int NetMsg_WriteReliableCompleteDataPacket(u8* dataOut, u8* dataIn, int dataSize, u32 messageIdentifier);

int NetMsg_WriteReliableFragmentDataPacket(u8* dataOut, u8* dataIn, int dataSize, u16 numFragments, u16 sequenceNumber, u32 messageIdentifier);

int NetMsg_WriteReliableDataAckPacket(u8* dataOut, u32 messageIdentifier);

int NetMsg_SizeOfHeaders(enum NetRawMessageType msgType);


#endif
