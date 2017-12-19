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
#include <sys/stat.h>
#define MAXTOPIC 5
#define PORT 4000

char username[50] = {0};

//recv list topic
void recvLtopic(int sockfd){
	int i;
	printf ("\nList user online: ");
    for (i = 0; i < MAXTOPIC; i++){
        char userTopic[500] = {0};
        read (sockfd, userTopic, sizeof(userTopic));
        printf ("\n%d. %s", i, userTopic);
   }
}

void sendFile (int connfd, char file_name[256]) {
	int file_size;
	int remain_data;
	struct stat st;
	char buffer[1024];
  char *ptr;
  ptr = strtok(file_name, "$");

	FILE *fs = fopen(ptr, "rb");
	if(fs == NULL) {
		printf("ERROR: File %s not found on local.\n", ptr);
		file_size = -1;
		write(connfd, &file_size, sizeof(int));
		fclose(fs);
	}
	else {
		stat(ptr, &st);
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
		fclose(fs);
		printf("Send File Success!\n");
	}
}

void receiveFile (int sockfd, char file_name[256]) {
	char buffer[1024];
	int file_size;
	file_name[strcspn(file_name, "\n")]=0;
	char *ptr;
	ptr = strtok(file_name, "&");
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


//thread write
static void *writemsg (void *arg){
  int sockfd = *((int *)arg);
  while(1){
      char msg[1024] = {0}, sendmsg[1024] = {0}, recvmsg[1024] = {0};
      fflush (stdin);
      fgets (msg, 1024, stdin);
      msg[strcspn(msg,"\n")] = 0;
      if (strcmp (msg, "@") == 0){ //Finish chat
          write (sockfd, msg, sizeof (msg));
          break;
      }
      //truyen file cho user
      if(msg[0] == '#'){
				char *msgFile;
      	write (sockfd, msg, sizeof (msg));
				msgFile = strstr(msg, ":");
				msgFile++;
      	sendFile(sockfd, msgFile);
      } else if(msg[0] == '$') {
				char *msgFile;
				write (sockfd, msg, sizeof (msg));
				msgFile = msg;
				msgFile++;
	      sendFile(sockfd, msgFile);
    } else if(msg[0] == '&') {
      	write (sockfd, msg, sizeof (msg));
		} else if (msg[0] !=  '!'){ //chat group
	      strcat (sendmsg, "G>");
	      strcat (sendmsg, username);
	      strcat (sendmsg, ": ");
	      strcat (sendmsg, msg);
	    	write (sockfd, sendmsg, sizeof (msg));
		} else {
				write (sockfd, msg, sizeof (msg)); //chat user
		}

  }
}

//thread read
static void *readmsg (void *arg){
    int sockfd = *((int *)arg);
    char buff [1024] = {0};
	printf ("\n");
    while (read (sockfd, buff, sizeof(buff))> 0){
			if(buff[0] == '&') { //Nhan file tu server
				char *msgFile = buff;
				msgFile = strtok(buff, "&");
				receiveFile(sockfd, msgFile);
			} else if(buff[0] == '#') {
				char *str = buff;
				char *str2;
				str2 = strstr(str, ":");
				str2++;
				receiveFile(sockfd, str2);
			} else {
				printf ("%s\n", buff);
        //buff = {0};
			}
    }
}


int main(int argc, char **argv){
	int *sockfd, i, j, uid, topic;
	struct sockaddr_in servaddr;
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

    recvLtopic (*sockfd);

    //send topic to server
    do{
      printf("\nChoose group that you want to subcribe: ");
      scanf ("%d", &topic);
		}while (topic > 4 || topic < 0);
    	write (*sockfd, &topic, sizeof(int));

    //send username to server
		int ch; while((ch=getchar())!='\n'&&ch!=EOF); //Lam sach bo dem sau scanf

    while(1){
		int tmp;
	    printf ("Enter your name: ");
//		int ch; while((ch=getchar())!='\n'&&ch!=EOF); //Lam sach bo dem sau scanf
	    fgets (username, sizeof(username), stdin);
	    username[strcspn (username, "\n")] = 0;
	    write (*sockfd, username, sizeof (username));
	    read (*sockfd, &tmp, sizeof(int));
	    if (tmp != 1) break;
	    else printf ("Name existed. ");
	}

    printf ("\nWelcome to group %d! (Enter your message)", topic);
    printf ("\nEnter \"!username: message\" to chat with other user or \"@\" to finish.");
    printf ("\nEnter \"$filename\" to transfer File to Group or \"#username:filename\" to transfer File to user");
    //printf ("\nEnter \"@L\" to receive list user online.\n");

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
