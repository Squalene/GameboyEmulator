#include <stdint.h>
#include "sidlib.h"
#include "gameboy.h"
#include "error.h"
#include "ourError.h"
#include "image.h"
#include "joypad.h"
#include <sys/time.h>


// Key press bits
#define MY_KEY_UP_BIT    0x01
#define MY_KEY_DOWN_BIT  0x02
#define MY_KEY_RIGHT_BIT 0x04
#define MY_KEY_LEFT_BIT  0x08
#define MY_KEY_A_BIT     0x10
#define MY_KEY_B_BIT     0x12
#define MY_KEY_SELECT_BIT    0x14
#define MY_KEY_START_BIT     0x18

#define REFRESH_TIME 40 //Time between each image refresh in ms : 25Hz refresh rate
#define WINDOW_SCALE 3
#define MICROSECONDS_IN_SECONDS 1000000

#define MAX_COLOR_VALUE 255

#define GREY_SCALE_CONVERSION(pixel)\
    (MAX_COLOR_VALUE - 85 * pixel)

#define PRESS_KEY(gameboy, symbol)\
do { \
    int err=joypad_key_pressed(&(gameboy.pad), symbol ##_KEY);\
    if(err!=ERR_NONE){\
        fprintf(stderr, "Could not press " #symbol"\n");\
        return FALSE;\
    }\
    else {\
        return TRUE;\
    }\
   } while(0)

#define RELEASE_KEY(gameboy, symbol)\
do { \
    int err=joypad_key_released(&(gameboy.pad), symbol ##_KEY);\
    if(err!=ERR_NONE){\
        fprintf(stderr, "Could not release " #symbol"\n");\
        return FALSE;\
    }\
    else {\
        return TRUE;\
    }\
   } while(0)

//global variable for gameboy
gameboy_t gameboy;
struct timeval start;
struct timeval paused;

uint64_t get_time_in_GB_cyles_since(struct timeval* from){
    struct timeval currentTime;
    if(gettimeofday(&currentTime,NULL)!=0){
        fprintf(stderr, "Could not get time of day");
        return 0;
    }
    
    if(timercmp(&currentTime, from, >)==0){//Return non zero if current time strictly greater than from and 0 otherwise
		fprintf(stderr, "from >= currentTime");
        return 0;
    }
    struct timeval delta;
    timersub(&currentTime, from, &delta);//Calculate difference between current time and from
    
    return delta.tv_sec * OUR_GB_CYCLES_PER_S +  (delta.tv_usec * OUR_GB_CYCLES_PER_S) / MICROSECONDS_IN_SECONDS;
    
}

// ======================================================================
static void set_grey(guchar* pixels, int row, int col, int width, guchar grey)
{
    const size_t i = (size_t) (3 * (row * width + col)); // 3 = RGB
    pixels[i+2] = pixels[i+1] = pixels[i] = grey;
}

// ======================================================================
static void generate_image(guchar* pixels, int height, int width)
{
    uint64_t cycleNumber = get_time_in_GB_cyles_since(&start);

    if(gameboy_run_until(&gameboy, cycleNumber)!=ERR_NONE){
        fprintf(stderr, "Error in gameboy_run_until");
    }
    
    uint8_t pixelColor=0;

    for(size_t y=0;y<height;++y){
         for(size_t x=0;x<width;++x){
             
             //We only need to recompute the value every WINDOW_SCALE values
             if(x%WINDOW_SCALE == 0 || y%WINDOW_SCALE == 0){
                 M_PRINT_IF_ERROR(image_get_pixel(&pixelColor, &(gameboy.screen.display), x/WINDOW_SCALE, y/WINDOW_SCALE), "Error reading image");
             }
             set_grey(pixels, y, x, width, GREY_SCALE_CONVERSION(pixelColor));
         }
    }
    
}

// ======================================================================
#define do_key(X) \
    do { \
        if (! (psd->key_status & MY_KEY_ ## X ##_BIT)) { \
            psd->key_status |= MY_KEY_ ## X ##_BIT; \
            puts(#X " key pressed"); \
        } \
    } while(0)

static gboolean keypress_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if (psd == NULL) return FALSE;

    switch(keyval) {
            
        case GDK_KEY_Up:{
			do_key(UP);
            PRESS_KEY(gameboy,UP);
        }

        case GDK_KEY_Down:{
            do_key(DOWN);
            PRESS_KEY(gameboy,DOWN);
        }

        case GDK_KEY_Right:{
            do_key(RIGHT);
            PRESS_KEY(gameboy,RIGHT);
        }

        case GDK_KEY_Left:{
            do_key(LEFT);
            PRESS_KEY(gameboy,LEFT);
        }

        case 'A':
        case 'a':{
            do_key(A);
            PRESS_KEY(gameboy,A);
        }
            
        case 'S':
        case 's':{
            do_key(B);
            PRESS_KEY(gameboy,B);
        }
        
        case 'O':
        case 'o':{
            do_key(SELECT);
            PRESS_KEY(gameboy,SELECT);
        }
            
        case 'I':
        case 'i':{
            do_key(START);
            PRESS_KEY(gameboy,START);
        }
            
        //Handle pause
        case GDK_KEY_space:{
            if( psd->timeout_id>0){
				if(gettimeofday(&paused, NULL)!=0){
					fprintf(stderr, "Could not get time of day");
                }
            }
            else {
                struct timeval currentTime;
                if(gettimeofday(&currentTime,NULL)!=0){
                    fprintf(stderr, "Could not get time of day");
                }
                
                timersub(&currentTime, &paused, &paused);//Mesure time spent since paused and store it in store
                timeradd(&start, &paused, &start);
            }
        }
    }

    return ds_simple_key_handler(keyval, data);
}
#undef do_key

// ======================================================================
#define do_key(X) \
    do { \
        if (psd->key_status & MY_KEY_ ## X ##_BIT) { \
          psd->key_status &= (unsigned char) ~MY_KEY_ ## X ##_BIT; \
            puts(#X " key released"); \
        } \
    } while(0)

static gboolean keyrelease_handler(guint keyval, gpointer data)
{
    simple_image_displayer_t* const psd = data;
    if (psd == NULL) return FALSE;

    switch(keyval) {
        case GDK_KEY_Up:{
            do_key(UP);
            RELEASE_KEY(gameboy,UP);
        }

        case GDK_KEY_Down:{
            do_key(DOWN);
            RELEASE_KEY(gameboy,DOWN);
        }

        case GDK_KEY_Right:{
            do_key(RIGHT);
            RELEASE_KEY(gameboy,RIGHT);
        }

        case GDK_KEY_Left:{
            do_key(LEFT);
            RELEASE_KEY(gameboy,LEFT);
        }

        case 'A':
        case 'a':{
            do_key(A);
            RELEASE_KEY(gameboy,A);
        }
        case 'S':
        case 's':{
            do_key(B);
            RELEASE_KEY(gameboy,B);
        }
        
        case 'O':
        case 'o':{
            do_key(SELECT);
            RELEASE_KEY(gameboy,SELECT);
        }
            
        case 'I':
        case 'i':{
            do_key(START);
            RELEASE_KEY(gameboy,START);
        }
    }

    return FALSE;
}
#undef do_key

// ======================================================================
int main(int argc, char *argv[]){

    if (argc < 2) {
        fprintf(stderr, "please provide input_file");
        return 1;
    }

    const char* const filename = argv[1];
    
    gettimeofday(&start, NULL);//Initialize start with current time at beginning of simulation
    timerclear(&paused);//Reset pause to EPOCH
    
    M_EXIT_IF_ERR(gameboy_create(&gameboy, filename));//gameboy is already freed when there is an error
        
    sd_launch(&argc, &argv,
              sd_init(filename, WINDOW_SCALE*LCD_WIDTH, WINDOW_SCALE*LCD_HEIGHT, REFRESH_TIME,
                      generate_image, keypress_handler, keyrelease_handler));
    
    
    gameboy_free(&gameboy);
    
    return 0;
}

#undef PRESS_KEY
#undef RELEASE_KEY

