/*
* ���� ���� 
* 1. ���α׷����� > �ɻ�� ����
* 2. ����� �Է� > ä�� ����! > ä�ý� ����� ���� �����̱�
* 3. ���� '1' �ܹ��� ��� // ���� '2' ��������
* �ܹ��� ��Ž� null���� �ν��Ͽ� ���������� �� ����
* ����� ��Ž� ���� �� �߽� �� Ȯ�� ���� ���
* 
*/

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PATH "/dev/ttyS0"

#define Tx 14
#define Rx 15

#define IN  0
#define OUT 1



int main() {
	
	int fd;
	fd = open(PATH, O_WRONLY | O_NDELAY | O_NOCTTY );

	if (fd == -1) {
		perror("open_port : Unable to oopen /dev/ttyS0 - ");
		return -1;
	}


	// Turn off blocking for reads, use (fd, F_SETFL, FNDELAY) if you want that
	fcntl(fd, F_SETFL, 0);

	// Write to the port
	int n = write("Hello Peripheral\n");
	if (n < 0) {
		perror("Write failed - ");
		return -1;
	}

	// Read up to 255 characters from the port if they are there
	char buf[256];
	n = read(fd, (void*)buf, 255);
	if (n < 0) {
		perror("Read failed - ");
		return -1;
	}
	else if (n == 0) printf("No data on port\n");
	else {
		buf[n] = '\0';
		printf("%i bytes read : %s", n, buf);
	}

	// Don't forget to clean up
	close(fd);
	return 0;


}


