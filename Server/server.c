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

#define MAXUSER 100;
#define MAXTOPIC 5;
#define PORT 4000;

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
	user[uid].sockfd = sockfd
 
 //        user[uid].userFlag = 1;
 	printf ("\nname: %s, sockfd: %d", user[uid].name, user[uid].sockfd);
	//send list user online
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

int main (int argc, char **argv){
	int listenfd;
	socklen_t clien;
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
		iptr = malloc (sizeof(int))
		*iptr = accept (listenfd, (struct sockaddr *) &cliaddr, &clilen);

		char *addr;
		addr = inet_ntoa (cliaddr.sin_addr);
		printf ("\nOne client %s:%d connected.", addr, cliaddr.sin_port);
		pthread_create (&tid, NULL, doit, (void*) iptr);		
	}

	free (iptr);
	return 0;
}
