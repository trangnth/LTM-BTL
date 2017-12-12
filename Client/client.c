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

#define PORT 4000

int main(int argc, char **argv){
	int sockfd, i, uid;
	struct sockaddr_in servaddr;
	char username[50] = {0};
	

	if (sockfd = socket (AF_INET, SOCK_STREAM, 0) == -1){
		exit (EXIT_FAILURE);
	}
	bzero (&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port   = htons(PORT);

    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    if ((connect(sockfd, (struct sockfd *) &servaddr)) == -1){
    	exit(EXIT_FAILURE);
    }

    fflush (stdin);
    printf ("Enter your name: ");
    fgets (username, 50, stdin);
    username[strcspn (username, "\n")] = 0;
    write (socfd, username, sizeof (username));
    read (socfd, &uid, sizeof (int));

    printf ("\nList user online: ");
    for (i = 0; i <= uid; i++){
    	char userOnline[50] = {0};
    	read (sockfd, userOnline, sizeof (userOnline));
    	printf ("\n%d. %s", i + 1, userOnline);
    }

	return 0;
}