//Dennis Preston Beaty
//COSC 360
//Laba
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h> 
#include <unistd.h>
#include "sockettome.h"
#include "jrb.h"
#include "dllist.h"

//Struct Declarations
typedef struct Chat_Room {
	char *chat_name;
	Dllist c_list;
	Dllist m_list;
	pthread_cond_t *pthread_cond;
	pthread_mutex_t *pthread_mute;
}Chat_Room;

typedef struct Client_var {
	char *chat_name;
	int file;
	Dllist store_list;
	FILE *output;
	FILE *input;
	Chat_Room *c_room;
}Client_var;

typedef struct Chat_Server {
	Client_var *client;
	JRB room_tree;
}Chat_Server;

//Function Declarations
void *pthread_create_arg2(void *nothing) {
	//vars
	char buffer_1[100];
	char buffer_2[100];
	char *chat_1;
	char *chat_2;
	JRB temp;
	Dllist temp_list;
	Client_var *client;
	Chat_Room *chat_room;

	pthread_detach(pthread_self());
	//pull client and room from jrb tree
	JRB room_tree= (JRB)((Chat_Server*)nothing)->room_tree;
	client= (Client_var*)((Chat_Server*)nothing)->client;
	fprintf(client->output,"Chat Rooms:\n");
	jrb_traverse(temp,room_tree) {
		chat_room= ((Chat_Room*)temp->val.v);
		//print chat condition
		fprintf(client->output,"\n%s:",chat_room->chat_name);
		pthread_mutex_lock(chat_room->pthread_mute);
		printf("Locked mutex for room %s\n",chat_room->chat_name);
		dll_traverse(temp_list, chat_room->c_list) {
		//print list of users
		fprintf(client->output," %s",((Client_var*) temp_list->val.v)->chat_name);
		}
		pthread_mutex_unlock(chat_room->pthread_mute);
		printf("Unlocked mutex for room %s\n",chat_room->chat_name);
	}
	//ask for desired chat_name
	fprintf(client->output,"\n\nEnter your chat name (no spaces):\n");
	//reset & fetch
	fflush(client->output);
	fgets(buffer_2, 100, client->input);
	//add terminating character
	int ending= strlen(buffer_2)-1;
	buffer_2[ending]= '\0';
	//set chat_name
	client->chat_name= strdup(buffer_2);
	fprintf(client->output,"Enter chat room:\n");
	//reset & fetch
	fflush(client->output);
	fgets(buffer_2,100,client->input);
	//add terminating character
	buffer_2[strlen(buffer_2)-1]= '\0';
	temp= jrb_find_str(room_tree,buffer_2);
	//if room with chat_name doesnt exist until given room is found
	while(temp==NULL) {
		/* code */
		//continue to prompt if given null string
		fprintf(client->output,"No chat room %s\n",buffer_2);
		//reset & fetch
		fflush(client->output);
		fgets(buffer_2,100,client->input);
		//add terminating character
		ending= strlen(buffer_2)-1;
		buffer_2[ending]= '\0';
		temp= jrb_find_str(room_tree,buffer_2);
	}
	//set c_list room once room is found
	client->c_room= ((Chat_Room*)temp->val.v);
	chat_2= (char *)malloc(sizeof(char)*(strlen(client->chat_name)+14));
	sprintf(chat_2,"%s has joined\n",client->chat_name);
	//Finalize user joining the chat room
	pthread_mutex_lock(client->c_room->pthread_mute);
	printf("Locked mutex for room %s\n",chat_room->chat_name);
	dll_append(client->c_room->c_list,new_jval_v(client));
	dll_append(client->c_room->m_list,new_jval_s(chat_2));
	client->store_list= dll_last(client->c_room->c_list);
	pthread_mutex_unlock(client->c_room->pthread_mute);
	pthread_cond_signal(client->c_room->pthread_cond);
	printf("Unlocked mutex for room %s\n",chat_room->chat_name);
	//read chat_1 from user
	while(fgets(buffer_1,100,client->input)!=NULL && (feof(client->input)==0)) {
		/* code */
		//form the chat_1 char *
		chat_1= (char*)malloc(sizeof(char)*(strlen(buffer_1)+strlen(client->chat_name)+4));
		sprintf(chat_1,"%s: %s",client->chat_name,buffer_1);
		pthread_mutex_lock(client->c_room->pthread_mute);
		printf("Locked mutex for room %s\n",client->c_room->chat_name);
		printf("Message from client %s in room %s: %s\n",client->chat_name,client->c_room->chat_name,chat_1);
		printf("Appending to %s's message queue and signaling",client->c_room->chat_name);
		dll_append(client->c_room->m_list,new_jval_s(chat_1));
		pthread_mutex_unlock(client->c_room->pthread_mute);
		pthread_cond_signal(client->c_room->pthread_cond);
		printf("Unlocked mutex for room %s\n",client->c_room->chat_name);
	}
	Chat_Room *client_c_room= client->c_room;
	pthread_mutex_lock(client_c_room->pthread_mute);
	//print notifs
	printf("Locked mutex for room %s\n",client_c_room->chat_name);
	printf("Client_var %s has left\n",client->chat_name);
	chat_1= (char*)malloc(sizeof(char)*(strlen(client->chat_name)+11));
	sprintf(chat_1,"%s has left\n",client->chat_name);
	printf("Cleaning up after client\n");
	//free mem
	free(client->chat_name);
	dll_delete_node(client->store_list);
	fclose(client->input);
	fclose(client->output);
	close(client->file);
	free(client);
	//add user left chat_1
	dll_append(client_c_room->m_list,new_jval_s(chat_1));
	//prep
	pthread_mutex_unlock(client_c_room->pthread_mute);
	pthread_cond_signal(client_c_room->pthread_cond);
	printf("Unlocked mutex for room %s\n",client_c_room->chat_name);
	pthread_exit(NULL);
}

void *pthread_create_arg1(void *nothing) {
	//vars
	Dllist messages_list;
	Dllist clients_list;
	Chat_Room *temp_room= (Chat_Room *) nothing;
	printf("Locked mutex for room %s\n",temp_room->chat_name);
	pthread_mutex_lock(temp_room->pthread_mute);
	while(1) {
		/* code */
		//print m_list
		printf("Unlocked mutex for room %s\n",temp_room->chat_name);
		printf("Chat_Room %s blocking on condition variable\n",temp_room->chat_name);
		pthread_cond_wait(temp_room->pthread_cond,temp_room->pthread_mute);
		printf("Chat_Room %s signaled and active\n",temp_room->chat_name);
		printf("Locked mutex for room %s\n",temp_room->chat_name);
		while(!dll_empty(temp_room->m_list)) {
			/* code */
			messages_list= dll_first(temp_room->m_list);
			//send the chat_1 to all c_list
			printf("Sending message to all c_list: %s",jval_s(messages_list->val));
			dll_traverse(clients_list,temp_room->c_list) {
				//memory leak check
				int mem_check=0;
				if(fputs(jval_s(messages_list->val),((Client_var*) jval_v(clients_list->val))->output)<1 || fflush(((Client_var*) jval_v(clients_list->val))->output)==EOF) {
					/* code */
					mem_check= -1;
				}
				if(mem_check< 0) {
					/* code */
					//free mem
					free(((Client_var*) jval_v(clients_list->val))->chat_name);
					dll_delete_node(((Client_var*) jval_v(clients_list->val))->store_list);
					fclose(((Client_var*) jval_v(clients_list->val))->input);
					fclose(((Client_var*) jval_v(clients_list->val))->output);
					close(((Client_var*) jval_v(clients_list->val))->file);
					free(((Client_var*) jval_v(clients_list->val)));
				}
			}
			//free memory of the chat_1
			free(messages_list->val.s);
			dll_delete_node(messages_list);
		}
	}
}


//MAIN
int main(int argc, char **argv) {
	//vars
	int file;
	int connection;
	int argv_1;
	int argc_convert;
	JRB temp;
	JRB room_tree;
	pthread_t thread_room;
	pthread_t thread_client;
	Chat_Server chat_server;
	Chat_Room *chat_room;

	if(argc<3) {
		/* code */
		fprintf(stderr,"usage: ./chat-server port Chat-Chat_Room-Names ...\n");
		exit(1);
	}
	argv_1= atoi(argv[1]);
	if (argv_1<5000) {
		/* code */
		fprintf(stderr,"usage: ./chat-server port Chat-Chat_Room-Names ...\n");
		fprintf(stderr,"       port must be > 5000\n");
		exit(1);
	}
	room_tree= make_jrb();
	argc_convert= argc- 2;
	for(size_t i=0; i<argc_convert; i++) {
		/* code */
		//create each room
		chat_room= (Chat_Room*) malloc(sizeof(Chat_Room));
		//create lists
		chat_room->m_list= new_dllist();
		chat_room->c_list= new_dllist();
		//chat_name set
		chat_room->chat_name= argv[i+2];
		//insert into jrb tree
		jrb_insert_str(room_tree, chat_room->chat_name,new_jval_v((void *) chat_room));
	}
	jrb_traverse(temp, room_tree) {
		chat_room= (Chat_Room*) temp->val.v;
		chat_room->pthread_mute= (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
		chat_room->pthread_cond= (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
		pthread_mutex_init(chat_room->pthread_mute, NULL);
		pthread_create(&thread_room,NULL,pthread_create_arg1,((void*) temp->val.v));
	}
	printf("Serving socket %d...\n",argv_1);
	connection= serve_socket(argv_1);
	while(1) {
		/* code */
		printf("Waiting for client to connect...\n");
		//open client and accept connection
		file= accept_connection(connection);
		printf("Client_var connected! file is %d\n",file);
		chat_server.client= (Client_var*) malloc(sizeof(Client_var));
		chat_server.client->file= file;
		chat_server.client->input= fdopen(file,"r");
		chat_server.client->output= fdopen(file,"w");
		chat_server.room_tree= room_tree;
		pthread_create(&thread_client,NULL,pthread_create_arg2,&chat_server);
	}
	return 0;
}
