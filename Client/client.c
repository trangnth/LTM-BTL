/*
 * Client
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

int main(int argc, char **argv){
	int sockfd, i, j, uid, topic;
	struct sockaddr_in servaddr;
	char username[50] = {0};


	if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) == -1){
        fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));
		exit (EXIT_FAILURE);
	}
	bzero (&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(PORT);

    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    if ((connect(sockfd, (struct sockaddr *) &servaddr, sizeof (servaddr))) == 1){
	fprintf(stderr, "Error creating socket --> %s\n", strerror(errno));    
	exit(EXIT_FAILURE);
    }

    //recv list topic
    printf ("\nList user online: ");
    for (i = 0; i < MAXTOPIC; i++){
        char userTopic[500] = {0};
        read (sockfd, userTopic, sizeof(int));
        printf ("\n%d. %s", i, userTopic);
    }

    printf("\nChose topic that you want to subcribe: ");
    scanf ("%d", &topic);
    write (sockfd, &topic, sizeof(int));

    fflush (stdin);
    printf ("Enter your name: ");
    fgets (username, 50, stdin);
    username[strcspn (username, "\n")] = 0;
    write (sockfd, username, sizeof (username));
    printf ("\nWelcome to group! (Enter your message)");
    printf ("\nEnter \"!username: message\" to chat with user or \"@\" to finish.");

    //Start chat
    
    while(1){
        char msg[1024] = {0}, sendmsg[1024] = {0}, recvmsg[1024] = {0};
        fflush (stdin);
        fgets (msg, 1024, stdin);
        if (msg[0] !=  '!'){
            //strcat (sendmsg, topic);
            strcat (sendmsg, "Topic. ");
            strcat (sendmsg, username);
            strcat (sendmsg, ": ");
            strcat (sendmsg, msg);
        }
        if (strcmp (msg, "@") == 0){
            write (sockfd, msg, sizeof (msg));
            break;
        }
        write (sockfd, sendmsg, sizeof (sendmsg));
    }

    printf ("\nFinish chat.\n");
    close (sockfd);
	return 0;
}
