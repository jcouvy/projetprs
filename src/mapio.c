#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <animation.h>
#include "map.h"
#include "error.h"

#include <stdbool.h>

#ifdef PADAWAN

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

void map_new(unsigned width, unsigned height) {
  map_allocate(width, height);

  for (int x = 0; x < width; x++)
    map_set(x, height - 1, 0); // Ground

  for (int y = 0; y < height - 1; y++) {
    map_set(0, y, 1);         // Wall
    map_set(width - 1, y, 1); // Wall
  }

  map_object_begin(6);

  /* Object_add order will be equal to object field in map_set(x, y, object) */
  map_object_add("images/ground.png", 1, MAP_OBJECT_SOLID);
  map_object_add("images/wall.png",   1, MAP_OBJECT_SOLID);
  map_object_add("images/grass.png",  1, MAP_OBJECT_SEMI_SOLID);
  map_object_add("images/marble.png", 1,
                 MAP_OBJECT_SOLID | MAP_OBJECT_DESTRUCTIBLE);
  map_object_add("images/flower.png", 1, MAP_OBJECT_AIR);
  map_object_add("images/coin.png",  20,
                 MAP_OBJECT_COLLECTIBLE | MAP_OBJECT_AIR);

  map_object_end();
}

bool isvalueinarray(int val, int *arr, int size){
    int i;
    for (i=0; i < size; i++) {
        if (arr[i] == val)
            return true;
    }
    return false;
}

/* SaveFile Format (line breaks are for readability only, not present in binary):
    Map width  (UNSIGNED)
    Map height (UNSIGNED)
    Maximum amount of object (UNSIGNED)
    Number of objects in map & editor (UNSIGNED) -- equals to max when using set_objects.

    Object 1:
        Found physically in the map (INT)
        Type (INT)
        Frames of its decal (INT)
        Solidity (INT)
        Destructibiliy (INT)
        Collectibility (INT)
        Generator (INT)
        Length of its texture name (INT)
        Texture name (CHAR *)

    (...) Object n

    2D Matrix of the map (INT)
*/
void map_save(char *filename) {

    int SaveFile = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0600);
    if (SaveFile == -1) perror("Error during open()");

    Map_s map;

    map.width      = map_width();
    map.height     = map_height();
    map.max_obj    = map_objects();
    map.nb_objects = 0;

    int ret;
    ret = write(SaveFile, &map.width, sizeof(unsigned));
    ret = write(SaveFile, &map.height, sizeof(unsigned));
    ret = write(SaveFile, &map.max_obj, sizeof(unsigned));
    if (ret < 0) perror("Error during writing Map_s");

    int matrix[map.width][map.height];
    int diff_types[map.max_obj];
    for (int i=0 ; i<map.max_obj ; i++) {
        diff_types[i] = -1;
    }

    /* Filling an integer 2D Matrix with sprite types.
    Also filling a 1D int array saving each type found */
    for (int y=0 ; y<map.height ; y++) {
        for (int x=0 ; x<map.width ; x++) {
            matrix[x][y] = map_get(x, y);
            for (int i=0 ; i<map.max_obj ; i++) {
                if (!isvalueinarray(matrix[x][y], diff_types, map.max_obj)
                && matrix[x][y] != MAP_OBJECT_NONE) {
                    diff_types[map.nb_objects] = matrix[x][y];
                    map.nb_objects++;
                }
            }
        }
    }

    for(int i = 0; i < map.nb_objects; i++)
        printf(" %d\n", diff_types[i]);
    /*for(int i = 0; i < map.nb_objects; i++){
        int old_type = diff_types[i];
        for (int y=0 ; y<map.height ; y++) {
            for (int x=0 ; x<map.width ; x++) {
                if(matrix[x][y] == old_type)
                    matrix[x][y] = i;
            }
        }
        diff_types[i] = i;
    }*/

    ret = write(SaveFile, &map.nb_objects, sizeof(unsigned));
    if (ret < 0) perror("Error writing nb obj");


    /* Creating a struct Object_s of sprites and filling it with each sprite
    characteristics to be found in the map */
    Object_s objs[map.nb_objects];

    for (int i=0 ; i<map.nb_objects ; i++) {
        int type = diff_types[i];
        char* str = map_get_name(type);

        objs[i].found        = true;
        objs[i].type         = type;
        objs[i].frame        = map_get_frames(type);
        objs[i].solidity     = map_get_solidity(type);
        /* Writes MACRO value defined in map.h if bool is True */
        objs[i].destructible = map_is_destructible(type) ? MAP_OBJECT_DESTRUCTIBLE:0;
        objs[i].collectible  = map_is_collectible(type) ? MAP_OBJECT_COLLECTIBLE:0;
        objs[i].generator    = map_is_generator(type) ? MAP_OBJECT_GENERATOR:0;
        /* Saving sprite path length in order to read properly later */
        objs[i].name_length = strlen(str)+1;
        objs[i].name = malloc(objs[i].name_length * sizeof(char));
        strcpy(objs[i].name, str);

        ret = write(SaveFile, &objs[i].found,        sizeof(int));
        ret = write(SaveFile, &objs[i].type,         sizeof(int));
        ret = write(SaveFile, &objs[i].frame,        sizeof(int));
        ret = write(SaveFile, &objs[i].solidity,     sizeof(int));
	ret = write(SaveFile, &objs[i].destructible, sizeof(int));
        ret = write(SaveFile, &objs[i].collectible,  sizeof(int));
        ret = write(SaveFile, &objs[i].generator,    sizeof(int));
        ret = write(SaveFile, &objs[i].name_length,  sizeof(int));
        ret = write(SaveFile, objs[i].name,          objs[i].name_length * sizeof(char));
        if (ret < 0) perror("Error during writing Object_s carac");
    }

    /* Writing in the save file the 2D matrix where matrix[x][y] is the sprite
    type. Nb: this loop should be merged with the one above */
    for (int y=0 ; y<map.height ; y++) {
            for (int x=0 ; x<map.width ; x++) {
                    ret = write(SaveFile, &matrix[x][y], sizeof(int));
                    if (ret < 0) perror("Error writing 2D matrix");
            }
    }

    /**********************************************/
    /* Test prints to check if values are correct */
    /**********************************************/
    printf("- SAVEFILE -\n");
    printf("Map width:     %d\n", map.width);
    printf("Map height:    %d\n", map.height);
    printf("Map max objs:  %d\n", map.max_obj);
    printf("Map diff objs: %d\n", map.nb_objects);

    /* Print map 2D matrix */
    for (int y=0 ; y<map.height ; y++) {
        for (int x=0 ; x<map.width ; x++) {
            printf("%+d ", matrix[x][y]);
        }
        printf("\n");
    }

    for (int i=0 ; i<map.nb_objects ; i++) {
        printf("Type: %d\n", objs[i].type);
        printf("Sprite path: %s\n", objs[i].name);
    }
    /**********************************************/

    for (int i=0 ; i<map.nb_objects ; i++) {
            free(objs[i].name);
    }
    animation_objects_save(SaveFile);
    close(SaveFile);
}

void map_load(char *filename) {

    int LoadFile = open(filename, O_RDONLY);
    if (LoadFile == -1) perror("Error during open()");

    Map_s map;

    int ret;
    ret = read(LoadFile, &map.width,      sizeof(unsigned));
    ret = read(LoadFile, &map.height,     sizeof(unsigned));
    ret = read(LoadFile, &map.max_obj,    sizeof(unsigned));
    ret = read(LoadFile, &map.nb_objects, sizeof(unsigned));
    if (ret < 0) perror("Error during reading Map_s");

    int matrix[map.width][map.height];

    Object_s objs[map.nb_objects];
    for (int i=0 ; i<map.nb_objects ; i++) {
        ret = read(LoadFile, &objs[i].found,        sizeof(int));
        ret = read(LoadFile, &objs[i].type,         sizeof(int));
        ret = read(LoadFile, &objs[i].frame,        sizeof(int));
        ret = read(LoadFile, &objs[i].solidity,     sizeof(int));
	ret = read(LoadFile, &objs[i].destructible, sizeof(int));
        ret = read(LoadFile, &objs[i].collectible,  sizeof(int));
        ret = read(LoadFile, &objs[i].generator,    sizeof(int));
        ret = read(LoadFile, &objs[i].name_length,  sizeof(int));
        objs[i].name = malloc(objs[i].name_length * sizeof(char));
        ret = read(LoadFile, objs[i].name,          objs[i].name_length * sizeof(char));
        if (ret < 0) perror("Error during reading Object_s carac");
    }

    for (int y = 0; y < map.height; ++y) {
            for (int x = 0; x < map.width; ++x) {
                    ret = read(LoadFile, &matrix[x][y], sizeof(int));
                    if (ret < 0) perror("Error during reading 2DMatrix");
            }
    }

    /**********************************************/
    /* Test prints to check if values are correct */
    /**********************************************/
    printf("- LOADFILE -\n");
    printf("Map width:     %d\n", map.width);
    printf("Map height:    %d\n", map.height);
    printf("Map max objs:  %d\n", map.max_obj);
    printf("Map diff objs: %d\n", map.nb_objects);

    for (int i=0 ; i<map.nb_objects ; i++) {
        printf("Type: %d\n", objs[i].type);
        printf("Sprite path: %s\n", objs[i].name);
    }

    /* Print map 2D matrix */
    for (int y=0 ; y<map.height ; y++) {
        for (int x=0 ; x<map.width ; x++) {
            printf("%+d ", matrix[x][y]);
        }
        printf("\n");
    }

    /**********************************************/

    map_allocate(map.width, map.height);

    /* Fills the map with sprites; We search the type corresponding to the order object_add() calls
       We make sure to add only the objects found as the array objs contains elements only
       available in edition mode. */
    for (int y=0 ; y<map.height ; y++) {
        for (int x=0 ; x<map.width ; x++) {
            for (int i=0 ; i<map.nb_objects  && objs[i].found == true; i++) {
                if (objs[i].type == matrix[x][y])
                    map_set(x, y, i);
            }
        }
    }

    map_object_begin(map.nb_objects);

    for (int i=0 ; i<map.nb_objects ; i++) {
        map_object_add(objs[i].name, objs[i].frame, objs[i].solidity | objs[i].collectible | objs[i].generator | objs[i].destructible);

        printf("Object %d - %s\n  Sprite Frames: %d\n  Solidity: %d\n  Collectible: %d\n  Generator: %d\n  Destructible: %d\n", i, objs[i].name, objs[i].frame,  objs[i].solidity, objs[i].collectible, objs[i].generator, objs[i].destructible);
    }

    map_object_end();

    for (int i=0 ; i<map.nb_objects ; i++) {
            free(objs[i].name);
    }

    animation_objects_load(LoadFile);
    close(LoadFile);
}

#endif
