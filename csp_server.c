/************************************************************************
 *  Name         : csp_server.c
 *  Developer    : Harika Vusirikala
 *  Date         : 04/29/18
 *  Course	 : CS5310
 *  Description  : This program simulates the switch that controls the flow
 *                 of traffic in ethernet server. Whenever it receives a 
 *                 request from a client, it responds with an 
 *                 appropriate acknowledgement. When it receives a data
 *                 frame it puts it in a queue and forwards whenever the 
 *                 destination SP is ready.
 ************************************************************************/
#include        "../lib/unp.h"
#include        "../lib/error.c"
#include        "../lib/wrapsock.c"
#include        "../lib/wrapunix.c"
#include        "../lib/wraplib.c"
#include        "../lib/wrapstdio.c"
#include	"../lib/writen.c"
#include        "../lib/readline.c"
#include        "../lib/frameStruct.h"
#define		QUEUE_SIZE	10

int
main(int argc, char **argv)
{
	int    		i, maxi, maxfd, listenfd, connfd, sockfd, nsockfd;
        // Creates client array to hold the established connections
	int    		client[FD_SETSIZE];
	int		nready,rfront=0,rrear=0,dfront=0,drear=0;
	int		rsize=0,dsize=0,ack_num=0;
	ssize_t 	n;
        struct 		sockaddr_in      servaddr,cliaddr;
	FILE		*log_file;
        char   		buff[MAXLINE];
	// Creates read set and write set
	fd_set 		rset, allset,wset;
	char 		line[MAXLINE];
	socklen_t 	clilen;
	struct  frame   dframe,ackframe,reqframe;
	// dframeq - Holds incoming dataframes
	// rframeq - Holds unserved requests
	struct	frame	dframeq[QUEUE_SIZE],rframeq[QUEUE_SIZE];
	
	// Opening the Log file.	
	log_file = Fopen("Server_log.txt","w");
	
	if(!log_file){
		err_quit("Log file Open Error");
	}
	// Creates a TCP listening socket
        listenfd = Socket(AF_INET, SOCK_STREAM, 0);
	
	// Binding the server to a pre-defined port.	
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family      = AF_INET;
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        servaddr.sin_port        = htons(SERV_PORT); 

        Bind(listenfd, (SA *) &servaddr, sizeof(servaddr));
	
	// Listens for incoming requests
        Listen(listenfd, LISTENQ);
	maxfd = listenfd;
	maxi=-1;

	// Intializing the client array with -1
        for ( i=0;i<FD_SETSIZE ;i++ )
		client[i] = -1;
	// Intializing allset and write set.
	FD_ZERO(&allset);
	FD_ZERO(&wset);
	
	// Adding Listen descriptor to allset
	FD_SET(listenfd,&allset);
	
	// Makes the server process run forever to serve incoming
	// connections, frame processing, forwarding and requests.
	for( ; ; ){
	    // Assign all the descriptors in allset to read set
	    rset = allset;
	    // Wait for any incoming connections / client request / 
	    // data frames / forwarding frames
	    nready = Select(maxfd+1, &rset, &wset, NULL, NULL);	
	    // Establish connection with any incoming connections on the
	    // Listen descriptor and add the connection descriptor to the 
	    // client array, readset and write set
	    if (FD_ISSET(listenfd, &rset)){
		clilen = sizeof(cliaddr);
		// Establish connection with client that is requesting 
		// connection
		connfd = Accept(listenfd, (SA *) &cliaddr, &clilen);
		// Add the connection descriptor to the client array	
		for (i = 0; i < FD_SETSIZE; i++)
		    if (client[i] < 0) {
			client[i] = connfd;
			break;
		    }
		// If there are more than 256 clients, connection will be 
		// closed. 
		if (i == FD_SETSIZE){
		    close(connfd);
		    fprintf(log_file,"Too many clients");
		}
		
		// Add the new connection to all set and write set
		FD_SET(connfd, &allset);
		FD_SET(connfd, &wset);
		
		// Assign the new connection as maxfd if it is greater
		// than the existing maxfd
		if (connfd > maxfd)
		    maxfd = connfd;
		if (i > maxi)
		    maxi = i;
		// If there are no other descriptors that is ready to read/
		// write / connect, then go to the next iteration.
                if(--nready<=0)
		    continue;
	    }
	    
	    // Check if any of the descriptor is set.
	    for (i = 0; i <= maxi; i++){
		if ( (sockfd = client[i]) < 0)
		    continue;
		// If any of the descriptor in read set is set, read the
		// frame and process it.
		if (FD_ISSET(sockfd, &rset)) {
		    // Receive the frame from the client.
		    if ( (n = Read(sockfd, &dframe, MAXLINE)) > 0) {
			// If the frame recived is a Request frame and 
			// 1. The Dataframe queue(dqueue) is not full, 
			// send a positive acknowledgement to the client.
			// 2. If the data frame queue is full, enqueue the 
			// request in Request Queue(rqueue).
			// 3. If both Request Queue (rqueue) and Data frame
			// queue (dqueue) are full, send a negative acknow-
			// ledgement to the client.
			if(dframe.frameType == REQUEST_FRAME){
			    fprintf(log_file,"Receive Request from SP %d\n",i);
			    if(dsize!=QUEUE_SIZE){
				ackframe.frameType=POS_ACK_FRAME;
				ackframe.seqno = ack_num++;
				ackframe.sourceAddress = -2;
				ackframe.destinationAddress = i;
				ackframe.data = "";
				Writen(sockfd,&ackframe,sizeof(ackframe));
				fprintf(log_file,"Send Positive Reply to SP %d\n",ackframe.destinationAddress);
			    }
			    else if(dsize==QUEUE_SIZE && rsize!=QUEUE_SIZE){
				dframe.sourceAddress = i;
				rframeq[rrear] = dframe;
				rrear= (rrear+1)%QUEUE_SIZE;
				rsize++;
			    }
			    else if(dsize==QUEUE_SIZE && rsize==QUEUE_SIZE){
				ackframe.frameType = NEG_ACK_FRAME;
                                ackframe.seqno = ack_num++;
                                ackframe.sourceAddress = -2;
                                ackframe.destinationAddress = i;
                                ackframe.data = "";
                                Writen(sockfd,&ackframe,sizeof(ackframe));
				fprintf(log_file,"Send negative Reply to SP %d\n",ackframe.destinationAddress);
			    }
		    	}
			// If the received frame is a data frame, enqueue it
			// in Data Frame queue(dqueue).
                        else if(dframe.frameType == DATA_FRAME){
			    if(dsize!=QUEUE_SIZE){
			    	fprintf(log_file,"Receive data frame from SP %d (to SP %d)\n",i,dframe.destinationAddress);
				fflush(log_file);
                            	dframe.sourceAddress = i;
                            	dframeq[drear]=dframe;
			    	drear = (drear+1)%QUEUE_SIZE;
			    	dsize++;
			    }
                        }
                    }
		    // If the Read function on the client connection
		    // returns 0, close the connection and remove it 
		    // from all set and write set.
                    else if( (n=Read(sockfd,&dframe,MAXLINE))==0 ){
                        close(sockfd);
                        FD_CLR(sockfd, &allset);
			FD_CLR(sockfd, &wset);
                        client[i] = -1;
                    }
		    if(--nready<=0)
			break;
		}	
		// If the descriptor corresponding to the front data frame 
		// in data frame queue is set, the frame will be forwarded 
		// to that client.
		// As there is an empty slot in the data frame queue(dqueue)
		// Process the oldest request in request queue(rqueue).
		else if(FD_ISSET(sockfd, &wset)){
		    if(dsize>0){
		    	dframe = dframeq[dfront];
			if(dframe.destinationAddress == i){
			    fprintf(log_file,"Forward dataframe (from SP %d) to SP %d\n",dframe.sourceAddress,i);
			    //fflush(log_file);
		    	    dfront = (dfront+1)%QUEUE_SIZE;
		    	    dsize--;
			    Writen(sockfd,&dframe,sizeof(dframe));
			    
			    if(rsize>0){
				ackframe.frameType=POS_ACK_FRAME;
				ackframe.seqno = ack_num++;
				ackframe.sourceAddress = -2;
				ackframe.destinationAddress = rframeq[rfront].sourceAddress;
				ackframe.data ="";
				nsockfd=client[rframeq[rfront].sourceAddress];
				Writen(nsockfd,&ackframe,sizeof(ackframe));
				rfront=(rfront+1)%QUEUE_SIZE;
				fprintf(log_file,"Send Positive Reply to SP %d\n",ackframe.sourceAddress);
				//fflush(log_file);
				rsize--;
			   }
			}	
		    }
		    if(--nready<=0)
		    	break;
		}    
            }
	    fflush(log_file);
	}
}
