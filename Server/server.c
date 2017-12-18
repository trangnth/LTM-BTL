/*
 * Server
 * Cac chuc nang:
 	1. Cho phep nguoi dung noi chuyen truc tiep
	2. Cho phep nguoi dung noi chuyen theo nhom
	3. Cho phep nguoi dung gui file cho nhau
	4. Cho phep nguoi dung gui file theo nhom

 * Mo ta phan chat nhom va chat rieng:
 	-> clinet login: chon topic minh muon va nhap username
 	-> thong bao cho server biet
 	-> server luu thong tin cho tung user
 	-> mac dinh client se chat nhom, neu muon chat rieng thi nhap theo cu phap '!username: message'
 	-> user nhap mess, nhan '@' ket thuc chat
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
	int sockfd;
};

struct Topic
{
	struct User user[MAXUSER];
	int curUser;
};

struct Topic topic[MAXTOPIC];

pthread_mutex_t curUser_mutex = PTHREAD_MUTEX_INITIALIZER;

//<<<<<<< HEAD

//send list topic
void sendLtopic(int sockfd){
	int i, j;
	for (i = 0; i < MAXTOPIC; i++){
		char userTopic[500] = {0};
			for (j = 0; j < topic[i].curUser; j++){
				strcat (userTopic, topic[i].user[j].username);
				strcat (userTopic, " ");
			}
		write (sockfd, userTopic, sizeof (userTopic));
	}
}
//=======
//>>>>>> b489953ec601f611b85473ed7fdf99d5c2fcdb1f

void sendFile (int connfd, char file_name[256]) {
	int file_size;
	
	int remain_data;
	struct stat st;
	char buffer[1024];

	//while(1) {
		//int n = read(connfd, file_name, sizeof(file_name));
		//file_name[n] = '\0';
		//if(strcmp(file_name, "@") == 0) {
		//	file_size = -1;
		//	write(connfd, &file_size, sizeof(int));
		//	break;
		//}
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
			printf("File_size: %d\n", file_size);
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
	//}
}

void receiveFile (int sockfd, char file_name[256]) {
	char buffer[1024];
	int file_size;
			file_name[strcspn(file_name, "\n")]=0;
			read(sockfd, &file_size, sizeof(int));
			printf("file_size la %d\n", file_size);

		if(file_size == -1) {
			printf("File not exists\n");
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

//=======
//>>>>>>> a66337c99f8adf8abbb6cb855c4295da40bd8b9b
static void *chat (void *arg){
	int sockfd, uid;

	sockfd = *((int *) arg);
	pthread_detach (pthread_self ());

	pthread_mutex_lock (&curUser_mutex);

		int i, j;
	
		sendLtopic(sockfd);
		fprintf (stdout, "\n");

		//recv topic from client
		int uTopic;
		read (sockfd, &uTopic, sizeof (int));
		if (uTopic > 4 || uTopic < 0){
			close(sockfd);
			return NULL;
		}
		uid = topic[uTopic].curUser;	//id of user in topic
		topic[uTopic].curUser ++;
	pthread_mutex_unlock (&curUser_mutex);

	//recv username client
	while(1){
		int tmp = 0;
		read (sockfd, topic[uTopic].user[uid].username, sizeof (topic[uTopic].user[uid].username));
		for (i = 0; i < MAXTOPIC; i++)
	 		for (j = 0; j < topic[i].curUser - 1; j++)
	 			if (strcmp(topic[i].user[j].username, topic[uTopic].user[uid].username) == 0){
	 				tmp = 1;
	 			}
		write (sockfd, &tmp, sizeof(int));
		if (tmp == 0) break;
	}
	topic[uTopic].user[uid].sockfd = sockfd;

 	//recvice and send message
 	while(1){
	 	char recvmsg[1024] = {0}, *msg, sendmsg[1024] = {0};
	 	read (sockfd, recvmsg, sizeof(recvmsg));

	 	if (strcmp(recvmsg, "@") == 0) // user finish chat
	 		break;

	 	//Chat user
	 	char buff[1024], *str;
		strcpy (buff, recvmsg);
		str = buff;
	 	if (str [0] == '!'){
	 		str ++;
	 		strtok (str, ":");
	 		msg = strstr(recvmsg, ":");
			strcat(sendmsg, ">");
	 		strcat(sendmsg, topic[uTopic].user[uid].username);
	 		strcat(sendmsg, msg);

	 		for (i = 0; i < MAXTOPIC; i++)
	 			for (j = 0; j < topic[i].curUser; j++)
	 				if (strcmp(topic[i].user[j].username, str) == 0)
	 					write (topic[i].user[j].sockfd, sendmsg, sizeof(sendmsg));

//	 	}else if(strcmp (recvmsg, "@L") == 0){
//			sendLtopic(sockfd);
		}else{
			// Nhan File tu Client
			if(str[0] == '$') {
				printf("Download file %s\n", recvmsg);
				receiveFile(sockfd, recvmsg);
				for (i = 0; i < topic[uTopic].curUser; i++){
	 				if (i == uid) continue;
		 				sendFile(topic[uTopic].user[i].sockfd), recvmsg	
	 			}	
			}

			if (str[0] == '#'){
				printf("Download file %s\n", recvmsg);
				receiveFile(sockfd, recvmsg);
				char buf[1024], *p;
				strcpy (buf, recvmsg);
				p = buf;
				p ++;
				strtok (str, ":");
	 			msg = strstr(recvmsg, ":");
	 			for (i = 0; i < MAXTOPIC; i++)
	 			for (j = 0; j < topic[i].curUser; j++)
	 				if (strcmp(topic[i].user[j].username, str) == 0)
	 					sendFile(topic[i].user[j].sockfd, msg);
				
			}

	 		for (i = 0; i < topic[uTopic].curUser; i++){
	 			if (i == uid) continue;
	 			write (topic[uTopic].user[i].sockfd, recvmsg, sizeof(recvmsg));
//if (str[0] == '$')
//sendFile(topic[uTopic].user[i].sockfd, recvmsg);
			}
	 	}

	}

 	printf("\n%s finish chat.\n", topic[uTopic].user[uid].username);
    pthread_mutex_lock(&curUser_mutex);
        for (i = uid; i < topic[uTopic].curUser - 1; i++){
            memset(topic[uTopic].user[i].username, '0', 50);
            strcpy(topic[uTopic].user[i].username, topic[uTopic].user[i + 1].username);
            topic[uTopic].user[i].sockfd = topic[uTopic].user[i + 1].sockfd;
        }
        topic[uTopic].curUser --;
    pthread_mutex_unlock(&curUser_mutex);

    free(arg);

    close (sockfd);
    return (NULL);

}

int main (int argc, char **argv){
	int listenfd;
	socklen_t clilen;
	int *iptr, i;
	pthread_t tid;
	struct sockaddr_in cliaddr, servaddr;

	for (i = 0; i < MAXTOPIC; i++)
		topic[i].curUser = 0;

	listenfd = socket (AF_INET, SOCK_STREAM, 0);

	bzero (&servaddr, sizeof (servaddr));
	servaddr.sin_family  	 = AF_INET;
	servaddr.sin_addr.s_addr = htonl (INADDR_ANY);
	servaddr.sin_port 		 = htons (PORT);
	bind (listenfd, (struct sockaddr *) &servaddr, sizeof (servaddr));

	listen (listenfd, 10);
	for (;;){
		clilen = sizeof (cliaddr);
		iptr = malloc (sizeof(int));
		*iptr = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

		fprintf (stdout, "\nOne client %s:%d connected.", inet_ntoa (cliaddr.sin_addr), cliaddr.sin_port);
		pthread_create (&tid, NULL, &chat, (void*) iptr);
	}

	free (iptr);
	return 0;
}
