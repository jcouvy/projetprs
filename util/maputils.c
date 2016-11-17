#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>

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



#define GET_WIDTH    1
#define GET_HEIGHT   2
#define GET_OBJECTS  4
#define GET_INFO     8
#define SET_WIDTH    16
#define SET_HEIGHT   32

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


