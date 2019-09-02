/************************************************************************
 *  Name         : sp_client.c
 *  Developer    : Harika Vusirikala
 *  Date         : 05/01/18
 *  Course       : CS5310
 *  Description  : This program simulates an Ethernet Client that receives
 *                 and sends data via CSP by establishing a TCP connection
 *                 with it.
 *************************************************************************/

#include	"stdio.h"
#include        "../lib/unp.h"
#include        "../lib/error.c"
#include	"../lib/wrapsock.c"
#include	"../lib/wraplib.c"
#include	"../lib/wrapstdio.c"
#include        "../lib/wrapunix.c"
#include	"../lib/readline.c"
#include        "../lib/writen.c"
#include	"time.h"
#include	"../lib/frameStruct.h"

int
main(int argc, char **argv)
{
        int    	sockfd, n , frame_num , wait_for , i , rq_frame_num=1;
        char   	recvline[MAXLINE],*file_name,*log_file_name;
        struct 	sockaddr_in      servaddr;
	// File descriptors for input and output files
	FILE 	*fp,*ofp;
	// dframe 	- Data Frame
	// ackframe 	- Acknowledgement Frame
	// reqframe 	- Request Frame
	struct 	frame	dframe,ackframe,reqframe;
	// Time Limit for Binary exponential Algorithm.
	int	time_limit=2,dest_address;
	long 	clientId;
	srand(time(NULL));

	// Client recieves 2 arguments from Command line. 
	// 1. IP Address
	// 2. Client ID Number (ranges from 1-10)
        if (argc != 3)
                err_quit("usage: a.out <IPaddress> <Client number>");
	
	// Based on the Client Number from the commandline, assign the 
	// input, output file names.
	clientId = strtol(argv[2],NULL,10);
	printf("%d",clientId);
        switch(clientId){
            case 0: file_name = "sim_input_0.txt";
		    log_file_name = "activity_log_0.txt";
                    break;
            case 1: file_name = "sim_input_1.txt";
		    log_file_name = "activity_log_1.txt";
                    break;
            case 2: file_name = "sim_input_2.txt";
		    log_file_name = "activity_log_2.txt";
                    break;
            case 3: file_name = "sim_input_3.txt";
		    log_file_name = "activity_log_3.txt";
                    break;
            case 4: file_name = "sim_input_4.txt";
		    log_file_name = "activity_log_4.txt";
                    break;
            case 5: file_name = "sim_input_5.txt";
		    log_file_name = "activity_log_5.txt";
                    break;
            case 6: file_name = "sim_input_6.txt";
		    log_file_name = "activity_log_6.txt";
                    break;
            case 7: file_name = "sim_input_7.txt";
		    log_file_name = "activity_log_7.txt";
                    break;
            case 8: file_name = "sim_input_8.txt";
		    log_file_name = "activity_log_8.txt";
                    break;
            case 9: file_name = "sim_input_9.txt";
		     log_file_name = "activity_log_9.txt";
                    break;
            default: printf("Invalid Client: Client Id must be between 0-9");
		     exit(1);
                    break;
        }

	// Create a TCP Socket
        if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
                err_sys("socket error");

	// Build Server Address structure with server address provided 
	// from the console and pre-defined server port.
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port   = htons(SERV_PORT);
        if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
                err_quit("inet_pton error for %s", argv[1]);

	// Connect to the Server using the server address structure 
	// and the socket
        if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0)
                err_sys("connect error");
	
	//printf("Please enter the file name:\t");
	//scanf("%s",&file_name);
	
	//Open the Input file in read mode 	
	fp = Fopen(file_name,"r+");
        
	if(!fp){
		err_quit("Input file Open Error");
	}
	
	// Open the log file in write mode
	ofp = Fopen(log_file_name,"w");
	if(!ofp){
		err_quit ("Log file Open Error");
	}

	// Read the input line till end of the file. If the input line is:
	// 1. Frame X to SP Y: send a request to the CSP. 
	//   a. If the acknowledgement sent by the server is positive,
	//      send the data frame to the server.
	//   b. If it receives negative acknowledgement, retry after a random
	//      time chosen from time_limit slots. First time it waits for 0
	//      or 1 slots, 2nd time 0,1,2 or 3 slots. Every time it retries, 
	//      the time_limit will be multiplied by 2.
	// 2. Wait for n frames: Receive n frames from CSP.
	while ( fgets(recvline, sizeof(recvline),fp)) {
	    time_limit=2;
	    if(recvline[0]=='F'){
		// Build a request frame to the server.
		frame_num = recvline[6]-'0';
		reqframe.frameType = REQUEST_FRAME;
	        reqframe.seqno = rq_frame_num;
	        reqframe.sourceAddress = -1;
		dest_address = recvline[15]-'0';
	        reqframe.destinationAddress = dest_address;
	 	reqframe.data = "";
		// Write the request frame to CSP
		Write(sockfd,&reqframe,sizeof(reqframe));
	        fprintf(ofp,"Send request to CSP to send data frame %d to SP %d\n",frame_num,dest_address);
		for(i=0;i<=3;i++){
		// Receive an acknowledgement from CSP
	        if((n=Read(sockfd,&ackframe, sizeof(ackframe)))>0){
		    // If the received acknowledgement is positive
		    // send the data frame to the server
	    	    if(ackframe.frameType == POS_ACK_FRAME){
	  	        dframe.frameType = DATA_FRAME;
			dframe.seqno = frame_num;
			fprintf(ofp,"Receive positive reply (permission) from CSP to send data frame %d to SP %d\n",frame_num,dest_address);
			dframe.destinationAddress = dest_address;
			dframe.sourceAddress = -1;
			dframe.data ="";
			Write(sockfd,&dframe,sizeof(dframe));
			fprintf(ofp,"Send (via CSP) data frame %d to SP %d\n",frame_num,dest_address);
			break;		    
		    }
		    // If the received acknowledgement is negative, retry 
		    // after a random time between 0 through time_limit
		    // to avoid collision.
		    else if(ackframe.frameType == NEG_ACK_FRAME){
			fprintf(ofp,"Receive reject reply from CSP to send data frame %d to SP %d\n",frame_num,reqframe.destinationAddress);
			sleep(rand()%time_limit);
			fprintf(ofp,"Retransmit request to CSP to send data frame %d to SP %d\n",frame_num,reqframe.destinationAddress);
			// After every retry, double the time limit.
			time_limit*=2;
		    }
		}
		else if(n<0){
		    err_sys("read error");		
		}
		else if(n==0){
		    Fclose(fp);
        	    Fclose(ofp);
        	    close(sockfd);
		    err_sys("Server Terminated Pre-maturely");
		}
	        }
		rq_frame_num++; 
	    }
	    else if(recvline[0]=='W'){
		// Receive n frames from CSP(n is from input file).
	        wait_for=recvline[19]-'0';
	        for(i=0;i<wait_for;i++){
	    	    if (n=Read(sockfd,&dframe, sizeof(dframe))>0){
                        fprintf(ofp,"Receive (via CSP) data frame %d from SP %d\n",dframe.seqno,dframe.sourceAddress);
		    }
	    	}
	    }
        }
	// Close Files
	Fclose(fp);
	Fclose(ofp);
	close(sockfd);

        exit(0);
}
