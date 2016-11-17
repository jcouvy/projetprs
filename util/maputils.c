<<<<<<< HEAD
#define _POSIX_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <getopt.h>

#define GET_WIDTH    1
#define GET_HEIGHT   2
#define GET_OBJECTS  4
#define GET_INFO     8
#define SET_WIDTH    16
#define SET_HEIGHT   32

/*int getopt_long_only(int argc, char * const argv[],
                  const char *optstring,
                  const struct option *longopts, int *longindex);
si le programme accepte seulement les options
       longues, alors optstring devrait être indiquée avec une  chaîne  vide  « "" »  et  non  avec
       NULL


struct option {
               const char *name;
               int         has_arg;
               int        *flag;
               int         val;
           };

*/

void set_width(int fd, int w){
  printf(" je suis dans set_width avec %d en arg \n",w);
}

void set_height(int fd, int h){
  printf(" je suis dans set_height avec %d en arg \n",h);
}



=======


enum { WIDTH=0, HEIGHT, MAX_OBJ, NB_OBJ, OBJ_INFO, MATRIX };

void get_width(int fd)
{
        unsigned res;
        lseek(fd, WIDTH * sizeof(unsigned), SEEK_SET);
        read(fd, &res, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);
        printf("Map width: %u\n\n", res);
}

void get_height(int fd)
{
        unsigned res;
        lseek(fd, HEIGHT * sizeof(unsigned), SEEK_SET);
        read(fd, &res, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);
        printf("Map height: %u\n\n", res);
}

void get_objects(int fd)
{
        unsigned res;
        lseek(fd, NB_OBJ * sizeof(unsigned), SEEK_SET);
        read(fd, &res, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);
        printf("Number of objects: %u\n\n", res);
}

void get_obj_info(int fd)
{
        unsigned nb_obj;
        lseek(fd, NB_OBJ * sizeof(unsigned), SEEK_SET);
        read(fd, &nb_obj, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);

        int res;
        char str[1024];
        lseek(fd, OBJ_INFO * sizeof(unsigned), SEEK_SET);
        for (int i=0; i<nb_obj; i++) {
                read(fd, &res, sizeof(int));
                printf("Obj type: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Frame: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Solidity: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Destructible: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Generator: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Collectible: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Name length: %d\n", res);
                read(fd, &str, res*sizeof(char));
                printf("Name : %s\n\n", str);
        }
        lseek(fd, 0, SEEK_SET);
}

void get_info(int fd)
{
        get_width(fd);
        get_height(fd);
        get_objects(fd);
        get_obj_info(fd);
}



int main(int argc, char* argv[]){

  if(argc < 3){
       fprintf(stderr, " <file> --option \n");
       exit(EXIT_FAILURE);
  }

  int fd = open(argv[1], O_RDWR, 0600);

  int value_getopt;

  while(1){
       int option_index = 0;
       struct option long_option[] = {
            {"getwidth",    no_argument,         0,   GET_WIDTH},
            {"getheight",   no_argument,         0,   GET_HEIGHT},
            {"getobjects",  no_argument,         0,   GET_OBJECTS},
            {"getinfo",     no_argument,         0,   GET_INFO},
            {"setwidth",    required_argument,   0,   SET_WIDTH},
            {"setheight",   required_argument,   0,   SET_HEIGHT},
            {0,             0,                   0,   0 }

       };

       value_getopt = getopt_long_only(argc,argv,"",long_option,&option_index);

       switch(value_getopt){
       case GET_WIDTH     :
      
         get_width(fd);
         break;
       case GET_HEIGHT    :

         get_height(fd);
         break;
       case GET_OBJECTS   :

         get_objects(fd);
         break;

       case GET_INFO   :

         get_info(fd);
         break;  
       case SET_WIDTH     :
      
         set_width(fd,atoi(optarg));
         break;

       case SET_HEIGHT    :

         set_height(fd,atoi(optarg));
         break;
       }
      break;
    }
}



/*int main(void)
{
        int fd = open("../maps/saved.map", O_RDWR);
        get_height(fd);
        get_width(fd);
        get_objects(fd);
        printf("\n-----------------------\n");
        get_info(fd);
}
*/


/* Change the width to w in an opened file pointed by the descriptor fd */
/*void set_width(int fd, unsigned int w)
   {
        lseek(fd, WIDTH * sizeof(w), SEEK_SET);
        if (write(fd, &w, sizeof(w)) == -1)
                perror("Writing error on set_width()");

   }
 */
/* Change the height to h in an opened file pointed by the descriptor fd */
/*void set_height(int fd, unsigned int h)
   {
        lseek(fd, HEIGHT * sizeof(w), SEEK_SET);
        if (write(fd, &h, sizeof(h)) == -1)
                perror("Writing error on set_height()");
        lseek(fd, 0, SEEK_SET);
   }
 */
>>>>>>> 7ebcdfd023f1a8903c448ce3393154c95e10a8a1
