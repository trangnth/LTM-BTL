 /* Client
 * Cac chuc nang:
 	1. Cho phep nguoi dung noi chuyen truc tiep
	2. Cho phep nguoi dung noi chuyen theo nhom
	3. Cho phep nguoi dung gui file cho nhau
	4. Cho phep nguoi dung gui file theo nhom

 * Cac thanh vien:
	1. Nguyen Thi Huyen Trang
	2. Le Quynh Trang
	3. Hoang Van Thanh
 * Note: Su dung mutithread de vua doc vua ghi mess
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

#define MAXTOPIC 5
#define PORT 4000

char username[50] = {0};

//thread write
static void *writemsg (void *arg){
    int sockfd = *((int *)arg);
    while(1){
        char msg[1024] = {0}, sendmsg[1024] = {0}, recvmsg[1024] = {0};
        fflush (stdin);
        fgets (msg, 1024, stdin);
	msg[strcspn(msg,"\n")] = 0;
//printf ("%s", msg);
        if (strcmp (msg, "@") == 0){ //Finish chat
            write (sockfd, msg, sizeof (msg));
            break;
        }
        if (msg[0] !=  '!'){ //chat group
            strcat (sendmsg, "Topic. ");
            strcat (sendmsg, username);
            strcat (sendmsg, ": ");
            strcat (sendmsg, msg);
		write (sockfd, sendmsg, sizeof (msg));
        }else
        	write (sockfd, msg, sizeof (msg)); //chat user
    }
}

//thread read
static void *readmsg (void *arg){
    int sockfd = *((int *)arg);
    char buff [1024] = {0};
	printf ("\n");
    while (read (sockfd, buff, sizeof(buff))> 0){
        printf ("%s\n", buff);
        //buff = {0};
    }
}


int main(int argc, char **argv){
	int *sockfd, i, j, uid, topic;
	struct sockaddr_in servaddr;
	//char username[50] = {0};
    pthread_t w_tid, r_tid;
	sockfd = malloc (sizeof (int));	

	if ((*sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));
		exit (EXIT_FAILURE);
	}
	bzero (&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(PORT);

    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    if ((connect(*sockfd, (struct sockaddr *) &servaddr, sizeof (servaddr))) == 1){
	fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));    
	exit(EXIT_FAILURE);
    }

    //recv list topic
    printf ("\nList user online: ");
    for (i = 0; i < MAXTOPIC; i++){
        char userTopic[500] = {0};
        read (*sockfd, userTopic, sizeof(userTopic));
        printf ("\n%d. %s", i, userTopic);
    }

    printf("\nChose topic that you want to subcribe: ");
    scanf ("%d", &topic);
//topic = 1;
    write (*sockfd, &topic, sizeof(int));

//    fflush (stdin);
    printf ("\nEnter your name: ");
//sleep(1);
int ch; while((ch=getchar())!='\n'&&ch!=EOF);
//	fflush (stdin);
//	scanf ("%s", username);
    fgets (username, sizeof(username), stdin);
    username[strcspn (username, "\n")] = 0;
//printf("\n%s\n", username);
    write (*sockfd, username, sizeof (username));
    printf ("\nWelcome to group %d! (Enter your message)", topic);
    printf ("\nEnter \"!username: message\" to chat with user or \"@\" to finish.\n");

    //Start chat
    pthread_create (&w_tid, NULL, &writemsg, (void *) sockfd);
    pthread_create (&r_tid, NULL, &readmsg, (void *) sockfd);
	pthread_join(w_tid, NULL);
	pthread_join(r_tid, NULL);

    printf ("\nFinish chat.\n");
    close (*sockfd);
	free (sockfd);
	return 0;
}
