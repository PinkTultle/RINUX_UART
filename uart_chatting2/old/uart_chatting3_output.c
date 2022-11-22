/* 출력함수
 * 양방향 통신을 하려면 송신 코드에
 * 스레드 함수생성하여 현재파일 소스코드 추가필요
 */


#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>


int main() {
  int fd; //포트 파일을 저장할 공간
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

  char str[256]={}; 
  //출력 버퍼 str 초기화 안할시 최초 문자 전송시에 쓰레기값 전송됨으로 선언과 동시에 초기화
  char mes[] = "chat start\n", outmes[] = "Message : ";
  outmes[-1] = 0;
  int n;


  write(fd, mes, sizeof(mes));
  //최초 "chat start" 문자 출력

  while(1){

	  printf("Message : ");
	  fgets(str, sizeof(str), stdin);

	  if( (strncmp(str,"quit",3) == 0) ) //입력 버퍼 4번째 자리까지 문자열 대조
		  {
			  //quit가 입력되었다면 수신을 종료한다.
			  write(fd,"chat quit\n", 10);
			  return 0;
		  }
	  
	  write(fd, outmes, sizeof(outmes));
	  n = write(fd,str ,sizeof(str) );
	  //출력버퍼 전송 
	  memset(str, 0, sizeof(str) );
	  // 전송후 다음 출력을 위해 버퍼 초기화
	  if(n < 0){
		  perror("Write failed - ");
		  return -1;
	  }
  }

  //열었던 포트 파일 닫기
  close(fd);
  return 0;
}

