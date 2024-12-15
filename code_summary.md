# Bitmap image rotation

This task has two parts: First, we need to read the `header`, process it properly and display it, and then, read the pixel colors and display them with the appropriate modifications.

## Header
To read each byte of our file one by one, it is convenient to read characters, which are one byte long. So, we can start the program by reading 54 bytes and getting the information we need. So, we read 54 characters and store them in an array of unsigned characters header. We will need:
- File size (imagesize) -Bytes 2-5
- Offset - Bytes 10-13
- Width - Bytes 18-21
- Height - Bytes 22-25
- Image size (pixelsize) - Bytes 22-25 
 
We should not ignore that the above are in little endian format and we need to convert them. For example, the offset is a hexadecimal number by putting header[13]-header[12]-header[11]-header[10] in order and with all of this we need to make the integer 54.

### How will this be done?
If the numbers were in the decimal system, for example, let's say a=12, b=41, c=03, d=09 and we wanted to create 09034112, we would first create 09000000, 030000, 4100 and 12 and add them. That is, we would get (d\*10<sup>6</sup>)+(c\*10<sup>4</sup>)+(b\*10<sup>2</sup>)+(a\*10^0).
For the hexadecimal system:
Let's say a=ae, b=0a, c=00, d=af. We want to create the number af000aae. We can create the numbers af000000, 000000, 0a00, ae for starters
That is (d\*16<sup>6</sup>)+(c\*16<sup>4</sup>)+(b\*16<sup>2</sup>)+(a\*16<sup>0</sup>) = (d\*(2<sup>4</sup>)<sup>6</sup>)+(c\*(2<sup>4</sup>)<sup>4</sup>)+(b\*(2<sup>4</sup>)<sup>2</sup>)+(a\*(2<sup>4</sup>)<sup>0</sup>) = (d\*2<sup>24</sup>)+(c\*2<sup>16</sup>)+(b\*2<sup>8</sup>)+a = (d<<24)+(c<<16)+(b<<8)+a. And since we're using bitwise operations, it's better to convert the addition to a bitwise or, which gives the same result. So, for the header array, we have:
```
int offset = (header[13] << 24) | (header[12] << 16) | (header[11] << 8) | header[10];
int imagesize = (header[5] << 24) | (header[4] << 16) | (header[3] << 8) | header[2];
int pixelsize = (header[37] << 24) | (header[36] << 16) | (header[35] << 8) | header[34];
int n = (header[25] << 24) | (header[24] << 16) | (header[23] << 8) | header[22];
int m = (header[21] << 24) | (header[20] << 16) | (header[19] << 8) | header[18];
```
where n=height and m=width.

The changes that need to be made to the header are to swap the width with the height (since we will rotate the image), but also to change the two sizes, as the padding will also change. Therefore, we need to calculate in advance the padding of the original image, as well as the padding of the one we will create.

## Padding
The number of bytes of a line is equal to the number of columns (width) * 3, since each pixel needs 3 bytes to be represented. This number must be a multiple of 4, that is, (m\*3)%4=0. If it is not, then the remainder of this division is also the padding. Obviously, the padding of the new image (let's say newpadding) is (n*3)%4 (since columns became rows and vice versa).

## Sizes
The size of the image is determined by the bytes used to represent the image. That is, the pixels of each row * 3, including padding and all that * the number of rows. That is, imagesize=(m*3+padding)\*n. So, the new size after rotation is newsize=(n\*3+newpadding)*m.
The size of the file is equal to the size of the image + the size of the header, which is indicated by the offset. So filename=newsize+offset.
Finally, we just need to put the integers newsize and filename in the appropriate positions of the header, so that the little endian format is preserved.
So, just as we previously multiplied by some power of 16, this time we will divide by the corresponding power. That is, the filename will be divided in such a way that the division with the highest power of 16 is stored in the 4th byte of the header out of the 4 that interest us. As before, the result of each division will be multiplied by 11111111 to be sure that we keep only the two digits that interest us each time (Equivalent: Bitwise and with 0xFF). The result of this operation, however, we must not forget to convert it to unsigned char, as the header array has been declared.

All of the above is summarized in the following code:

First, we declare a 54-position array for the header:
```
unsigned char *header = malloc(54 * sizeof(unsigned char));
    if(!header) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);
```
Read the first 54 bytes of the file, making the header:
```
    for(int i=0; i<54; ++i){
        header[i]=getchar();
    }
```

Convert our data into the formats we need to process it:
```
    int offset = (header[13] << 24) | (header[12] << 16) | (header[11] << 8) | header[10];
    int n = (header[25] << 24) | (header[24] << 16) | (header[23] << 8) | header[22];
    int m = (header[21] << 24) | (header[20] << 16) | (header[19] << 8) | header[18];
    int imagesize = (header[5] << 24) | (header[4] << 16) | (header[3] << 8) | header[2];
    int pixelsize = (header[37] << 24) | (header[36] << 16) | (header[35] << 8) | header[34];
```

Calculate the paddings and sizes:
```
    int padding = (m*3)%4;
    int newpadding = (n*3)%4;
    int newsize=m*n*3+m*newpadding;
    int filesize=newsize+offset;
``` 
Replace each byte of width with its height:
```
    for(int i=18; i<22; ++i){
        unsigned char temp=header[i];
        header[i]=header[i+4]; //change width and height
        header[i+4]=temp;
    }
``` 
Put the new sizes in the new header, in little endian format:
```
    header[34] = (unsigned char)(newsize & 0xFF);
    header[35] = (unsigned char)((newsize >> 8) & 0xFF);
    header[36] = (unsigned char)((newsize >> 16) & 0xFF);
    header[37] = (unsigned char)((newsize >> 24) & 0xFF);    
```
```
    header[2] = (unsigned char)(filesize & 0xFF);
    header[3] = (unsigned char)((filesize >> 8) & 0xFF);
    header[4] = (unsigned char)((filesize >> 16) & 0xFF);
    header[5] = (unsigned char)((filesize >> 24) & 0xFF);  
``` 
Display the formatted header in the new file:
``` 
    for(int i=0; i<offset; ++i){
       putchar(header[i]);
    }
``` 

Deallocate the memory we reserved for the table:
```
free(header);
```


### Technical Specifications
When processing the header, we must pay attention to the following:

1) It starts the magic header with the bytes: 'B' 'M'.

2) It uses 24-bit for color representation (monochrome BMPs
are not accepted).

3) It uses at least 54 (14 + 40) bytes for the representation of the
headers following the Windows 3.x format (Bitmap file header +
DIB header). For example, files that conform to the header format shown in Figure 1 are accepted.

4) It has the correct file size (the header sizes match the file contents).

5) It has the correct length, width, and padding.
 
---

1) First, we read two bytes, header[0] and header[1], and do the appropriate checking:
``` 
header[0]=getchar();
if(header[0]!='B') fprintf(stderr, "Error: not a BMP file\n"), exit(1);
header[1]=getchar();
if(header[1]!='M') fprintf(stderr, "Error: not a BMP file\n"), exit(1);
```

2) This is, according to the bmp file documentation, the depth of the file, which is represented by bytes 28-31. Therefore, as above, we calculate the depth, and check if it is equal to 24:
```
int depth = (header[31] << 24) | (header[30] << 16) | (header[29] << 8) | header[28];
if(depth!=24) fprintf(stderr, "Error: Unhandleable data found.\n"), exit(1);
``` 
3) It is certain that the header will contain 14 bytes, but we do not know exactly how many and we should check it with the help of the offset. It is useful to define the array of size 14, read the offset and redefine its size. Then, we can read the remaining bytes, according to the offset:
```
    for(int i=2; i<14; ++i){ //i is initialized at 2, since header[0] and header[1] have already been read
        header[i]=getchar();
    }

    int offset = (header[13] << 24) | (header[12] << 16) | (header[11] << 8) | header[10];

    if(offset<54) fprintf(stderr, "Error: File contains less than 54 bytes to represent headers.\n"), exit(1);

    header = realloc(header, offset * sizeof(unsigned char));
    if(!header) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);

    for(int i=14; i<offset; ++i){
        header[i]=getchar();
    }
```

4) In order for the sizes we read to match, the file size (imagesize) must be equal to the image size (pixelsize) + the header size, which is the offset:
``` if(pixelsize+offset!=imagesize) fprintf(stderr, "Error: Sizes don't add up.\n"), exit(1); ```

5) In order to have correct padding and dimensions, the image size (pixelsize) must be as follows: ((columns(width) * 3) + padding) * rows(height) = (m\*3+(m\*3%4))\*n, that is:
 ``` if((m*3+(m*3%4))*n!=pixelsize) fprintf(stderr, "Error: Image dimensions don't match with file size.\n"), exit(1); ``` 

And after we've finished with the necessary checks, the code results:
```
    unsigned char *header = malloc(14 * sizeof(unsigned char));
    if(!header) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);

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

    //(Columns x 3 + (Rows x 3) %4) x Rows = pixelsize
    if((m*3+(m*3%4))*n!=pixelsize) fprintf(stderr, "Error: Image dimensions don't match with file size.\n"), exit(1);

    //edit new header
    int padding = (m*3)%4;
    int newpadding = (n*3)%4;
    int newsize=m*n*3+m*newpadding;

    for(int i=18; i<22; ++i){
        unsigned char temp=header[i];
        header[i]=header[i+4]; //change width and height
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
```

## Colors 

The idea is to read the bytes one by one into a colors array of the file and then create a rotate array, which will be the colors with the appropriate rotation. Then, we will print the rotate array.

The colors and rotate arrays will initially be two-dimensional arrays, as shown in the image. However, because each pixel of the image consists of 3 bytes, we will add one more dimension and, due to limited size on the stack, we will allocate them with malloc:
```
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
```

The above is equivalent to:
```
unsigned char colors[n][m][3];
unsigned char rotate[m][n][3];
```

Next, we read the colors array and take care, at the end of each line, to read arbitrarily (without storing them, that is) the padding.
In the same way, we print the rotate array and at the end of each line, we print the character \0 (NULL) for the new padding. The way in which the rotate table is obtained is explained below. Thus, the code results:
```
for(int i=0; i<n; ++i){
        for(int j=0; j<m; ++j){
            for(int k=0; k<3; ++k){
                colors[i][j][k]=getchar();
                rotate[m-1-j][i][k]=colors[i][j][k];
            }
        }
        for(int j=0; j<padding; ++j){ //padding
            getchar(); //read padding
        }
    }

    for(int i=0; i<m; ++i){
        for(int j=0; j<n; ++j){
            for(int k=0; k<3; ++k){
                putchar(rotate[i][j][k]);
            }
        }
        for(int j=0; j<newpadding; ++j){
            putchar('\0'); //print padding
        }
    }
``` 

### How is the rotate table obtained?

Although the image is of the form:
|R|G|
|---|---|
|Y|B|
 
the bytes are in a different order. Specifically, it is of the form:
|Y|B|
|---|---|
|R|G|
 
We want to end up and display the image in the form:
|Y|R|
|---|---|
|B|G|

So the bytes in our file should be arranged in the following way:
|B|G|
|---|---|
|Y|R|
 
Therefore, we want to go from the array
|Y|B|
|---|---|
|R|G|

to the array
|B|G|
|---|---|
|Y|R|

which, as we observe, results from rotating the original array to the left.
 
So, all we have to do is rotate the colors array to the left (i.e. the third dimension of the array will remain as it is since, otherwise, each pixel would be corrupted).
To rotate an array A[n][m] to the left, we will visualize this rotation. Let n=2 and m=3. The positions of array A are:
|0,0 | 0,1 | 0,2|
|---|---|---|
|1,0 | 1,1 | 1,2|
 
while the positions of the inverted array, let B, (based on those of A) are:
|0,2 | 1,2|
|---|---|
|0,1 | 1,1|
|0,0 | 1,0|
 
that is:
- Position 0,0 of A, goes to position 2,0 of B.
- Position 0,1 of A goes to position 1,0 of B.
- Position 0,2 of A goes to position 0,0 of B.
- Position 1,0 of A goes to position 2,1 of B.
- Position 1,1 of A goes to position 1,1 of B.
- Position 1,2 of A goes to position 0,1 of B.

We notice that, if each position is [i][j], in the new array, i of A becomes j in B, while i of B will be equal to 2-j = 3-1-j = m-1-j

Thus, in the loop:
```
for(int i=0; i<n; ++i){
        for(int j=0; j<m; ++j){
            for(int k=0; k<3; ++k){
                colors[i][j][k]=getchar();
```

we add a command ``` rotate[?][?][k]=colors[i][j][k] ```. 
According to the above, the first dimension of rotate is m-1-j and the second is i. So, the command is: ``` rotate[m-1-j][i][k]=colors[i][j][k]; ```  
 
Finally, we must not forget to free the memory we reserved for the two arrays:
```
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
```
And after all this, the final code is:
```
#include <stdio.h>
#include <stdlib.h>

int main(){

    unsigned char *header = malloc(14 * sizeof(unsigned char));
    if(!header) fprintf(stderr, "Error: Memory allocation failed.\n"), exit(1);

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

    //edit new header
    int padding = (m*3)%4;
    int newpadding = (n*3)%4;
    int newsize=m*n*3+m*newpadding;

    for(int i=18; i<22; ++i){
        unsigned char temp=header[i];
        header[i]=header[i+4]; //change width and height
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
        for(int j=0; j<padding; ++j){ //padding
            getchar(); //read padding
        }
    }

    for(int i=0; i<m; ++i){
        for(int j=0; j<n; ++j){
            for(int k=0; k<3; ++k){
                putchar(rotate[i][j][k]);
            }
        }
        for(int j=0; j<newpadding; ++j){
            putchar('\0'); //print padding
        }
    }

    //free section
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
```
