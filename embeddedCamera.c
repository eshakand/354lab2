/*Andrew Eshak, Valentin Degtyarev
 * program that uses camera to perform many functions 
 * including negation, grayscale and rotation 
 * as well as edge detection and spot the difference 
 */

// #include "address_map_arm.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define KEY_BASE              0xFF200050
#define VIDEO_IN_BASE         0xFF203060
#define FPGA_ONCHIP_BASE      0xC8000000
	int  pixels [320][240];

/* This program demonstrates the use of the D5M camera with the DE1-SoC Board
 * It performs the following: 
 * 	1. Capture one frame of video when any key is pressed.
 * 	2. Display the captured frame when any key is pressed.		  
 */
/* Note: Set the switches SW3 and SW4 to high and rest of the switches to low for correct exposure timing while compiling and the loading the program in the Altera Monitor program.
 */

//writes a character to the screen
void write_char(int x, int y, char c) {
	volatile char * character_buffer = (char *) (0xc9000000 + (y << 7) + x);
	*character_buffer = c;
}
void spotDifference()
{
	
}
//changes individual pixel value, used for debugging 
void write_pixel(int x, int y, short colour) {

//x and y are coordinates of pixel to be changed, colour is the value that the pixel should be changed to.  
	volatile short *vga_addr = (volatile short*) (0xc8000000 + (y << 10)
			+ (x << 1));
	*vga_addr = colour;
}

//clears all written character on the screen 
void clear_chars() {
	int x, y;
	//loops through all pixels of the screen and clears all characters. 
	for (x = 0; x < 320; x++) {
		for (y = 0; y < 240; y++) {
			write_char(x,y,0);
		}
	}
}
/* program takes picture on press of button 1, 
 * then mirrors on press of button 2 
 * or rotates on press of button 3. 
 * on first press of button 4, the taken picture is negated
 * on second press of button 4, the grayscale of the taken picture is displayed
 * on third press, the original picture is displayed 
 * if button 2 is held while in live mode, a picture is taken then 
 * the program performs "spot the difference" between taken picture and live feed
 * if button 3 is pressed while in live mode, a picture is taken then 
 * the program performs edge detection on the still picture
 * if button 4 is held while in live mode, the program performs edge detection on the live feed.  
 */
int main(void) {

	//a counter for the number of pictures taken
	volatile int pictureCounter = 0;
	//to determine grayscale, negate or regular picture based on click 
	volatile int state=0;
	//temporary array to mirror picture 


	while (1) {
	//key and video memory variables 
		volatile int * KEY_ptr = (int *) KEY_BASE;
		volatile int * Video_In_DMA_ptr = (int *) VIDEO_IN_BASE;
		volatile short * Video_Mem_ptr = (short *) FPGA_ONCHIP_BASE;
		
		//variables used to loop through array of pixels 
		int x, y;
		
		*(Video_In_DMA_ptr + 3) = 0x4;					// Enable the video

		if (*KEY_ptr == 0x01)					// check if any KEY was pressed
		{
			pictureCounter++;  //add 1 to picture counter
			*(Video_In_DMA_ptr + 3) = 0x0;// Disable the video to capture one frame


			time_t t = time(NULL);
			struct tm tm = *localtime(&t); //take time of picture taken 
			char timeAndDate[45];
			sprintf(timeAndDate, "Picture Count: %02d     %04d-%02d-%02d %02d:%02d:%02d GMT", pictureCounter, tm.tm_year + 1900,
				tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec); //store picture Count and timestamp in array 
			x = 2;
			//display characters in array on screen to display timestamp and picture counter 
			for (y = 0; y < 45; y++) {
				write_char(x, 58, timeAndDate[y]);
				x++;

			}

			while (*KEY_ptr != 0);					//wait for pushbutton KEY release
			//while not in live mode 
			while (1) {

				if (*KEY_ptr == 0x01)			//check key 1 is pressed while not in live mode
				{
					*(Video_In_DMA_ptr + 3) = 0x4;  //enable the video to return to live mode 
					clear_chars(); 			//clear counter and timestamp from screen 
					while (*KEY_ptr != 0);		//wait until key is released and return to live mode 
					break;
				}

				if (*KEY_ptr == 0x02)			// check key 2 is pressed while not in live mode 
				{

					short horizPixels[320];		//create an array to store inverted x coordinates
					short count;
					int y1, x1, x2;			//variables used to loop through pixels 
						
					for (y1 = 0; y1 < 240; y1++) {		//flip x cordinates of pixels in array to mirror image 
						for (x1 = 0; x1 < 320; x1++) {
							horizPixels[x1] = *(Video_Mem_ptr + (y1 << 9) + x1);
						}

						count = 0;
						for (x2 = 319; x2 > 0; x2 = x2 - 1) {           //dispaly flipped coordinates on the screen 
							write_pixel(x2, y1, horizPixels[count]);
							count++;
						}
					}


					while (*KEY_ptr != 0);			//wait until key is released 
				}

				if (*KEY_ptr == 0x04)		//check if button 3 was pressed while not in live mode 
				{
					short horizPixels[320];     //array of horizontal pixels to be flipped
					short vertPixels[240];      //array of vertical pixels to be flipped 
					short count;                 
					int y1, y2, x1, x2;	    //variables used to loop through pixels 

					for (y1 = 0; y1 < 240; y1++) {
						for (x1 = 0; x1 < 320; x1++) {
							horizPixels[x1] = *(Video_Mem_ptr + (y1 << 9) + x1); //flip horizontal pixels in array 
						}

						count = 0;
						for (x2 = 319; x2 > 0; x2 = x2 - 1) {
							write_pixel(x2, y1, horizPixels[count]);     //display flipped horizontal pixels on screen
							count++;
						}
					}

					for (x1 = 0; x1 < 320; x1++) {
						for (y1 = 0; y1 < 240; y1++) {
							vertPixels[y1] = *(Video_Mem_ptr + (y1 << 9) + x1);	//flip vertical pixels in array 
						}

						count = 0;
						for (y2 = 239; y2 > 0; y2 = y2 - 1) {
							write_pixel(x1, y2, vertPixels[count]);			//display flipped vertical pixes on screen 
							count++;
						}
					}

					while (*KEY_ptr != 0);				// wait for pushbutton KEY release
				}

				if (*KEY_ptr == 0x08)		//check if button 4 is pressed while not in live mode  
				{
					int pic[320][240];	 //array to go through all pixels of array 
					state++;		 //add 1 to state each time button is pressed to change between picture, negation, and grayscale 
					if (state==1)		//if button is pressed once, negate 
					{
						for (y = 0; y < 240; y++) {		//go through all pixels of picture to store in array 
							for (x = 0; x < 320; x++) {
								pic[x][y] =  *(Video_Mem_ptr + (y << 9) + x);
							}
						}

						for (y = 0; y < 240; y++) {
							for (x = 0; x < 320; x++) {	//negate values store in array and display on screen 
								*(Video_Mem_ptr + (y << 9) + x) =0xFFFF - pic[x][y];
							}
						}

						while (*KEY_ptr != 0);		//wait until button is released 
					}

					if (state==2)		// if button 4 was pressed twice, do grayscale 
					{
						for (y = 0; y < 240; y++) {
							for (x = 0; x < 320; x++) {		//for each pixel, filter out the red, green, and blue components of each pixel  
								int r = (pic[x][y] & 0xF800) >> 11;
								int g = (pic[x][y] & 0x07E0) >> 5;
								int b = (pic[x][y] & 0x001F);
								int sum = (r + g + b) / 3;	//take the average value of all three components to do grayscale
								int newPix = 0x0000 | sum | (sum << 6) | (sum << 11); 	//fill in the red, green and blue of new pixel with average value
								*(Video_Mem_ptr + (y << 9) + x) = newPix;    //display grayscale picture on screen
							}
						}
						while (*KEY_ptr != 0);    //wait until key was released

					}
					if (state==3)		// if button 4 was pressed three times, return to original picture
					{
						for (y = 0; y < 240; y++) {
							for (x = 0; x < 320; x++) {
								*(Video_Mem_ptr + (y << 9) + x) =pic[x][y];		//display original pixels on screen 
							}
						}
						while (*KEY_ptr != 0);		//wait until key is released
						state=0;		//return to original state where button three was not pressed
					}
				}
				
				/*used to test code, displays pixels on screen
				
				for (y = 0; y < 240; y++) {
					for (x = 0; x < 320; x++) {
						short temp2 = *(Video_Mem_ptr + (y << 9) + x);
						*(Video_Mem_ptr + (y << 9) + x) = temp2;
					}
				}*/
				
			}
		}
		
		
		if (*KEY_ptr == 0x02)		//if second button was pressed while in live feed, do spot the difference
		{
			*(Video_In_DMA_ptr + 3) = 0x0;	//Disable the video to capture one frame
			for (y = 0; y < 240; y++) {			//capture the value of the pixels in the pixels array 
				for (x = 0; x < 320; x++) {
					short temp2 = *(Video_Mem_ptr + (y << 9) + x);
					pixels[x][y]=temp2;
				}
			}
			*(Video_In_DMA_ptr + 3) = 0x4;  // ENABLE the video 
			while (*KEY_ptr != 0){
				for (y = 0; y < 240; y++) {  //compare RGB values of each pixel in live video to RGB value of pixels in stored array 
					for (x = 0; x < 320; x++) {
						short temp2 = *(Video_Mem_ptr + (y << 9) + x);
						//if RGB values are vastly different, then highlight in blue 
						if ((abs(((pixels[x][y] & 0xF800) >> 11)-((temp2 & 0xF800) >> 11))>0x6)||(abs(((pixels[x][y] & 0x07E0) >> 5)-((temp2 & 0x07E0) >> 5))>0x6)||(abs(((pixels[x][y] & 0x001F))-((temp2 & 0x001F)))>0x6))
						{
							*(Video_Mem_ptr + (y << 9) + x) = 0xFF00FF;
						}
					}
				}// wait for pushbutton KEY release then return to live mode */
			}
		}
		//if button 4 is pressed in live mode, do edge detection on still picture 
		if (*KEY_ptr == 0x04)
		{
			short pix1, pix2, pix3;
			//stop feed to capture picture 
			*(Video_In_DMA_ptr + 3) = 0x00;
			//store pixels in pixels array 
			for (y = 0; y < 240; y++) {
				for (x = 0; x < 320; x++) {
					pixels[x][y] =  *(Video_Mem_ptr + (y << 9) + x);
				}
			}
			//compare each pixel to its horizontal, vertical and diagonal neighbors 
			for (y = 0; y < 239; y++) {
				for (x = 0; x < 319; x++) {
					pix1 = (pixels[x][y] - pixels[x - 1][y]);
					pix2 = (pixels[x][y] - pixels[x][y + 1]);
					pix3 = (pixels[x][y] - pixels[x + 1][y + 1]);
					//if pixels are vastly different, highlight in blue 
					if ( pix1 > 0x2240 || pix2 > 0x2240 || pix3 > 0x2240)
					{
						*(Video_Mem_ptr + (y << 9) + x) = 0xFF00FF;
					}
				}
			}

			while(*KEY_ptr != 0x00); //when button is released return to live mode 
			*(Video_In_DMA_ptr + 3) = 0x04;
		}
		//if button 8 is pressed, do edge detection on live video feed
		if (*KEY_ptr == 0x08)
		{
			short pix1, pix2, pix3;
			//while key is held do edge detection
			while( *KEY_ptr != 0x00 ) {
			
				//store pixels in pixels array 
				for (y = 0; y < 240; y++) {
					for (x = 0; x < 320; x++) {
						pixels[x][y] =  *(Video_Mem_ptr + (y << 9) + x);
					}
				}
				//compare each pixel to its horizontal, vertical and diagonal neighbors 
				for (y = 0; y < 239; y++) {
					for (x = 0; x < 319; x++) {
						pix1 = (pixels[x][y] - pixels[x - 1][y]);
						pix2 = (pixels[x][y] - pixels[x][y + 1]);
						pix3 = (pixels[x][y] - pixels[x + 1][y + 1]);
					//if pixels are vastly different, highlight in blue 
						if ( pix1 > 0x2240 || pix2 > 0x2240 || pix3 > 0x2240)
						{
							*(Video_Mem_ptr + (y << 9) + x) = 0xFF00FF;
						}
					}
				}
			}
		}
	}
}
