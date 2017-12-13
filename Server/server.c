/*
 * Server
 * Cac chuc nang:
 	1. Cho phep nguoi dung noi chuyen truc tiep
	2. Cho phep nguoi dung noi chuyen theo nhom
	3. Cho phep nguoi dung gui file cho nhau
	4. Cho phep nguoi dung gui file theo nhom

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
#define MAXUSER 100
#define MAXTOPIC 5
#define PORT 4000

struct User
{
	char username[50];
	int uid;
	int topic;
	int sockfd;
	//int typeUser; //subcriber (1) or user (0) = chat with user or group
};


struct User user[MAXUSER];
int topic[MAXTOPIC] = {1, 2, 3, 4, 5};

pthread_mutex_t curUser_mutex = PTHREAD_MUTEX_INITIALIZER;

int curUser = 0; //current user

static void *doit (void *arg){
	int sockfd, uid;

	sockfd = *((int *) arg);
	pthread_detach (pthread_seft ());

	pthread_mutex_lock (&curUser_mutex);
		uid = curUser;
		curUser ++;
		printf ("\n%d user online", curUser);
	pthread_mutex_unlock (&curUser_mutex);

	//recv username client
	read (sockfd, user [uid].username, sizeof (user[uid].username));
	write (sockfd, &uid, sizeof (int)); //send current user online
	user[uid].uid = uid;
	user[uid].sockfd = sockfd;

 //        user[uid].userFlag = 1;
 	printf ("\nname: %s, sockfd: %d", user[uid].username, user[uid].sockfd);
	//send list user online
	int i;
	for (i = 0; i <= uid; i++){
		write (sockfd, user[i].username, sizeof(user[i].username));
 //                sleep(1);
	}

	//for(;;){
 //                printf("\n%s: ", user[uid].name);
 //                char mess[1000] = {0};

 //                read(connfd, mess, 1000);
 //                if (strcmp(mess, "@") == 0)
 //                        break;
 //                printf (" %s", mess);
 //                char buff[1052] = {0};
 //                strcat (buff, user[uid].name);
 //                strcat (buff, ": ");
 //                strcat (buff, mess);
 //                strcat (buff, " ");
 //                for(i = 0; i < n_user; i++){
 //                        if (i == uid) continue;
 //                        write(user[i].sockfd, buff, strlen(buff)); //strlen thay cho sizeof
 //                }
 //        }
// 	printf("\n%s  close.\n", user[uid].name);
//         pthread_mutex_lock(&curUser_mutex);
//         for (i = uid; i < n_user - 1; i++){
//         //      memset(user[i].name, '0', 50);
//                 strcpy(user[i].name, user[i + 1].name);
//                 user[i].sockfd = user[i + 1].sockfd;
//         }
//         user[n_user - 1].userFlag = 0;
//         curUser --;
//         pthread_mutex_unlock(&curUser_mutex);

//         free(arg);
// //      pthread_detach(pthread_self());

        // close (socfd);
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
	int *iptr;
	pthread_t tid;
//	pid_t childpid;
	struct sockaddr_in cliaddr, servaddr;

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
		pthread_create (&tid, NULL, doit, (void*) iptr);
	}

	free (iptr);
	return 0;
}
