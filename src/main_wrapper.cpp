// main wrapper

// #define _GNU_SOURCE
// #include <sched.h>
#include <cstddef>

#include "platforms.hpp"

#ifdef __x86_64__ARM__
	#include <pthread.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <stdio.h>
	#include <unistd.h>
	#include <signal.h>
	#include <string.h>
	#include <stdlib.h>
	#include <errno.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <sys/socket.h>
	#include "the_gasnet_globals.hpp"
    #include "the_gasnet_core.hpp"
    
	
	
	extern gasnet_node_routing_table_t *gasnet_node_routing_table;
	extern gasnet_ip_host_t *gasnet_ip_hosts;
	extern unsigned int gasnet_ip_host_cnt;
	extern unsigned int gasnet_number_cpus;
	
	typedef enum {by_ip,by_fpga} gasnet_routing_path;
	typedef enum {no,yes} gasnet_ip_loopback;
	
	typedef struct gasnet_host_str{
			unsigned int ip_addr;
			
			int threads_first_id;
			int threads_last_id;
			gasnet_routing_path threads_reach;
			gasnet_ip_loopback  threads_ip_loopback;
			
			int fpga_first_id;
			int fpga_last_id;
			gasnet_routing_path fpga_reach;
	} gasnet_host_t;

	typedef struct gasnet_app_args_str {
			int argc;
			char ** argv;		
	} gasnet_app_args_t;

	extern "C" void __real_main (int argc, char** argv);

	static std::thread* app_threads;
	static void* gasnet_app_thread(gasnet_app_args_t& data);

static unsigned int check_cpus()
{
	cpu_set_t *cpusetp;
	size_t size;
	int largest_affine_cpu = -1;
	int number_affine_cpus = 0;
	unsigned int MAX_CPU_NUMBER = 16;
	
	cpusetp = CPU_ALLOC(MAX_CPU_NUMBER);
	size = CPU_ALLOC_SIZE(MAX_CPU_NUMBER);
	pthread_getaffinity_np(pthread_self(), size, cpusetp);
	
	int cindex;
	for (cindex=0;cindex<MAX_CPU_NUMBER;cindex++)
		if (CPU_ISSET_S(cindex,size,cpusetp))
		{
			number_affine_cpus++;
			largest_affine_cpu = cindex;
		}
	
	if (!number_affine_cpus)
		return 0;
	if (number_affine_cpus!=(largest_affine_cpu+1))
		return 0;
	
	return number_affine_cpus;
}


extern "C" void __wrap_main (int argc, char** argv)
{
	int t,u;
	
	char* cfg_file_name = NULL;
	FILE *cfg_file;
	
	char line[256];

	struct in_addr ipaddr_buf;
	unsigned int my_ip = 0;
	unsigned int pin_memory = 1;
	
	if (!(gasnet_number_cpus = check_cpus()))
	{
		fprintf(stderr,"Limited or no CPU affinity\r\n");
		return;
	}
	
    #ifdef SET_CPU_AFFINITY
	print_cpu_affinity("main() wrapper CPU affinity",0);
    #endif
	
	// parse arguments
	for (t=1;t<argc;t++)
	{
		int remove_arg = 0;
		
		// pore over arguments; use the ones relevant here; for those, set remove_arg=1
		if (strncmp("--tgn_",argv[t],6)==0) // is a THeGASNet argument
		{
			remove_arg = 1;
			
			if (strncmp("--tgn_myip=",argv[t],11)==0)
			{
				if(!inet_aton(&argv[t][11], &ipaddr_buf))
				{
					fprintf(stderr,"Error: Invalid IP address.\r\n");
					exit(1);
				}
				else
				{
					my_ip = ipaddr_buf.s_addr;
					//printf("My IP stored as 0x%8X\r\n",my_ip);
				}
			}
				
			if (strncmp("--tgn_config=",argv[t],13)==0)
			{
				cfg_file_name = (char*) malloc(strlen(argv[t])-13+1);
				strcpy(cfg_file_name,&argv[t][13]);
				//printf("Config file path: %s\r\n",cfg_file_name);
			}

			if (strcmp("--tgn_nopin",argv[t])==0)
			{
				pin_memory = 0;
				//printf("Memory pinning disabled.\r\n");
			}
		}
		
		if (remove_arg)
		{
			for(u=t ; u<(argc-1) ; u++) // copy remaining arguments one down
				argv[u] = argv[u+1];
			t--;    // repeat this loop iteration - new argument in this position
			argc--; // reduce number of arguments
		}
	}
	
	if(!my_ip)
	{
		fprintf(stderr,"Error: No IP address specified.\r\n");
		exit(1);
	}
	
	if(!cfg_file_name)
	{
		fprintf(stderr,"Error: No configuration file path specified.\r\n");
		exit(1);
	}
	
	if((cfg_file=fopen(cfg_file_name, "r"))==NULL)
	{
		fprintf(stderr,"Error: Specified configuration file path could not be opened.\r\n");
		exit(1);
	}

	int my_host = -1;
	int host_count = 0;
	int max_node = -1;
	int num_local_threads = -1;
	int num_local_fpga = -1;
	gasnet_host_t *hosts;
	char* tokens[14];
	
	{	// TODO write more flexible parser
		while (!feof(cfg_file))
		{
			
			fgets(line, 256, cfg_file);

			// break line into tokens
			tokens[0] = strtok(line," \t\r\n");
			for(t=1;t<14;t++)
			{
				if (tokens[t-1]==NULL) break;
				
				tokens[t] = strtok(NULL," \t\r\n");
				// printf("Token %d: %s", t, tokens[t]);
			}

			if ((tokens[0]==NULL) || (strcmp("HOST",tokens[0])!=0))
				continue; // not a valid host line; next line
			
			host_count++;
			
			if(host_count==1)
				hosts = (gasnet_host_t*) malloc(sizeof(gasnet_host_t));
			else
				hosts = (gasnet_host_t*) realloc(hosts, sizeof(gasnet_host_t)*(host_count));

			hosts[host_count-1] = (gasnet_host_t) { 0,    -1, -1, by_ip, yes,    1, 1, by_fpga     };
				
			if(!inet_aton(tokens[1], &ipaddr_buf)) // MODIFIED
			// if(!inet_pton(AF_INET, tokens[1], &ipaddr_buf))
			{
				fprintf(stderr,"Error: Invalid IP address in host description.\r\n");
				exit(1);
			}
			else
				hosts[host_count-1].ip_addr = ipaddr_buf.s_addr;

			int continue_while = 0;
			for(t=2;t<14;t++)
			{
				if (tokens[t]==NULL || t==14) { continue_while = 1; break; }
				
				if (strcmp("THREADS",tokens[t])==0)
				{
					t++; if (tokens[t]==NULL || t==14) { continue_while = 1; break; }
					hosts[host_count-1].threads_first_id = atoi(tokens[t]);

					t++; if (tokens[t]==NULL || t==14) { continue_while = 1; break; }
					hosts[host_count-1].threads_last_id = atoi(tokens[t]);

					t++; if (tokens[t]==NULL || t==14) { continue_while = 1; break; }
					if (strcmp("BY_IP",tokens[t])==0)
						hosts[host_count-1].threads_reach = by_ip; // only relevant for FPGA internal routing tables (FPGA->thread)
					else
						hosts[host_count-1].threads_reach = by_fpga; // only relevant for FPGA internal routing tables (FPGA->thread)

					t++; if (tokens[t]==NULL || t==14) { continue_while = 1; break; }
					if (strcmp("IP_LOOPBACK",tokens[t])==0)
						hosts[host_count-1].threads_ip_loopback = yes;
					else
						hosts[host_count-1].threads_ip_loopback = no;
				}
				else if (strcmp("FPGA",tokens[t])==0)
				{
					t++; if (tokens[t]==NULL || t==14) { continue_while = 1; break; }
					hosts[host_count-1].fpga_first_id = atoi(tokens[t]);
					
					t++; if (tokens[t]==NULL || t==14) { continue_while = 1; break; }
					hosts[host_count-1].fpga_last_id = atoi(tokens[t]);
					
					t++; if (tokens[t]==NULL || t==14) { continue_while = 1; break; }
					if (strcmp("BY_IP",tokens[t])==0)
						hosts[host_count-1].fpga_reach = by_ip;
					else
						hosts[host_count-1].fpga_reach = by_fpga;
				}
				else { continue_while = 1; break; }
			}
			if (continue_while) continue; // not a valid host line; next line
		}
		
		for(t=0;t<host_count;t++)
		{
			if (hosts[t].ip_addr==my_ip)
				my_host = t;
			
			if (hosts[t].threads_first_id > max_node) max_node = hosts[t].threads_first_id;
			if (hosts[t].threads_last_id > max_node) max_node = hosts[t].threads_last_id;
			if (hosts[t].fpga_first_id > max_node) max_node = hosts[t].fpga_first_id;
			if (hosts[t].fpga_last_id > max_node) max_node = hosts[t].fpga_last_id;
			
			if ( (hosts[t].threads_last_id < hosts[t].threads_first_id) || (hosts[t].fpga_last_id < hosts[t].fpga_first_id) )
			{
				fprintf(stderr,"Error: Reversed node id range found for host %d.%d.%d.%d\r\n"
																			,((unsigned char*)&hosts[t].ip_addr)[0]
																			,((unsigned char*)&hosts[t].ip_addr)[1]
																			,((unsigned char*)&hosts[t].ip_addr)[2]
																			,((unsigned char*)&hosts[t].ip_addr)[3]);
				exit(1);
			}
		}

		if(my_host==-1)
		{
			fprintf(stderr,"Error: Local IP not listed in config file.\r\n");
			exit(1);
		}

		if(max_node==-1)
		{
			fprintf(stderr,"Error: No legal node id (>=0) found in config file.\r\n");
			exit(1);
		}
	}
	
	fclose(cfg_file);
	
	// BUILD ROUTING TABLE
	gasnet_node_routing_table = (gasnet_node_routing_table_t*)calloc(max_node+1,sizeof(gasnet_node_routing_table_t));
	gasnet_ip_hosts = (gasnet_ip_host_t*) calloc(host_count,sizeof(gasnet_ip_host_t)); // if no IP loopback this might be one bigger than needed
	gasnet_ip_host_cnt = 0;
	
	for(t=0;t<host_count;t++)
	{
		if (t==my_host)
		{
			if (hosts[t].threads_first_id >= 0)
				num_local_threads = hosts[t].threads_last_id - hosts[t].threads_first_id + 1;
			else
				num_local_threads = 0;

			if (hosts[t].fpga_first_id >= 0)
				num_local_fpga = hosts[t].fpga_last_id - hosts[t].fpga_first_id + 1;
			else
				num_local_fpga = 0;
			
			if (hosts[t].threads_first_id > -1)
				for (u=hosts[t].threads_first_id ; u<=hosts[t].threads_last_id ; u++)
				{
					if (hosts[t].threads_ip_loopback==yes)
					{
						if (u==hosts[t].threads_first_id) // once per host
						{
							gasnet_ip_hosts[gasnet_ip_host_cnt].ip4addr = 0x0100007F; // 127.0.0.1
							// pthread_mutex_init(&gasnet_ip_hosts[gasnet_ip_host_cnt].IPsend_mutex, NULL);
							gasnet_ip_host_cnt++;
						}
						gasnet_node_routing_table[u].type = ip; // local threads via IP loopback
						gasnet_node_routing_table[u].index = gasnet_ip_host_cnt - 1;
					}
					else
						gasnet_node_routing_table[u].type = thread; // local threads
				}
			if (hosts[t].fpga_first_id > -1)
				for (u=hosts[t].fpga_first_id ; u<=hosts[t].fpga_last_id ; u++)
				{
					gasnet_node_routing_table[u].type = fpga; // local fpga components
				}
		}
		else
		{
			if (hosts[t].threads_first_id > -1)
				for (u=hosts[t].threads_first_id ; u<=hosts[t].threads_last_id ; u++)
				{
					// For now, "by_fpga" only done if it starts from FPGA node
					//if (hosts[t].threads_reach==by_ip)
					{
						if (u==hosts[t].threads_first_id) // once per host
						{
							gasnet_ip_hosts[gasnet_ip_host_cnt].ip4addr = hosts[t].ip_addr;
							// pthread_mutex_init(&gasnet_ip_hosts[gasnet_ip_host_cnt].IPsend_mutex, NULL);
							gasnet_ip_host_cnt++;
						}
						gasnet_node_routing_table[u].type = ip;
						gasnet_node_routing_table[u].index = gasnet_ip_host_cnt - 1;
					}	
						// TODO = hosts[t].ip_addr; // target host's IP address
					//else // by FPGA
						//gasnet_node_routing_table[u].type = fpga; // route through FPGA network - not done for thread-thread
				}
			if (hosts[t].fpga_first_id > -1)
				for (u=hosts[t].fpga_first_id ; u<=hosts[t].fpga_last_id ; u++)
				{
					if (hosts[t].fpga_reach==by_ip)
					{
						if ((u==hosts[t].fpga_first_id) && (hosts[t].threads_first_id<0)) // once per host (here: no threads)
						{
							gasnet_ip_hosts[gasnet_ip_host_cnt].ip4addr = hosts[t].ip_addr;
							// pthread_mutex_init(&gasnet_ip_hosts[gasnet_ip_host_cnt].IPsend_mutex, NULL);
							gasnet_ip_host_cnt++;
						}
						gasnet_node_routing_table[u].type = ip;
						gasnet_node_routing_table[u].index = gasnet_ip_host_cnt - 1;
					}
						// TODO = hosts[t].ip_addr; // target host's IP address - route through IP
					else
						gasnet_node_routing_table[u].type = fpga; // route through FPGA network
				}
		}
	}
	
	for(t=0;t<=max_node;t++)
		if (gasnet_node_routing_table[t].type==none)
		{
			fprintf(stderr,"Error: Node %d has no route.\r\n",t);
			exit(1);
		}
	

	// allocate application thread variables, argument structs
	// app_threads = (std::thread*) malloc(num_local_threads*sizeof(std::thread()));
	app_threads = new std::thread[num_local_threads];
	gasnet_app_args_t *appargs = (gasnet_app_args_t *)malloc(num_local_threads*sizeof(gasnet_app_args_t));
	
	// start threads
	for (t=hosts[my_host].threads_first_id ; t<=hosts[my_host].threads_last_id ; t++)
	{
		int thrnum = t - hosts[my_host].threads_first_id;
		
		// allocate argv pointer array and copy it
		appargs[thrnum].argv = (char **) malloc((argc+5)*sizeof(char*));
		for(u=0; u<argc; u++)
			appargs[thrnum].argv[u] = argv[u];
		
		appargs[thrnum].argc = argc;
		
		// allocate and write GASNET_NUM_NODES argument
		appargs[thrnum].argv[appargs[thrnum].argc] = (char*) malloc(sizeof(char)*30);
		sprintf(appargs[thrnum].argv[appargs[thrnum].argc],"--GASNET_NUM_NODES=%d",max_node+1);
		appargs[thrnum].argc++;
	
		// allocate and write GASNET_LOCAL_THREADS argument
		appargs[thrnum].argv[appargs[thrnum].argc] = (char*) malloc(sizeof(char)*30);
		sprintf(appargs[thrnum].argv[appargs[thrnum].argc],"--GASNET_LOCAL_THREADS=%d",num_local_threads);
		appargs[thrnum].argc++;

		// allocate and write GASNET_LOCAL_FPGA argument
		appargs[thrnum].argv[appargs[thrnum].argc] = (char*) malloc(sizeof(char)*30);
		sprintf(appargs[thrnum].argv[appargs[thrnum].argc],"--GASNET_LOCAL_FPGA=%d",num_local_fpga);
		appargs[thrnum].argc++;

		// allocate and write per-thread GASNET_NODE argument
		appargs[thrnum].argv[appargs[thrnum].argc] = (char*) malloc(sizeof(char)*30);
		sprintf(appargs[thrnum].argv[appargs[thrnum].argc],"--GASNET_NODE=%d",t);
		appargs[thrnum].argc++;
		
		// allocate and write per-thread GASNET_PIN_MEM argument
		if (pin_memory)
		{
			appargs[thrnum].argv[appargs[thrnum].argc] = (char*) malloc(sizeof(char)*30);
			sprintf(appargs[thrnum].argv[appargs[thrnum].argc],"--GASNET_PIN_MEM");
			appargs[thrnum].argc++;
		}
		
		// pthread_create (&app_threads[thrnum], NULL, (void*)&gasnet_app_thread, (void*)&appargs[thrnum]); // TODO change data
		// printf("Creating app thread\n");
		app_threads[thrnum] = std::thread(gasnet_app_thread, std::ref(appargs[thrnum]));
		// app_threads[thrnum].detach();
		// printf("Detached app thread\n");
	}
	
	// collect finished threads	 - TODO clean up all threads, clean application exit
	for (t=hosts[my_host].threads_first_id ; t<=hosts[my_host].threads_last_id ; t++){
		// pthread_join(app_threads[t-hosts[my_host].threads_first_id],NULL);
        app_threads[t-hosts[my_host].threads_first_id].join();
    }

	// sleep(5);
	// TODO verify all threads are finished before exiting for completeness
	printf("Joined all app threads\n");
}



static void* gasnet_app_thread(gasnet_app_args_t& data)
{
	gasnet_app_args_t* args = &data;
	
	__real_main(args->argc,args->argv);
	
	return NULL;
}

#else // __MICROBLAZE__
#include "string.h"
extern int __real_main (int argc, char** argv);

//#define ARG1 --X=64
//#define ARG2 --Y=64

int __wrap_main (int argc, char** argv)
{
	int argcount = 0;
	char *argvector[5];
	
	char *arg0 = "./elf_binary"; // program name becomes first command line argument
	argvector[argcount] = arg0;
	argcount++;

	#define STRFY1(x) #x
	#define STRFY2(x) STRFY1(x)
	
	#ifdef ARG1
		char *arg1 = STRFY2(ARG1);
		//arg1[strlen(arg1)-1]='\0';
		argvector[argcount] = arg1;
		argcount++;
		#ifdef ARG2
			char *arg2 = STRFY2(ARG2);
			//arg2[strlen(arg2)-1]='\0';
			argvector[argcount] = arg2;
			argcount++;
			#ifdef ARG3
				char *arg3 = STRFY2(ARG3);
				//arg3[strlen(arg3)-1]='\0';
				argvector[argcount] = arg3;
				argcount++;
				#ifdef ARG4
					char *arg4 = STRFY2(ARG4);
					//arg4[strlen(arg4)-1]='\0';
					argvector[argcount] = arg4;
					argcount++;
					#ifdef ARG5
						char *arg5 = STRFY2(ARG5);
						//arg5[strlen(arg5)-1]='\0';
						argvector[argcount] = arg5;
						argcount++;
					#endif
				#endif
			#endif
		#endif
	#endif

	return __real_main(argcount,argvector);
}
#endif
