/************************************************************************
 * Name		: frameStruct.h
 * Course	: CS5310
 * Date		: 04/28/18
 * Description	: This file holds the structure of the frame which is the 
 * 		  unit of communication between SP and CSP. It has 5 fields
 * 		  1. frameType - Denotes the type of frame being exchanged
 * 		    0 - Request Frame
 * 		    1 - Data Frame
 * 		    2 - Positive acknowledgement Frame
 * 		    3 - Negative acknowledgement type
 * 		  2. seqno - Sequence number of the frame.
 * 		  3. sourceAddress - Address of the SP sending the frame
 * 		  4. destinationAddress - Address of the SP this frame is 
 * 		     addressed to.
 * 		  5. data - data being exchanged in the frame.
 **************************************************************************/
#define         REQUEST_FRAME   0
#define         DATA_FRAME      1
#define         POS_ACK_FRAME   2
#define         NEG_ACK_FRAME   3

struct frame{
        int frameType;
        int seqno;
        int sourceAddress;
        int destinationAddress;
        char *data;
};
