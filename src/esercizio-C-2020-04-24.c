#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>


char * text_to_write = "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. At ultrices mi tempus imperdiet nulla malesuada pellentesque elit. Non enim praesent elementum facilisis leo vel fringilla est ullamcorper. Non quam lacus suspendisse faucibus interdum. Metus vulputate eu scelerisque felis imperdiet. At ultrices mi tempus imperdiet nulla malesuada pellentesque. At tempor commodo ullamcorper a lacus vestibulum. Consequat semper viverra nam libero justo laoreet sit amet. Facilisis magna etiam tempor orci eu. Convallis tellus id interdum velit laoreet id donec ultrices. Interdum velit euismod in pellentesque massa placerat duis ultricies. Senectus et netus et malesuada fames. Pharetra vel turpis nunc eget lorem dolor. Nisi porta lorem mollis aliquam ut porttitor leo a. Euismod elementum nisi quis eleifend quam adipiscing vitae proin.";


off_t get_current_file_offset(int fd) {

	off_t file_offset;

	file_offset = lseek(fd, 0, SEEK_CUR); // SEEK_CUR: posizione relativa rispetto alla posizione corrente

	if (file_offset == -1) {
		return -1;
	}

	return file_offset;

}


int main(int argc, char * argv[]) {

	char * file_name="output.txt";
	int text_to_write_len=strlen(text_to_write);
	int first_half_len;
	int second_half_len;
	int res;
	pid_t child_pid;
	off_t file_offset;

	first_half_len=text_to_write_len/2;
	if((text_to_write_len%2)!=0){
		second_half_len=first_half_len+1+1; //tengo conto del '/0'
	}
	else{
		second_half_len=first_half_len+1; //tengo conto del '/0'
	}

	int fd = open(file_name,
				  O_CREAT | O_TRUNC | O_WRONLY,
				  S_IRUSR | S_IWUSR
				 );

	if(fd==-1){
		perror("open()");
		exit(EXIT_FAILURE);
	}

	child_pid = fork();

	if (child_pid == 0) { //figlio

		sleep(3);

		//mi posiziono alla fine del file
		if ((file_offset = lseek(fd, 0, SEEK_END)) == -1) {
			perror("lseek()");
			exit(EXIT_FAILURE);
		}

		printf("Sono il figlio, in questo momento l'offset è: %ld\n", file_offset);

		//scrivo il secondo pezzo della stringa
		res=write(fd, &text_to_write[0]+first_half_len, second_half_len);
		if (res == -1) {
			perror("write()");
			exit(EXIT_FAILURE);
		}

		file_offset=get_current_file_offset(fd);
		printf("Sono il figlio e ho scritto la seconda metà della stringa, in questo momento l'offset è: %ld\n", file_offset);

		if (close(fd) == -1) {
			perror("close()");
		}

		//è andato tutto bene
		exit(EXIT_SUCCESS);

	} else if (child_pid > 0) { //padre

		//mi posiziono all'inizio del file
		file_offset = lseek(fd, 0, SEEK_SET);
		if (file_offset == -1) {
			perror("lseek()");
			exit(EXIT_FAILURE);
		}

		printf("Sono il padre, in questo momento l'offset è: %ld\n", file_offset);

		//scrivo il primo pezzo della stringa
		res=write(fd, text_to_write, first_half_len);
		if (res == -1) {
			perror("write()");
			exit(EXIT_FAILURE);
		}

		file_offset=get_current_file_offset(fd);
		printf("Sono il padre e ho scritto la prima metà della stringa, in questo momento l'offset è: %ld\n", file_offset);
		printf("Sono il padre e ora aspetto il figlio:\n.\n.\n.\n");

		//aspetto il figlio
		if (wait(NULL) == -1) {
			perror("wait error");
		}

		if (close(fd) == -1) {
			perror("close()");
		}

		exit(EXIT_SUCCESS);

	} else {
		perror("fork()");
		exit(EXIT_FAILURE);
	}
}
