#include <stdio.h>
#include <stdlib.h>

int main(){

    unsigned char *header = malloc(14 * sizeof(unsigned char));
    if(!header) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);

    //aparaititoi eleghoi
    header[0]=getchar();
    if(header[0]!='B') fprintf(stderr, "Error: not a BMP file\n"), exit(1);
    header[1]=getchar();
    if(header[1]!='M') fprintf(stderr, "Error: not a BMP file\n"), exit(1);

    for(int i=2; i<14; ++i){
        header[i]=getchar();
    }

    int offset = (header[13] << 24) | (header[12] << 16) | (header[11] << 8) | header[10];

    if(offset<54) fprintf(stderr, "Error: File contains less than 54 bytes to represent headers.\n"), exit(1);

    header = realloc(header, offset * sizeof(unsigned char));
    if(!header) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);

    for(int i=14; i<offset; ++i){
        header[i]=getchar();
    }

    int depth = (header[31] << 24) | (header[30] << 16) | (header[29] << 8) | header[28];
    if(depth!=24) fprintf(stderr, "Error: Unhandleable data found.\n"), exit(1);

    int n = (header[25] << 24) | (header[24] << 16) | (header[23] << 8) | header[22];
    int m = (header[21] << 24) | (header[20] << 16) | (header[19] << 8) | header[18];

    int imagesize = (header[5] << 24) | (header[4] << 16) | (header[3] << 8) | header[2];
    int pixelsize = (header[37] << 24) | (header[36] << 16) | (header[35] << 8) | header[34];

    if(pixelsize+offset!=imagesize) fprintf(stderr, "Error: Sizes don't add up.\n"), exit(1);

    if((m*3+(m*3%4))*n!=pixelsize) fprintf(stderr, "Error: Image dimensions don't match with file size.\n"), exit(1);

    int padding = (m*3)%4;
    int newpadding = (n*3)%4;
    int newsize=m*n*3+m*newpadding;

    for(int i=18; i<22; ++i){
        unsigned char temp=header[i];
        header[i]=header[i+4];
        header[i+4]=temp;
    }
    
    header[34] = (unsigned char)(newsize & 0xFF);
    header[35] = (unsigned char)((newsize >> 8) & 0xFF);
    header[36] = (unsigned char)((newsize >> 16) & 0xFF);
    header[37] = (unsigned char)((newsize >> 24) & 0xFF);    

    int filesize=newsize+offset;

    header[2] = (unsigned char)(filesize & 0xFF);
    header[3] = (unsigned char)((filesize >> 8) & 0xFF);
    header[4] = (unsigned char)((filesize >> 16) & 0xFF);
    header[5] = (unsigned char)((filesize >> 24) & 0xFF);  

    for(int i=0; i<offset; ++i){
       putchar(header[i]);
    }

    free(header);

    /*-----header(above)--------------------colors(below)-----*/

    //desmevsi pinakon
    unsigned char ***colors = malloc(n * sizeof(unsigned char **));
    if(!colors){
        exit(1);
    }

    for(int i=0; i<n; ++i){
        colors[i] = malloc(m * sizeof(unsigned char *)); 
        if(!colors[i]) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);
    
        for(int j=0; j<m; ++j){
            colors[i][j] = malloc(3 * sizeof(unsigned char));
            if(!colors[i][j]) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);
        }
    }
    
    unsigned char ***rotate = malloc(m * sizeof(unsigned char **));
    if(!rotate) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);

    for(int i=0; i<m; ++i){
        rotate[i] = malloc(n * sizeof(unsigned char *)); 
        if(!rotate[i]) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);
    
        for(int j=0; j<n; ++j){
            rotate[i][j] = malloc(3 * sizeof(unsigned char));
            if(!rotate[i][j]) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);
        }
    }

    for(int i=0; i<n; ++i){
        for(int j=0; j<m; ++j){
            for(int k=0; k<3; ++k){
                colors[i][j][k]=getchar();
                rotate[m-1-j][i][k]=colors[i][j][k];
            }
        }
        for(int j=0; j<padding; ++j){
            getchar();
        }
    }

    for(int i=0; i<m; ++i){
        for(int j=0; j<n; ++j){
            for(int k=0; k<3; ++k){
                putchar(rotate[i][j][k]);
            }
        }
        for(int j=0; j<newpadding; ++j){
            putchar('\0');
        }
    }

    for(int i=0; i<n; ++i){
        for(int j=0; j<m; ++j){
            free(colors[i][j]);
        }
        free(colors[i]);
    }
    free(colors);

    for(int i=0; i<m; ++i){
        for(int j=0; j<n; ++j){
            free(rotate[i][j]);
        }
        free(rotate[i]);
    }
    free(rotate);

return 0;
}
