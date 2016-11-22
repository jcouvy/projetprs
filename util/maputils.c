
#define _POSIX_SOURCE
#define _BSD_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <getopt.h>
#include <stdbool.h>

#define HELP        -1
#define GET_WIDTH    1
#define GET_HEIGHT   2
#define GET_OBJECTS  4
#define GET_INFO     8
#define SET_WIDTH    16
#define SET_HEIGHT   32
#define SET_OBJECTS  64

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


enum { WIDTH=0, HEIGHT, MAX_OBJ, NB_OBJ, OBJ_INFO, MATRIX };

void help(){
        printf("--getwidth\n --getheight\n \
  --getobjects\n --getinfo\n	    \
--setwidth\n --setheight\n");
}

typedef struct object_map object_map;


struct object_map
{
        int x;
        int y;
        int num_object;
        object_map *next;

};

//renvoie valeur width de la map
int get_width_value(int fd)
{
        unsigned res;
        lseek(fd, WIDTH * sizeof(unsigned), SEEK_SET);
        read(fd, &res, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);
        return res;
}


//renvoie valeur height de la map
int get_height_value(int fd)
{
        unsigned res;
        lseek(fd, HEIGHT * sizeof(unsigned), SEEK_SET);
        read(fd, &res, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);
        return res;
}

//Cree une liste des objets présent sur la map

object_map *create_object_map(int fd, int width, int height)
{

        int matrix[width][height];
        int ret;
        bool test = false;

        object_map *first_tab_object = malloc(sizeof(object_map));
        object_map *current_object   = malloc(sizeof(object_map));

        for (int y = 0; y<height; y++) {

                for (int x = 0; x<width; x++) {

                        if (test == false) {                        //Tant qu'on a pas de premier objet trouvé
                                ret = read(fd, &matrix[x][y], sizeof(int));
                                if (ret < 0) perror("Error during reading Object_s carac");
                                if (matrix[x][y] != -1 && x!=width-1) {
                                        first_tab_object->x = x;
                                        first_tab_object->y = y;
                                        first_tab_object->num_object = matrix[x][y];
                                        first_tab_object->next = NULL;
                                        current_object = first_tab_object;
                                        test = true;
                                }
                        }
                        else{
                                ret = read(fd, &matrix[x][y], sizeof(int)); //On ajoute a la liste les objets 1 par 1
                                if (ret < 0) perror("Error during reading Object_s carac");
                                if (matrix[x][y] != -1 && x<width && y < height) { // cherche les objets de  la map
                                        if (matrix[x][y] == 1 && x == width -1) ;
                                        else {
                                                object_map *new_object = malloc(sizeof(object_map));
                                                new_object->x = x;
                                                new_object->y = y;
                                                new_object->num_object = matrix[x][y];
                                                new_object->next = NULL;
                                                current_object->next = new_object;
                                                current_object = new_object;
                                        }
                                }
                        }
                }
        }

        return first_tab_object;
}


//Place le curseur dans le fichier fd a la matrice !
void go_to_matrix(int fd, int width, int height)
{
    int offset = -(width * height);
    lseek(fd, offset * sizeof(int), SEEK_END);
}



void set_width(int fd, int new_width)
{

        int ret, matrix_position;
        unsigned width  = get_width_value(fd);
        unsigned height = get_height_value(fd);

        lseek(fd, WIDTH * sizeof(unsigned), SEEK_SET);
        write(fd, &new_width, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);


        go_to_matrix(fd, width, height);
        matrix_position = lseek(fd, 0, SEEK_CUR); // On recupere la position de la matrix pour ne pas avoir a la recalculer

        object_map *first_tab_object = create_object_map(fd,width,height);


        lseek(fd, matrix_position, SEEK_SET);


        int matrix[new_width][height];

        for(int y = 0; y<height; y++) {                // On initialise la matrice en placant le sol et les murs
                for(int x = 0; x<new_width; x++) {
                        if(x!=new_width-1) matrix[x][y] = -1;
                        else matrix[x][y] = 1;
                }
        }


        while(first_tab_object->next != NULL) {    // On place les objets dans la map
                int x = first_tab_object->x;
                int y = first_tab_object->y;
                int num_object = first_tab_object->num_object;
                if (x < new_width-1)
                    matrix[x][y] = num_object;  // uniquement les objets qui doivent rester ( < new_width )
                else if (x == new_width-1 && y == height-1)
                    matrix[x][y] = num_object;
                first_tab_object = first_tab_object->next;
        }

        lseek(fd, matrix_position, SEEK_SET);

        ftruncate(fd,matrix_position);

        for (int y=0; y<height; y++) {
                for (int x=0; x<new_width; x++) {
                        ret = write(fd, &matrix[x][y], sizeof(int));
                        if (ret < 0) perror("Error writing 2D matrix");
                }
        }

}


void set_height(int fd, int new_height)
{
        int ret, matrix_position;
        unsigned width  = get_width_value(fd);
        unsigned height = get_height_value(fd);

        lseek(fd, HEIGHT * sizeof(unsigned), SEEK_SET);
        write(fd, &new_height, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);

        go_to_matrix(fd, width, height);
        matrix_position = lseek(fd, 0, SEEK_CUR);


        object_map *first_tab_object = create_object_map(fd,width,height);

        lseek(fd, matrix_position, SEEK_SET);

        int matrix[width][new_height];

        for(int y = 0; y<new_height; y++) {
                for(int x = 0; x<width; x++) {
                        if(x!=width-1) matrix[x][y] = -1;
                        else matrix[x][y] = 1;
                }
        }

        int diff_height = new_height - height;

        while(first_tab_object->next != NULL) {
                int x = first_tab_object->x;
                int y = first_tab_object->y;
                int num_object = first_tab_object->num_object;
                if (diff_height>=0)
                    matrix[x][y + diff_height] = num_object;
                else if(diff_height<0 && y> (-diff_height-1))
                    matrix[x][y + diff_height] = num_object;
                first_tab_object = first_tab_object->next;

        }

        lseek(fd, matrix_position, SEEK_SET);

        ftruncate(fd,matrix_position);

        for (int y=0; y<new_height; y++) {
                for (int x=0; x<width; x++) {
                        ret = write(fd, &matrix[x][y], sizeof(int));
                        if (ret < 0) perror("Error writing 2D matrix");
                }
        }

}


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

/* Appends the new set of objects parsed in parameters to the savegame.
Each object characteristics are given by the user.
(ex: "images/ground.png" 1 solid not-destructible not-collectible not-generator).
If the object is already present in the map the savegame isn't modified.
Otherwise the new object is appended in edition-mode only (found flag set to false).
*/

void set_objects(int SaveFile, int nb_args, char *args[])
{
        bool object_in_savefile(int SaveFile, int index)
        {
            lseek(SaveFile, 7 * sizeof(int), SEEK_CUR);
            int name_length;
            while (1)
            {
                read(SaveFile, &name_length, sizeof(int));
                char sprite[name_length];
                if (read(SaveFile, sprite, sizeof(sprite)) < 0)
                break;
                if (strcmp(args[index], sprite) == 0)
                return true;
            }
            return false;
        }

        unsigned nb_objs;
        lseek(SaveFile, NB_OBJ * sizeof(unsigned), SEEK_SET);
        read(SaveFile, &nb_objs, sizeof(unsigned));

        unsigned width   = get_width_value(SaveFile);
        unsigned height  = get_height_value(SaveFile);
        int matrix[width][height];

        /* Truncate and save the 2D Matrix */
        go_to_matrix(SaveFile, width, height);
        int matrix_pos = lseek(SaveFile, 0, SEEK_CUR);

        for (int y=0 ; y<height ; y++)
            for (int x=0 ; x<width ; x++)
                read(SaveFile, &matrix[x][y], sizeof(int));

        ftruncate(SaveFile, matrix_pos);


        lseek(SaveFile, 0, SEEK_END);
        for (int i=3 ; i<nb_args ; i += 6)
        {
            if (!object_in_savefile(SaveFile, i))
            {
                lseek(SaveFile, 0, SEEK_END);

                int val = atoi(args[i+1]);
                write(SaveFile, false,   sizeof(int));  // Found flag
                write(SaveFile, &nb_objs, sizeof(int));  // Type = obj index
                write(SaveFile, &val, sizeof(int)); // Frame

                if   (strcmp("solid", args[i+2]) == 0) val = 2;
                else if (strcmp("semi-solid", args[i+2]) == 0) val = 1;
                else if (strcmp("air", args[i+2]) == 0) val = 0;
                else {
                    perror("Wrong solidity argument !");
                    exit(EXIT_FAILURE);
                }
                write(SaveFile, &val, sizeof(int));

                if   (strcmp("destructible", args[i+3]) == 0) val = 4;
                else if (strcmp("not-destructible", args[i+3]) == 0) val = 0;
                else {
                    perror("Wrong destructibiliy argument !");
                    exit(EXIT_FAILURE);
                }
                write(SaveFile, &val, sizeof(int));

                if   (strcmp("collectible", args[i+4]) == 0) val = 8;
                else if (strcmp("not-collectible", args[i+4]) == 0) val = 0;
                else {
                    perror("Wrong collectibility argument !");
                    exit(EXIT_FAILURE);
                }
                write(SaveFile, &val, sizeof(int));

                if   (strcmp("generator", args[i+4]) == 0) val = 16;
                else if (strcmp("not-generator", args[i+4]) == 0) val = 0;
                else {
                    perror("Wrong generator argument !");
                    exit(EXIT_FAILURE);
                }
                write(SaveFile, &val, sizeof(int));

                int name_length = strlen(args[i]) + 1;
                write(SaveFile, &name_length, sizeof(int));
                write(SaveFile, args[i], sizeof(name_length));

                nb_objs++;
            }
        }


        /* Overwrite Max obj & Nb obj */
        lseek(SaveFile, MAX_OBJ * sizeof(unsigned), SEEK_SET);
        write(SaveFile, &nb_objs, sizeof(unsigned));
        write(SaveFile, &nb_objs, sizeof(unsigned));

        lseek(SaveFile, 0, SEEK_END);
        /* Restore the 2D Matrix */
        for (int y=0 ; y<height ; y++)
            for (int x=0 ; x<width ; x++)
                write(SaveFile, &matrix[x][y], sizeof(int));

}


int main(int argc, char* argv[]){

        if(argc < 3) {
                fprintf(stderr, " <file> --option \n");
                exit(EXIT_FAILURE);
        }

        int fd = open(argv[1], O_RDWR, 0600);
        if(fd == -1) {
                fprintf(stderr, " Error durring open \n <file> --option \n");
                exit(EXIT_FAILURE);
        }

        int value_getopt;

        while(1) {
                int option_index = 0;
                struct option long_option[] = {
                        {"getwidth",    no_argument,         0,   GET_WIDTH},
                        {"getheight",   no_argument,         0,   GET_HEIGHT},
                        {"getobjects",  no_argument,         0,   GET_OBJECTS},
                        {"getinfo",     no_argument,         0,   GET_INFO},
                        {"setwidth",    required_argument,   0,   SET_WIDTH},
                        {"setheight",   required_argument,   0,   SET_HEIGHT},
			{"setobjects",  required_argument,   0,   SET_OBJECTS},
                        {0,             0,                   0,   HELP }

                };

                value_getopt = getopt_long_only(argc,argv,"",long_option,&option_index);

                switch(value_getopt) {
                case HELP:
                        help();
                        break;
                case GET_WIDTH:

                        get_width(fd);
                        break;
                case GET_HEIGHT:

                        get_height(fd);
                        break;
                case GET_OBJECTS:

                        get_objects(fd);
                        break;

                case GET_INFO:

                        get_info(fd);
                        break;
                case SET_WIDTH:
                        if(atoi(optarg) < 16 || atoi(optarg) > 1064) {
                                fprintf(stderr," 16 < width < 1064\n");
                                exit(EXIT_FAILURE);
                        }
                        else set_width(fd,atoi(optarg));
                        break;

                case SET_HEIGHT:
                        if(atoi(optarg) < 12 || atoi(optarg) > 20) {
                                fprintf(stderr," 12 < height < 20\n");
                                exit(EXIT_FAILURE);
                        }
                        else set_height(fd,atoi(optarg));
                        break;
		case SET_OBJECTS:

		  set_objects(fd,argc,argv);
		  break;
		  
                }
                break;
        }
        return EXIT_SUCCESS;
}
