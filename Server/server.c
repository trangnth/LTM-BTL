/*
 * Server
 * Cac chuc nang:
 	1. Cho phep nguoi dung noi chuyen truc tiep
	2. Cho phep nguoi dung noi chuyen theo nhom
	3. Cho phep nguoi dung gui file cho nhau
	4. Cho phep nguoi dung gui file theo nhom

 * Mo ta phan chat nhom va chat rieng: 
 	-> clinet login: nhap username va chon topic minh muon 
 	-> thong bao cho server biet
 	-> server luu thong tin cho tung user
 	-> user chon muon chat rieng hay nhom, neu rieng thi chon user muon chat
 	-> user nhap mess, nhan '@' de chuyen che do chat hoac ket thuc chat 
 	-> server la nguoi trung gian, nhan va chuyen mess toi dung nguoi nhan

 * Cac thanh vien:
	1. Nguyen Thi Huyen Trang
	2. Le Quynh Trang
	3. Hoang Van Thanh
*/

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/stat.h>

#define MAXUSER 10
#define MAXTOPIC 5
#define PORT 4000

struct User
{
	char username[50];
	//int uid;
	//int topic;
	int sockfd;
	//int typeUser; //subcriber (1) or user (0) = chat with user or group
};

struct Topic
{
	struct User user[MAXUSER];
	int curUser;
};

struct Topic topic[MAXTOPIC];

pthread_mutex_t curUser_mutex = PTHREAD_MUTEX_INITIALIZER;

//int curUser = 0; //current user

static void *chat (void *arg){
	int sockfd, uid;

	sockfd = *((int *) arg);
	pthread_detach (pthread_self ());

	pthread_mutex_lock (&curUser_mutex);
		
		int i, j;
		//send list topic
		//write (sockfd, &uid, sizeof (int)); //send current user online
		for (i = 0; i < MAXTOPIC; i++){
			char userTopic[500] = {0};
			//write (sockfd, &topic[i].n_user, sizeof(int));
			for (j = 0; j < topic[i].n_user; j++){
				strcat (userTopic, topic[i].user.username);
				strcat (userTopic, " ");
			}
			write (sockfd, userTopic, sizeof (userTopic));
		}

		//recv topic from client
		int uTopic;
		read (sockfd, &uTopic, sizeof (int));
		uid = topic[uTopic].curUser;
		topic[tmp].curUser ++;
	pthread_mutex_unlock (&curUser_mutex);

	//recv username client
	read (sockfd, topic.user[uid].username, sizeof (topic.user[uid].username));
	topic.user[uid].sockfd = sockfd;

 	printf ("\nName: %s, sockfd: %d", topic.user[uid].username, topic.user[uid].sockfd);

 	//recvice message
 	while(1){
	 	char recvmsg[1024] = {0}, msg[1024] = {0}, sendmsg[1024] = {0};
	 	read (sockfd, recvmsg, sizeof(recvmsg));

	 	if (strcmp(recvmsg, "@") == 0) 
	 		break;
	 	
	 	char buff[1024], *str;

	 	if (strcmp (str[0], "!") == 0){
	 		strcmy (buff, recvmsg);
	 		str = buff;
	 		str ++;
	 		strtok (str, ":"); //lay user => str
	 		msg = strstr(name, ":");//lay mess tu ':' tro di
	 		strcat(sendmsg, topic[uTopic].user[uid].username);
	 		strcat(sendmsg, msg);

	 		pthread_mutex_lock(&curUser_mutex);
	 		for (i = 0; i < MAXTOPIC; i++)
	 			for (j = 0; j < topic[i].curUser; j++)
	 				if (strcmp(topic[i].user[j].username, str) == 0)
	 					write (topic[i].user[j].sockfd, sendmsg, sizeof(sendmsg));
	 		pthread_mutex_unlock(&curUser_mutex);

	 	}else{
	 		pthread_mutex_lock(&curUser_mutex);
	 		for (i = 0; i < topic[uTopic].curUser; i++)
	 			if (i == uid) continue;
	 			write (topic[uTopic].user[i].sockfd, recvmsg, sizeof(recvmsg));
	 		pthread_mutex_unlock(&curUser_mutex);
	 	}

	}

 	printf("\n%s  finish chat.\n", topic[uTopic].user[uid].name);
    pthread_mutex_lock(&curUser_mutex);
        for (i = uid; i < topic[uTopic].curUser - 1; i++){
            memset(topic[uTopic].user[i].name, '0', 50);
            strcpy(topic[uTopic].user[i].name, topic[uTopic].user[i + 1].name);
            user[i].sockfd = user[i + 1].sockfd;
        }
        //user[n_user - 1].userFlag = 0;
        topic[uTopic].curUser --;
    pthread_mutex_unlock(&curUser_mutex);

    free(arg);

    close (sockfd);
    return (NULL);

}

void sendFile (int connfd) {
	int file_size;
	char file_name[256];
	int remain_data;
	struct stat st;
	char buffer[1024];

	while(1) {
		int n = read(connfd, file_name, sizeof(file_name));
		file_name[n] = '\0';
		if(strcmp(file_name, "@") == 0) {
			file_size = -1;
			write(connfd, &file_size, sizeof(int));
			break;
		}
		FILE *fs = fopen(file_name, "rb");
		if(fs == NULL) {
			printf("ERROR: File %s not found on server.\n", file_name);
			file_size = -1;
			write(connfd, &file_size, sizeof(int));
			break;
		}
		else {
			stat(file_name, &st);
			remain_data = st.st_size;
			file_size = remain_data;
			printf("file_size la %d\n", file_size);
			write(connfd, &file_size, sizeof(int));
			while(remain_data > 0) {
				if(remain_data < 1024) {
					fread(buffer, remain_data, 1, fs);
					write(connfd, buffer, remain_data);
				}
				else {
					fread(buffer, 1024, 1, fs);
					write(connfd, buffer, 1024);
				}
				remain_data-=1024;
			}
		}
		fclose(fs);
		printf("Send File Success!\n");
 }
}

void receiveFile (int sockfd) {
	char buffer[1024];
	char file_name[256];
	int file_size;
	while(1) {
			printf("Nhap ten file (Enter \"@\" to quit): ");
			fgets(file_name, 256, stdin);
			file_name[strcspn(file_name, "\n")]=0;
			write(sockfd, file_name, strlen(file_name));
			if(strcmp(file_name, "@") == 0) {
				break;
			}
			read(sockfd, &file_size, sizeof(int));
			printf("file_size la %d\n", file_size);

		if(file_size == -1) {
			printf("File not exists\n");
			break;
		}
		else {
			FILE *fr = fopen(file_name, "w");
			printf("Downloading...\n");

			while(file_size > 0) {

				if(file_size < 1024) {
					read(sockfd, buffer, file_size);
					fwrite(buffer, file_size, 1,fr);
				}
				else {
					read(sockfd, buffer, 1024);
					fwrite(buffer, 1024, 1, fr);
				}
				file_size-=1024;
			}
			fclose(fr);

			printf("Download Success\n");
		}
	}
}

int main (int argc, char **argv){
	int listenfd;
	socklen_t clilen;
	int *iptr, i;
	pthread_t tid;
//	pid_t childpid;
	struct sockaddr_in cliaddr, servaddr;

	for (i = 0; i < MAXTOPIC; i++)
		topic[i].n_user = 0;

	listenfd = socket (AF_INET, SOCK_STREAM, 0);

	bzero (&servaddr, sizeof (servaddr));
	servaddr.sin_family  	 = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	servaddr.sin_port 		 = htonl (PORT);
	bind (listenfd, (struct sockaddr *) &servaddr, sizeof (servaddr));

	listen (listenfd, MAXUSER);
	for (;;){
		clilen = sizeof (cliaddr);
		iptr = malloc (sizeof(int));
		*iptr = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

		char *addr;
		addr = inet_ntoa (cliaddr.sin_addr);
		printf ("\nOne client %s:%d connected.", addr, cliaddr.sin_port);
		pthread_create (&tid, NULL, chat, (void*) iptr);
	}

	free (iptr);
	return 0;
}
