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

//int curUser = 0; //current user

void sendFile (int connfd, char file_name[256]) {
	int file_size;
	int remain_data;
	struct stat st;
	char buffer[1024];
  char *ptr;
    ptr = strtok(file_name, "#");
		printf("filename %s\n", ptr);
		FILE *fs = fopen(ptr, "rb");
		if(fs == NULL) {
			printf("ERROR: File %s not found on server.\n", ptr);
		}
		else {
			stat(ptr, &st);
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


void receiveFile (int sockfd, char file_name[256]) {

	char buffer[1024];
	int file_size;
			file_name[strcspn(file_name, "\n")]=0;
			char *ptr;
			ptr = strtok(file_name, "$");
			read(sockfd, &file_size, sizeof(int));
			printf("file_size la %d\n", file_size);

		if(file_size == -1) {
			printf("File not exists\n");
		}
		else {
			FILE *fr = fopen(ptr, "w");
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

char *stringCat(char *ptr1, char textNoti[1024]) {
	char *ptr2, *ptr3;
	ptr2 = &textNoti[0];
	int n = strlen(ptr1);
  int m = strlen(textNoti);
	int i;
	for (i = 0; i < n; i++)
      {
          *(ptr3 + i)  = *(ptr1 + i);

      }

      for (i = 0; i < m; i++)
      {
          *(ptr3 + i + n)  = *(ptr2 + i);

      }
			return ptr3;
}

static void *chat (void *arg){
	int sockfd, uid;

	sockfd = *((int *) arg);
	pthread_detach (pthread_self ());

	pthread_mutex_lock (&curUser_mutex);

		int i, j;
		//send list topic
		for (i = 0; i < MAXTOPIC; i++){
			char userTopic[500] = {0};
				for (j = 0; j < topic[i].curUser; j++){
					strcat (userTopic, topic[i].user[j].username);
					strcat (userTopic, " ");
				}
				write (sockfd, userTopic, sizeof (userTopic));
		}
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
	read (sockfd, topic[uTopic].user[uid].username, sizeof (topic[uTopic].user[uid].username));
	topic[uTopic].user[uid].sockfd = sockfd;


 	//recvice message
 	while(1){
	 	char recvmsg[1024] = {0}, *msg, sendmsg[1024] = {0};
	 	read (sockfd, recvmsg, sizeof(recvmsg));

	 	if (strcmp(recvmsg, "@") == 0)
	 		break;

	 	char buff[1024], *str;
		strcpy (buff, recvmsg);
                str = buff;
	 	if (str[0] == '!'){
	 		str ++;
	 		strtok (str, ":"); //lay user => str vaf buff = '!username'
	 		msg = strstr(recvmsg, ":");//lay mess tu ':' tro di
	 		strcat(sendmsg, topic[uTopic].user[uid].username);
	 		strcat(sendmsg, msg);
//	 		pthread_mutex_lock(&curUser_mutex);
	 		for (i = 0; i < MAXTOPIC; i++)
	 			for (j = 0; j < topic[i].curUser; j++)
	 				if (strcmp(topic[i].user[j].username, str) == 0)
	 					write (topic[i].user[j].sockfd, sendmsg, sizeof(sendmsg));
//	 		pthread_mutex_unlock(&curUser_mutex);

	 	} else {
			// Nhan FIle tu Client
			if(str[0] == '$') {
				receiveFile(sockfd, recvmsg);
				char *ptr;
			  ptr = strtok(str, "$");
				char noti[1024] = "Has file uploaded. To download, Enter #";
				char *s3 = strcat(noti, ptr);
				char sendNoti[1024];
				strncpy(sendNoti, s3, sizeof(sendNoti)-1);
				// strncpy is not adding a \0 at the end of the string after copying it so you need to add it by yourself
				sendNoti[sizeof(sendNoti)-1] = '\0';
				for (i = 0; i < topic[uTopic].curUser; i++){
		 			if (i == uid) continue;
			//		fprintf (stdout, "\n\'%d:%s\'", topic[uTopic].user[i].sockfd, topic[uTopic].user[i].username);
		 			write (topic[uTopic].user[i].sockfd, sendNoti, sizeof(sendNoti));
				}
			}
			else if(str[0] == '#'){
				write (sockfd, recvmsg, sizeof (recvmsg));
				sendFile(sockfd, recvmsg);
			}
			 else {
				//	 		pthread_mutex_lock(&curUser_mutex);
					 		for (i = 0; i < topic[uTopic].curUser; i++){
					 			if (i == uid) continue;
						//		fprintf (stdout, "\n\'%d:%s\'", topic[uTopic].user[i].sockfd, topic[uTopic].user[i].username);
					 			write (topic[uTopic].user[i].sockfd, recvmsg, sizeof(recvmsg));
							}
				//	 		pthread_mutex_unlock(&curUser_mutex);
			}
	 	}
	}

 	printf("\n%s  finish chat.\n", topic[uTopic].user[uid].username);
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



// static void *chat (void *arg){
// 	int sockfd, uid;
//
// 	sockfd = *((int *) arg);
// 	pthread_detach (pthread_self ());
//
// 	pthread_mutex_lock (&curUser_mutex);
// 		//uid = curUser;
// 		//curUser ++;
// 		//printf ("\n%d user online", curUser);
//
// 		int i, j;
// 		//send list topic
// 		//write (sockfd, &uid, sizeof (int)); //send current user online
// 		for (i = 0; i < MAXTOPIC; i++){
// 			char userTopic[500] = {0};
// 			//write (sockfd, &topic[i].n_user, sizeof(int));
// 			for (j = 0; j < topic[i].n_user; j++){
// 				strcat (userTopic, topic[i].user.username);
// 				strcat (userTopic, " ");
// 			}
// 			write (sockfd, userTopic, sizeof (userTopic));
// 		}
//
// 		//recv topic from client
// 		int uTopic;
// 		read (sockfd, &uTopic, sizeof (int));
// 		uid = topic[uTopic].curUser;
// 		topic[tmp].curUser ++;
// 	pthread_mutex_unlock (&curUser_mutex);
//
// 	//recv username client
// 	read (sockfd, topic.user[uid].username, sizeof (topic.user[uid].username));
// 	topic.user[uid].sockfd = sockfd;
//
//  	printf ("\nName: %s, sockfd: %d", topic.user[uid].username, topic.user[uid].sockfd);
//
//  	//recvice message
//  	while(1){
// 	 	char recvmsg[1024] = {0}, msg[1024] = {0};
// 	 	read (sockfd, recvmsg, sizeof(recvmsg));
//
// 	 	if (strcmp(recvmsg, "@") == 0)
// 	 		break;
// 	 	int *str = recvmsg;
// 	 	if (strcmp (str[0], '!') == 0){
// 	 		str ++;
//
// 	 	}
// 	}
//
// 	//for(;;){
//  //                printf("\n%s: ", user[uid].name);
//  //                char mess[1000] = {0};
//
//  //                read(connfd, mess, 1000);
//  //                if (strcmp(mess, "@") == 0)
//  //                        break;
//  //                printf (" %s", mess);
//  //                char buff[1052] = {0};
//  //                strcat (buff, user[uid].name);
//  //                strcat (buff, ": ");
//  //                strcat (buff, mess);
//  //                strcat (buff, " ");
//  //                for(i = 0; i < n_user; i++){
//  //                        if (i == uid) continue;
//  //                        write(user[i].sockfd, buff, strlen(buff)); //strlen thay cho sizeof
//  //                }
//  //        }
//  	printf("\n%s  finish chat.\n", topic[uTopic].user[uid].name);
//     pthread_mutex_lock(&curUser_mutex);
//         for (i = uid; i < topic[uTopic].curUser - 1; i++){
//             memset(topic[uTopic].user[i].name, '0', 50);
//             strcpy(topic[uTopic].user[i].name, topic[uTopic].user[i + 1].name);
//             user[i].sockfd = user[i + 1].sockfd;
//         }
//         //user[n_user - 1].userFlag = 0;
//         topic[uTopic].curUser --;
//     pthread_mutex_unlock(&curUser_mutex);
//
//         free(arg);
// // //      pthread_detach(pthread_self());
//
//     close (sockfd);
//     return (NULL);
//
// }



int main (int argc, char **argv){
	int listenfd;
	socklen_t clilen;
	int *iptr, i;
	pthread_t tid;
//	pid_t childpid;
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
