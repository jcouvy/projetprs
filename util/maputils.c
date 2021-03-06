
#define _XOPEN_SOURCE 600

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
#define PRUNE_OBJ    128

typedef struct object_map object_map;
typedef struct liste_s liste_s;


typedef struct {
    unsigned width;
    unsigned height;
    unsigned max_obj;
    unsigned nb_objects;
} Map_s;

typedef struct {
    int      found; // Object is found in map
    int      type;
    int      frame;
    int      solidity;
    int      destructible;
    int      collectible;
    int      generator;
    int      name_length;
    char     *name;
} Object_s;

struct liste_s
{
  Object_s obj;
  liste_s *suivant;
};


enum { WIDTH=0, HEIGHT, MAX_OBJ, NB_OBJ, OBJ_INFO, MATRIX };

void help()
{
        printf("--getwidth\n"
               "--getheight\n"
               "--getobjects\n"
               "--getinfo\n"
               "--setwidth\n"
	       "--setheight\n"
               "--setobjects\n"
               "--pruneobjects\n");
}



//return width
int get_width_value(int fd)
{
        unsigned res;
        lseek(fd, WIDTH * sizeof(unsigned), SEEK_SET);
        read(fd, &res, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);
        return res;
}


//return height
int get_height_value(int fd)
{
        unsigned res;
        lseek(fd, HEIGHT * sizeof(unsigned), SEEK_SET);
        read(fd, &res, sizeof(unsigned));
        lseek(fd, 0, SEEK_SET);
        return res;
}



//put offset at matrix position in file
void go_to_matrix(int fd, int width, int height)
{
    int offset = -(width * height);
    lseek(fd, offset * sizeof(int), SEEK_END);
}

void copy_struct_objects(Object_s *src, Object_s *dest, int i_src, int i_dst)
{
        dest[i_dst].found = src[i_src].found;
        dest[i_dst].type = src[i_src].type;
        dest[i_dst].frame = src[i_src].frame;
        dest[i_dst].solidity = src[i_src].solidity;
        dest[i_dst].destructible = src[i_src].destructible;
        dest[i_dst].collectible = src[i_src].collectible;
        dest[i_dst].generator = src[i_src].generator;
        dest[i_dst].name_length = src[i_src].name_length;
        dest[i_dst].name = malloc(dest[i_dst].name_length * sizeof(char));
        dest[i_dst].name = src[i_src].name;
}

Object_s *read_savefile_objects(int SaveFile, int nb_objs)
{
   Object_s *objs = malloc(sizeof(Object_s) * nb_objs);
   int ret;
   for (int i=0 ; i<nb_objs ; i++) {
     ret = read(SaveFile, &objs[i].found,        sizeof(int));
     ret = read(SaveFile, &objs[i].type,         sizeof(int));
     ret = read(SaveFile, &objs[i].frame,        sizeof(int));
     ret = read(SaveFile, &objs[i].solidity,     sizeof(int));
     ret = read(SaveFile, &objs[i].destructible, sizeof(int));
     ret = read(SaveFile, &objs[i].collectible,  sizeof(int));
     ret = read(SaveFile, &objs[i].generator,    sizeof(int));
     ret = read(SaveFile, &objs[i].name_length,  sizeof(int));
     objs[i].name = malloc(objs[i].name_length * sizeof(char));
     ret = read(SaveFile, objs[i].name,          objs[i].name_length * sizeof(char));
     if (ret < 0) perror("Error during reading Object_s carac");
   }
   return objs;
}

void write_savefile_objects(int SaveFile,Object_s *objs,int i)
{
     int ret;
     ret = write(SaveFile, &objs[i].found,        sizeof(int));
     ret = write(SaveFile, &objs[i].type,         sizeof(int));
     ret = write(SaveFile, &objs[i].frame,        sizeof(int));
     ret = write(SaveFile, &objs[i].solidity,     sizeof(int));
     ret = write(SaveFile, &objs[i].destructible, sizeof(int));
     ret = write(SaveFile, &objs[i].collectible,  sizeof(int));
     ret = write(SaveFile, &objs[i].generator,    sizeof(int));
     ret = write(SaveFile, &objs[i].name_length,  sizeof(int));
     ret = write(SaveFile, objs[i].name,          objs[i].name_length * sizeof(char));
     if (ret < 0) perror("Error during reading Object_s carac");
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
        matrix_position = lseek(fd, 0, SEEK_CUR);

	int map_matrix[width][height];

	for(int y = 0; y < height; y++)
	    for(int x = 0; x < width; x++)
	        ret = read(fd, &map_matrix[x][y], sizeof(int)); //Save matrix


        lseek(fd, matrix_position, SEEK_SET);


        int new_matrix[new_width][height];

        for(int y = 0; y<height; y++)               //intialisation new_matrix 
                for(int x = 0; x<new_width; x++)
                        new_matrix[x][y] = -1;

	int diff_width = new_width - width;

	for(int y = 0; y<height; y++) {             //put old_matrix in new_matrix  
                for(int x = 0; x<width-1; x++) {    
		  if(diff_width >= 0)
		    {
		      new_matrix[x][y] = map_matrix[x][y];
		    }
		  else if( x < new_width )
		    {
		      new_matrix[x][y] = map_matrix[x][y];
		    }
                }
        }

	if(diff_width >= 0)
	  {
	    new_matrix[width-1][height-1] = map_matrix[width-1][height-1];
	  }

        ftruncate(fd,matrix_position);

        for (int y=0; y<height; y++) {              //write new_matrix in file
                for (int x=0; x<new_width; x++) {
                        ret = write(fd, &new_matrix[x][y], sizeof(int));
                        if (ret < 0)
			  {
			    perror("Error writing 2D matrix");
			  }
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

	int map_matrix[width][height];

	for(int y = 0; y < height; y++)    // save matrix
	    for(int x = 0; x < width; x++)
	        ret = read(fd, &map_matrix[x][y], sizeof(int));
	 

        lseek(fd, matrix_position, SEEK_SET);

        int new_matrix[width][new_height];

        for(int y = 0; y<new_height; y++)  // intialisation new_matrix
                for(int x = 0; x<width; x++)
                        new_matrix[x][y] = -1;
        

	int diff_height = new_height - height;

	for(int y = 0; y < height; y++){   // put old_matrix in new_matrix
	  for(int x = 0; x < width; x++){
	    if (diff_height >= 0) new_matrix[x][y + diff_height] = map_matrix[x][y];
	    else if(y > (-diff_height - 1)) new_matrix[x][y + diff_height] = map_matrix[x][y];
	  }
	}

        ftruncate(fd,matrix_position);


        for (int y=0; y<new_height; y++){ //write new_matrix in file
                for (int x=0; x<width; x++) {
                        ret = write(fd, &new_matrix[x][y], sizeof(int));
                        if (ret < 0) perror("Error writing 2D matrix");
                }
	}

}

//return true if object is already in map 

bool object_in_savefile(int SaveFile, int index, char *args[])
{
      int res;
      int ret = 1;
      int name_length;

        while (ret>0)
          {
            int in_SaveFile = lseek(SaveFile, 0, SEEK_CUR);
            ret = read(SaveFile, &res,sizeof(int));
            if(ret >0)
	    {
                lseek(SaveFile,6 * sizeof(int),SEEK_CUR);
                read(SaveFile, &name_length, sizeof(int));
                char *sprite = malloc(name_length * sizeof(char));
                read(SaveFile, sprite, name_length *sizeof(char));
                if (strcmp(args[index], sprite) == 0)
		  {
		       lseek(SaveFile, in_SaveFile, SEEK_SET);
                       return true;
		  }
            }
          }
        return false;
}

void set_objects(int SaveFile, int nb_args, char *args[])
{

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

        ftruncate(SaveFile,matrix_pos);
        lseek(SaveFile, OBJ_INFO * sizeof(unsigned), SEEK_SET);

        Object_s *objs = read_savefile_objects(SaveFile,nb_objs);

        bool test = true;
        liste_s *current = malloc(sizeof(liste_s));
        liste_s *first = malloc(sizeof(liste_s));


        int new_obj = 0;
        for (int i=3 ; i<nb_args ; i += 6)
        {
            lseek(SaveFile, OBJ_INFO * sizeof(unsigned), SEEK_SET);
            bool in_savefile = object_in_savefile(SaveFile, i,args);

            if (!in_savefile)
            {

                if(test)
		  {
                    first -> suivant = NULL;
                    current = first ;
                    test = false;
                  }
                else
		  {
                    liste_s *new_obj = malloc(sizeof(liste_s));
                    new_obj->suivant = NULL;
                    current->suivant = new_obj;
                    current = new_obj;
                  }
                int val = atoi(args[i+1]);
                /*write(SaveFile, false, sizeof(int));  // Found flag
                write(SaveFile, &nb_objs, sizeof(int));  // Type = obj index
                write(SaveFile, &val, sizeof(int)); // Frame*/

                current-> obj.found = false;
                if(test)current -> obj.type  = 2;
                else current -> obj.type = nb_objs;
                current -> obj.frame = val;

                if   (strcmp("solid", args[i+2]) == 0) current->obj.solidity = 2;
                else if (strcmp("semi-solid", args[i+2]) == 0) current->obj.solidity = 1;
                else if (strcmp("air", args[i+2]) == 0) current->obj.solidity = 0;
                else
		  {
                    perror("Wrong solidity argument !");
                    exit(EXIT_FAILURE);
		  }

                if   (strcmp("destructible", args[i+3]) == 0) current->obj.destructible = 4;
                else if (strcmp("not-destructible", args[i+3]) == 0) current->obj.destructible = 0;
                else
		  {
                    perror("Wrong destructibiliy argument !");
                    exit(EXIT_FAILURE);
		  }

                if   (strcmp("collectible", args[i+4]) == 0) current->obj.collectible = 8;
                else if (strcmp("not-collectible", args[i+4]) == 0) current->obj.collectible = 0;
                else
		  {
                    perror("Wrong collectibility argument !");
                    exit(EXIT_FAILURE);
		  }

                if   (strcmp("generator", args[i+5]) == 0) current->obj.generator = 16;
                else if (strcmp("not-generator", args[i+5]) == 0) current->obj.generator = 0;
                else
		  {
                    perror("Wrong generator argument !");
                    exit(EXIT_FAILURE);
		  }

                int name_length = strlen(args[i]) + 1;

                current->obj.name_length = name_length;
                current->obj.name = malloc(name_length* sizeof(char));
                current->obj.name = args[i];
                new_obj++;
            }
	    else
	      {
	        int val = atoi(args[i+1]);
		int obj_solidity;
		
	        if   (strcmp("solid", args[i+2]) == 0) obj_solidity = 2;
                else if (strcmp("semi-solid", args[i+2]) == 0) obj_solidity = 1;
                else if (strcmp("air", args[i+2]) == 0) obj_solidity = 0;
                else
		  {
                    perror("Wrong solidity argument !");
                    exit(EXIT_FAILURE);
		  }

		int obj_destructible;
		
                if   (strcmp("destructible", args[i+3]) == 0) obj_destructible = 4;
                else if (strcmp("not-destructible", args[i+3]) == 0) obj_destructible = 0;
                else
		  {
                    perror("Wrong destructibiliy argument !");
                    exit(EXIT_FAILURE);
		  }

		int obj_collectible;

                if   (strcmp("collectible", args[i+4]) == 0) obj_collectible = 8;
                else if (strcmp("not-collectible", args[i+4]) == 0) obj_collectible = 0;
                else
		  {
                    perror("Wrong collectibility argument !");
                    exit(EXIT_FAILURE);
		  }

		int obj_generator;

                if   (strcmp("generator", args[i+5]) == 0) obj_generator = 16;
                else if (strcmp("not-generator", args[i+5]) == 0) obj_generator = 0;
                else
		  {
                    perror("Wrong generator argument !");
                    exit(EXIT_FAILURE);
		  }

                int name_length = strlen(args[i]) + 1;
                /*write(SaveFile, &name_length, sizeof(int));
                  write(SaveFile, args[i], sizeof(name_length));*/

                int obj_name_length = name_length;
		char *obj_name = malloc(name_length* sizeof(char));
                obj_name = args[i];

		for( int i = 0; i < nb_objs; ++i){
		  if(strcmp(obj_name, objs[i].name) == 0)
		    {
		      objs[i].frame = val;
		      objs[i].solidity = obj_solidity;
		      objs[i].destructible = obj_destructible;
		      objs[i].collectible = obj_collectible;
		      objs[i].generator = obj_generator;
		      objs[i].name_length = obj_name_length;
		      objs[i].name = malloc(objs[i].name_length * sizeof(char));
		      objs[i].name = obj_name;
		    }     		
		}
	      }
        }

        current = first;

        int nb_objs_new = nb_objs + new_obj;

        Object_s *objs_new = malloc(sizeof(Object_s) * nb_objs_new);

        for(int i = 0; i < nb_objs; i++)
            copy_struct_objects(objs,objs_new,i,i);

        for( int j = nb_objs; j < nb_objs + new_obj; ++j)
	  {
	    objs_new[j].found = current->obj.found;
	    objs_new[j].type = current->obj.type;
	    objs_new[j].frame = current->obj.frame;
	    objs_new[j].solidity = current->obj.solidity;
	    objs_new[j].destructible = current->obj.destructible;
	    objs_new[j].collectible = current->obj.collectible;
	    objs_new[j].generator = current->obj.generator;
	    objs_new[j].name_length = current->obj.name_length;
	    objs_new[j].name = malloc(objs_new[j].name_length * sizeof(char));
	    objs_new[j].name = current->obj.name;
	    if( current -> suivant != NULL) current = current -> suivant;
	  }

        lseek(SaveFile, OBJ_INFO * sizeof(unsigned), SEEK_SET);
        /*int trunc = lseek(SaveFile, 0, SEEK_CUR);
        ftruncate(SaveFile, trunc);*/

        for(int i = 0; i < nb_objs_new; i++)
            write_savefile_objects(SaveFile,objs_new,i);

        /* Overwrite Max obj & Nb obj */
        lseek(SaveFile, MAX_OBJ * sizeof(unsigned), SEEK_SET);
        write(SaveFile, &nb_objs_new, sizeof(unsigned));
        write(SaveFile, &nb_objs_new, sizeof(unsigned));

        lseek(SaveFile, 0, SEEK_END);

        /* Restore the 2D Matrix */
        for (int y=0 ; y<height ; y++)
            for (int x=0 ; x<width ; x++)
                write(SaveFile, &matrix[x][y], sizeof(int));
}

void prune_obj(int SaveFile){
    unsigned nb_objs;
    lseek(SaveFile, NB_OBJ * sizeof(unsigned), SEEK_SET);
    read(SaveFile, &nb_objs, sizeof(unsigned));

    unsigned width  = get_width_value(SaveFile);
    unsigned height = get_height_value(SaveFile);
    int matrix[width][height];

    go_to_matrix(SaveFile,width,height);
    int matrix_pos = lseek(SaveFile, 0, SEEK_CUR);

    for (int y=0 ; y<height ; y++)
        for (int x=0 ; x<width ; x++)
            read(SaveFile, &matrix[x][y], sizeof(int));
 
    ftruncate(SaveFile,matrix_pos);

    lseek(SaveFile, OBJ_INFO * sizeof(unsigned), SEEK_SET);
    Object_s *objs = read_savefile_objects(SaveFile, nb_objs);

    int nb_in_map = 0;
    for(int i = 0; i < nb_objs; i++)
      {
        if(objs[i].found == 1) nb_in_map++;
      }

    Object_s *new_objs = malloc(sizeof(Object_s) * nb_in_map);

    for(int i = 0, j = 0; i < nb_objs; i++){
        if(objs[i].found)
        {
            copy_struct_objects(objs,new_objs,i,j);
            j++;
        }
    }


    lseek(SaveFile, NB_OBJ * sizeof(unsigned), SEEK_SET);

    write(SaveFile, &nb_in_map, sizeof(int));

    int obj_pos = lseek(SaveFile, 0, SEEK_CUR);
    ftruncate(SaveFile,obj_pos);

    for(int i = 0; i < nb_in_map; i++)
        write_savefile_objects(SaveFile,new_objs,i);


    for (int y=0 ; y<height ; y++)
        for (int x=0 ; x<width ; x++)
            write(SaveFile, &matrix[x][y], sizeof(int));

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
                printf("in map: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Obj type: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Frame: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Solidity: %d\n", res);
		            read(fd, &res, sizeof(int));
                printf("Destructible: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Collectible: %d\n", res);
                read(fd, &res, sizeof(int));
                printf("Generator: %d\n", res);
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






int main(int argc, char* argv[]){

        if(argc < 3)
	  {
                fprintf(stderr, " <file> --option \n");
                exit(EXIT_FAILURE);
	  }

        int fd = open(argv[1], O_RDWR, 0600);
        if(fd == -1)
	  {
                fprintf(stderr, " Error durring open \n <file> --option \n");
                exit(EXIT_FAILURE);
	  }

        int value_getopt;

        while(1) {
                int option_index = 0;
                struct option long_option[] =
		  {
                        {"getwidth",    no_argument,         0,   GET_WIDTH},
                        {"getheight",   no_argument,         0,   GET_HEIGHT},
                        {"getobjects",  no_argument,         0,   GET_OBJECTS},
                        {"getinfo",     no_argument,         0,   GET_INFO},
                        {"setwidth",    required_argument,   0,   SET_WIDTH},
                        {"setheight",   required_argument,   0,   SET_HEIGHT},
			{"setobjects",  required_argument,   0,   SET_OBJECTS},
                        {"pruneobjects",no_argument,         0,   PRUNE_OBJ},
                        {0,             0,                   0,   HELP }

		  };

                value_getopt = getopt_long_only(argc,argv,"",long_option,&option_index);

                switch(value_getopt)
		  {
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
		    if(atoi(optarg) < 16 || atoi(optarg) > 1064)
		      {
			fprintf(stderr," 16 < width < 1064\n");
			exit(EXIT_FAILURE);
		      }
		    else set_width(fd,atoi(optarg));
		    break;

		  case SET_HEIGHT:
		    if(atoi(optarg) < 12 || atoi(optarg) > 20)
		      {
			fprintf(stderr," 12 < height < 20\n");
			exit(EXIT_FAILURE);
		      }
		    else set_height(fd,atoi(optarg));
		    break;
		    
		  case SET_OBJECTS:
		    set_objects(fd,argc,argv);
		    break;

		  case PRUNE_OBJ:
		    prune_obj(fd);
                    break;
		  }

                break;
        }
        return EXIT_SUCCESS;
}
