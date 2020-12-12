/*
 * Copyright 2013 Tobii Technology AB. All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <math.h>
#include <string.h>
#include <stddef.h>
#include "Common.h"
#include "tobiigaze_discovery.h"

#define URL_SIZE 256

#if __linux__ || __APPLE__
int _kbhit(void);
#endif

#ifdef __linux__
int _kbhit(void)
{
    struct timeval tv;
    fd_set read_fd;

    tv.tv_sec=0;
    tv.tv_usec=0;

    FD_ZERO(&read_fd);
    FD_SET(0,&read_fd);

    if(select(1, &read_fd,NULL, NULL, &tv) == -1)
    return 0;

    if(FD_ISSET(0,&read_fd))
        return 1;

    return 0;
}
#endif
#ifdef __APPLE__
int _kbhit (void)
{
    struct timeval tv;
    fd_set rdfs;

    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&rdfs);
    FD_SET (STDIN_FILENO, &rdfs);

    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
}
#endif

unsigned char isInFile;

/*
 * This is a simple example that demonstrates the synchronous TobiiGazeCore calls.
 * It prints gaze data for 20 seconds.
 */

// Prints gaze information, or "-" if gaze position could not be determined.
void on_gaze_data(const struct tobiigaze_gaze_data* gazedata, const struct tobiigaze_gaze_data_extensions* extensions, void *user_data)
{
    if(!isInFile){
        printf("%20.3f ", gazedata->timestamp / 1e6); // in seconds
        printf("%d ", gazedata->tracking_status);
    }

    int millisec;
    struct timeval tv;

    gettimeofday(&tv, NULL);

    millisec = lrint(tv.tv_usec/1000.0); // Round to nearest millisec
    if (millisec>=1000) { // Allow for rounding up to nearest second
      millisec -=1000;
      tv.tv_sec++;
    }

    time_t t = time(NULL);
    char mbstr[100];
    strftime(mbstr, sizeof(mbstr), "%d_%m_%Y_%H_%M_%S", localtime(&t));

    if(isInFile) 
        fprintf((FILE *)user_data, "%s_%03d,", mbstr, millisec);

    if (gazedata->tracking_status == TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED ||
        gazedata->tracking_status == TOBIIGAZE_TRACKING_STATUS_ONLY_LEFT_EYE_TRACKED ||
        gazedata->tracking_status == TOBIIGAZE_TRACKING_STATUS_ONE_EYE_TRACKED_PROBABLY_LEFT) {
        
        if(!isInFile)        
            printf("[%7.4f,%7.4f] ", gazedata->left.gaze_point_on_display_normalized.x, gazedata->left.gaze_point_on_display_normalized.y);
        else
            fprintf((FILE *)user_data, "%7.4f,%7.4f,", gazedata->left.gaze_point_on_display_normalized.x, gazedata->left.gaze_point_on_display_normalized.y);
    }
    else {
        if(!isInFile)
            printf("[%7s,%7s] ", "-", "-");
        else        
            fprintf((FILE *)user_data, "%7s,%7s,", "-", "-");
    }

    if (gazedata->tracking_status == TOBIIGAZE_TRACKING_STATUS_BOTH_EYES_TRACKED ||
        gazedata->tracking_status == TOBIIGAZE_TRACKING_STATUS_ONLY_RIGHT_EYE_TRACKED ||
        gazedata->tracking_status == TOBIIGAZE_TRACKING_STATUS_ONE_EYE_TRACKED_PROBABLY_RIGHT) {
        
        if(!isInFile)        
            printf("[%7.4f,%7.4f] ", gazedata->right.gaze_point_on_display_normalized.x, gazedata->right.gaze_point_on_display_normalized.y);
        else        
            fprintf((FILE *)user_data, "%7.4f,%7.4f", gazedata->right.gaze_point_on_display_normalized.x, gazedata->right.gaze_point_on_display_normalized.y);
    }
    else {
        if(!isInFile)
            printf("[%7s,%7s] ", "-", "-");
        else        
            fprintf((FILE *)user_data, "%7s,%7s", "-", "-");
    }

    if(!isInFile)
        printf("\n");
    else    
        fprintf((FILE *)user_data, "\n");
}

// Error callback function.
void on_error(tobiigaze_error_code error_code, void *user_data)
{
    report_and_exit_on_error(error_code, tobiigaze_get_error_message(error_code));
}

// Queries for and prints device information.
void print_device_info(tobiigaze_eye_tracker* eye_tracker)
{
    tobiigaze_error_code error_code;
    struct tobiigaze_device_info info;

    tobiigaze_get_device_info(eye_tracker, &info, &error_code);
    report_and_exit_on_error(error_code, "tobiigaze_get_device_info");

    printf("Serial number: %s\n", info.serial_number);
}

void list_eye_trackers()
{
    tobiigaze_error_code error_code;
    char buffer[10 * URL_SIZE];
    const char* url;

    printf("Connected eye trackers:\n");
    tobiigaze_get_connected_eye_trackers(buffer, sizeof(buffer), &error_code);
    if (error_code) {
        printf("Eye tracker enumeration failed with error code %d.\n", error_code);
        exit(-1);
    }

    if (*buffer == 0) {
        printf("No eye trackers found.\n");
    }

    for (url = buffer; *url != 0; url += strlen(url) + 1) {
        printf("%s\n", url);
    }
}

int main(int argc, char** argv)
{

    time_t t = time(NULL);
	const char headers[] = "Time;LeftX;LeftY;RightX;RightY"; 
    char mbstr[100];
    FILE *f = NULL;
    strftime(mbstr, sizeof(mbstr), "%d_%m_%Y_%H_%M_%S", localtime(&t));
    char url[URL_SIZE];
    mkdir("./log", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    //fprintf(user_data, "timestamp,left_x,left_y,right_x,right_y");
    tobiigaze_error_code error_code;
    tobiigaze_eye_tracker* eye_tracker;

    // Process command-line arguments.
    if (argc == 2) {
        strncpy(url, argv[1], URL_SIZE);
        url[URL_SIZE - 1] = 0;

        if (!strcmp(url, "--list")) {
            list_eye_trackers();
            exit(0);
        }

        if (!strcmp(url, "-t")) {
            tobiigaze_get_connected_eye_tracker(url, URL_SIZE, &error_code);
            isInFile = 0;            
            if (error_code) {
                printf("No eye tracker found.\n");
                exit(-1);
            }        
        }

        if (!strcmp(url, "-a")) {
            tobiigaze_get_connected_eye_tracker(url, URL_SIZE, &error_code);
            isInFile = 1;
            if (error_code) {
                printf("No eye tracker found.\n");
                exit(-1);
            }        
        }

        if (!strcmp(url, "-h")){
            printf("usage: MinimalTracker {url|-a|-t|-h|--list}\n");
            printf("\n");
            printf("optional arguments:\n");
            printf("  -h,\t--help\tshow this help message and exit.\n");
            printf("  -a,\t\tstart of testing and save the results into file.\n");
            printf("  -t,\t\tcalibration of device.\n");
            exit(0);
        }
    }
    else {
        printf("usage: MinimalTracker {url|-a|-t|-h|--list}\n");
        return 0;
    }

    if(isInFile){
        char logFolder[100] = "log/";
        strcpy(mbstr, strcat(mbstr, ".TobiiGaze.csv"));
        f = fopen(strcat(logFolder, mbstr), "w");
    }

    printf("TobiiGazeCore version: %s\n", tobiigaze_get_version());

    // Create an eye tracker instance.
    if(!isInFile)
        printf("Creating eye tracker with url %s.\n", url);
    eye_tracker = tobiigaze_create(url, &error_code);
    report_and_exit_on_error(error_code, "tobiigaze_create");

    // Enable diagnostic error reporting. NOTE: This is optional!
    tobiigaze_register_error_callback(eye_tracker, on_error, NULL);
    //tobiigaze_set_logging("minimaltracker.log", TOBIIGAZE_LOG_LEVEL_INFO, NULL);

    // Start the event loop. This must be done before connecting.
    tobiigaze_run_event_loop_on_internal_thread(eye_tracker, 0, 0);

    // Connect to the tracker.
    tobiigaze_connect(eye_tracker, &error_code);
    report_and_exit_on_error(error_code, "tobiigaze_connect");
    
    printf("Connected.\n");
    print_device_info(eye_tracker);

    // Track for a while.

    if(isInFile)
        fputs(headers, f);

    tobiigaze_start_tracking(eye_tracker, &on_gaze_data, &error_code, f);
    report_and_exit_on_error(error_code, "tobiigaze_start_tracking");

    printf("Tracking started.\n");

    while(!_kbhit());

    tobiigaze_stop_tracking(eye_tracker, &error_code);
    report_and_exit_on_error(error_code, "tobiigaze_stop_tracking");
    
    printf("Tracking stopped.\n");

    // Disconnect and clean up.
    tobiigaze_disconnect(eye_tracker);
    
    printf("Disconnected.\n");
    tobiigaze_break_event_loop(eye_tracker);
    tobiigaze_destroy(eye_tracker);
    fclose(f);
    return 0;
}
