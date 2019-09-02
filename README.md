This Program is written using Berkeley C. I have not provided all the necessary library files needed to execute this project.

Important Files:
----------------
1. csp_server.c: Server process to simulate control switch in Fast Ethernet. Creates a log file Server_log.txt

2. sp_client.c: Client process to simulate a station in Fast Ethernet. This program takes ip address of the server(IPv4) format and an integer between 0-9 which is the id of the client.

3. frameStruct.c: This file provides the structure of the frame to be transmitted back and forth betweeen CSP and SP.

4. sim_input_<n>.txt: Input for nth SP. n must be between 0-9. 
Note:SP Ids begin from 0 to 9(not from 1-10).

5. activity_log_<n>.txt: This file will be created by nth SP. This is the log of activities performed byy that SP.

6. demo.mk: Make file to compile csp_server.c and sp_client.c 

7. Server_log.txt: This file will be created by the CSP. All the activities performed by the server.

8. There are several header files and .c files in lib folder on which this application has dependencies. So, please make sure none of those files in lib are deleted.

How to execute the application:
-------------------------------
- Compile csp_server.c and sp_client.c using the below command.
			> make -f demo.mk csp_server
			> make -f demo.mk sp_client
- In any of the university Linux machines, before running the CSP, get the ip address of the machine using the command:
			> ifconfig
  Get the INET under Broadcast which is the IPv4 address of the server process.
- Run the CSP using below command
			>./csp_server
- Before running SP processes make sure the server process is running. 
- Run the clients in differnet windows by providing the host IPv4 address of the server process and the client Id
			>./sp_client <server's IPv4 address> <client Id>
- Provide the client Id s in the order of running clients: For example when running the client for the first time, run it using ./sp_client <server's IPv4 address> 0.
For the next client run it using ./sp_client <server's IPv4 address> 1. Do not change the order of client ids.
-If you want to run the clients with differnt input files, please name them as sim_input_<n>.txt or copy the contents of your input file into sim_input_<n>.txt.
- Once the execution is done, client processes will be closed by themselves. Open the activity_log_<n>.txt file to see the activities performed by that SP.
- Similarly open the file Server_log.txt to see the activity log of CSP.


Note: > Represents shell prompt.
