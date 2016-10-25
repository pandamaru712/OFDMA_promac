#ifndef _macro_h
#define _macro_h

//#define SHOW_PROCESS
//#define SHOW_OPTIMIZATION
//#define SHOW_PROBABILITY
//#define SHOW_RATE
//#define SHOW_SELECTION
//#define SHOW_MATRIX
//#define SHOW_POSITION


#define BUFFER_SIZE 200
#define NUM_STA 50
#define EP_BUFFER_SIZE 2048
#define STR_MAX 256
#define CONFIG_FILE "config.txt"
#define NUM_MULTIPLEX 2

#ifdef SHOW_PROCESS
#define processPrintf printf
#else
#define processPrintf 1 ? (void) 0 : printf
#endif

#ifdef SHOW_OPTIMIZATION
#define optimizationPrintf printf
#else
#define optimizationPrintf 1 ? (void) 0 : printf
#endif

#ifdef SHOW_PROBABILITY
#define probabilityPrintf printf
#else
#define probabilityPrintf 1 ? (void) 0 : printf
#endif

#ifdef SHOW_RATE
#define ratePrintf printf
#else
#define ratePrintf 1 ? (void) 0: printf
#endif

#ifdef SHOW_SELECTION
#define selectionPrintf printf
#else
#define selectionPrintf 1 ? (void) 0: printf
#endif

#ifdef SHOW_MATRIX
#define matrixPrintf printf
#else
#define matrixPrintf 1 ? (void) 0: printf
#endif

#ifdef SHOW_POSITION
#define positionPrintf printf
#else
#define positionPrintf 1 ? (void) 0: printf
#endif

#ifndef SHOW_PROCESS
	#ifndef SHOW_OPTIMIZATION
		#ifndef SHOW_RATE
			#ifndef SHOW_PROBABILITY
				#ifndef SHOW_SELECTION
					#ifndef SHOW_MATRIX
						#ifndef SHOW_POSITION
							#define PROGRESS
						#endif
					#endif
				#endif
			#endif
		#endif
		#endif
#endif

#endif
