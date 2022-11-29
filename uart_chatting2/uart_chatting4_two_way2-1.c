#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>


pthread_mutex_t mutex_lock;
//void *send(void *arg);
void *receprion(void *arg);

int fd; //포트 파일을 저장할 공간
int who=0;

int main() {
	//int fd; //포트 파일을 저장할 공간
	fd = open("/dev/ttyS0", O_RDWR | O_NOCTTY | O_NDELAY);
	// uart1번은 miniuart이다. 따라서 ttyAMA0가 아닌 ttyS0 포트를 사용한다.
	// 3가지 옵션를 적용하여 포트 파일을 연다 
	// O_RDWR    >> 파일을 읽고 쓰기 모두 할 수 있도록 open.
	// O_NOCTTY  >> 코드 실행 중에 터미널 제어를 할수 없도록 막는다 ex)ctrl + C : 입력 강제종료
	// O_NDELAY  >> 읽을 data가 없는 경우 0을 리턴한다. EOF와 같은 기능
	
	if (fd == -1) {
		perror("open_port: Unable to open /dev/ttyS0 - ");
		return(-1);
	}

	fcntl(fd, F_SETFL, 0);
	//이미 연 파일의 옵션을 변경하는 함수 fd의 옵션을 O_NONBLOCK으로 설정한다.
	//다른 방법으로는 fd |= O_NONBLOCK이 있다.
	//이는 IO통신에서 호출함수를 바로 return시키냐를 결정한다.
	
	char buf[256],str[256] = {}; //입력버퍼 buf, 출력버퍼 str 
	char mes[] = ">>>";
	int n;
	int** re;
	pthread_t thread_id;
	pthread_mutex_init(&mutex_lock, NULL);
	
	if((n = pthread_create((&thread_id), NULL, receprion, &fd)) != 0){
		perror("error : don't create thread");
		return 0;
	}
  
	//출력 버퍼 str 초기화 안할시 최초 문자 전송시에 쓰레기값 전송됨으로 선언과 동시에 초기화
	char first[] = "chat start\n";

	write(fd, first, sizeof(first));
	write(fd, mes, sizeof(mes));
	//최초 "chat start" 문자 출력

	while(1){
		printf(">>>");
		fgets(str, sizeof(str), stdin);

		pthread_mutex_lock(&mutex_lock);
		if( (strncmp(str,"quit",3) == 0) ) //입력 버퍼 4번째 자리까지 문자열 대조
		{
			//quit가 입력되었다면 수신을 종료한다.
			write(fd,"chat quit\n", 10);
			return 0;
		}
		if( who != 1)
			write(fd, "\n", sizeof(char));
		
		who = 1;
		write(fd, mes, sizeof(mes));
		n = write(fd,str ,sizeof(str) );
		//출력버퍼 전송 
		memset(str, 0, sizeof(str) );
		//전송후 다음 출력을 위해 버퍼 초기화
		if(n < 0){
			perror("Write failed - ");
			return 0;
		}
		pthread_mutex_unlock(&mutex_lock);
	}

	pthread_join(thread_id, NULL);
	//열었던 포트 파일 닫기
	close(fd);
	return 0;
}


void *receprion(void *arg){
	//수신 PC >> rpi 스레드 함수
	int S_fd = (int)arg;

	fcntl(S_fd, F_SETFL, 0);

	//무한 루프를 이용하여 값이 들오오는지 항시 체크한다.
	
	char buf[256]; //입력버퍼 buf, 출력버퍼 str 
	char mes[] = ">>> ";
	int n;

	while(1){
		if((n = read(fd, (void*)buf, sizeof(buf)))>=0){
			pthread_mutex_lock(&mutex_lock);
			//값이 입력된경우 작동 read함수가 값을 읽었을 경우,
			//읽은 Byte수를 반환 즉 n > 0 경우 값이 입력된것이다.

			if( (strncmp(buf,"quit",3) == 0) ) //입력 버퍼 4번째 자리까지 문자열 대조
			{
				//quit가 입력되었다면 수신을 종료한다.
				printf("chat quit\n");
				return 0;
			}
			
			if(who != 2){
				printf("\n");
			}

			buf[-1] = 0;
			printf("PC message : %s\n", buf); //입력버퍼에 저장된 값출력
			write(fd, mes, sizeof(mes)); //송신측에 ">>>" 출력
			memset(buf, 0, sizeof(buf)); //다시 수신 받았을때 다른 값이 썪이지 않도록 버퍼 초기화
			who = 2;
		}
		else if(n < 0){ //반환값이 -1 이라면 읽기에 실패한경우
			perror("Read failed - ");
		}
		pthread_mutex_unlock(&mutex_lock);
	}
}




/*
void *send(void *arg){
//송신 rpi >> PC스레드 함수


	char str[256]={};

	//출력 버퍼 str 초기화 안할시 최초 문자 전송시에 쓰레기값 전송됨으로 선언과 동시에 초기화
	char mes[] = "chat start\n", outmes[] = "Message : ";
	outmes[-1] = 0;
	int n;


	 //write((int)arg, mes, sizeof(mes));
	 //최초 "chat start" 문자 출력

	 while(1){

		 printf("Message : ");
		 fgets(str, sizeof(str), stdin);
		 if( (strncmp(str,"quit",3) == 0) ) //입력 버퍼 4번째 자리까지 문자열 대조
		 {
			 //quit가 입력되었다면 수신을 종료한다.
			 write((int)arg,"chat quit\n", 10);
			 return 0;
		 }

		 write((int)arg, outmes, sizeof(outmes));
		 n = write((int)arg,str ,sizeof(str) );
		 //출력버퍼 전송
		 memset(str, 0, sizeof(str) );
		 // 전송후 다음 출력을 위해 버퍼 초기화
		 if(n < 0){
			 perror("Write failed - ");
			 //return 0;
		 }
	 }
}
*/
