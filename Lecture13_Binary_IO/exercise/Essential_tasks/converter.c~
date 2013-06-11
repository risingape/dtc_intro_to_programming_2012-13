#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char* argv[]) {
    FILE *binary_input;
    char str[8];
    char buffer[10000];
    int i = 0;
    int j = 0;
    char s = 0;
   
    if((binary_input = fopen(argv[1], "r")) == NULL) {
        perror("File not found\n");
        exit(1);
    }

    while(!feof(binary_input)) {
        fgets(str, 8, binary_input);
        for(i = 0; i < 8; i++) {
            if(str[i] == '1') {
                s = s + pow(2, 7-i);
            }
        }

        sprintf(buffer[j], "%c ", s);
        s = 0;
        j++;
    }

    fclose(binary_input);

    return(1);
}
