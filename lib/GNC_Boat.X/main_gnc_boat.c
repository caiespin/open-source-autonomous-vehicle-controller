/*
 * File:   main_gnc_boat.c
 * Author: Pavlo Vlastos
 * Brief:  Guidance, navigation, and control (GNC) library for a boat
 * Created on September 30, 2021, 4:09 PM
 */

/******************************************************************************
 * #INCLUDES                                                                  *
 *****************************************************************************/
#include "xc.h"
#include "Board.h"
#include "System_timer.h"
#include "common/mavlink.h"
#include "MavSerial.h"
#include "ICM_20948.h"
#include "linear_trajectory.h"
#include "pid_controller.h"
#include "RC_RX.h"
#include "RC_servo.h"
#include "Publisher.h"

/******************************************************************************
 * #DEFINES                                                                   *
 *****************************************************************************/
#define HEARTBEAT_PERIOD 1000 //1 sec interval for hearbeat update
#define GPS_PERIOD 1000 //1 Hz update rate (For the time being)
#define CONTROL_PERIOD 20 //Period for control loop in msec
#define SAMPLE_TIME (1.0 / ((float) CONTROL_PERIOD))
#define RAW 1
#define SCALED 2

#define UPPER_ACT_BOUND ((float) 0.8) // The maximum rudder actuator limit in radians
#define LOWER_ACT_BOUND ((float)-0.8) // The minimum rudder actuator limit in radians
#define SERVO_PAD 30
#define ACTUATOR_SATURATION (M_PI/20.0)/SAMPLE_TIME

/******************************************************************************
 * MAIN                                                                       *
 *****************************************************************************/
int main(void) {
    uint32_t cur_time = 0;
    uint32_t gps_start_time = 0;
    uint32_t control_start_time = 0;
    uint32_t heartbeat_start_time = 0;
    //    uint8_t index;
    int8_t IMU_state = ERROR;
    int8_t IMU_retry = 5;
    uint32_t IMU_error = 0;
    uint8_t error_report = 50;

    /**************************************************************************
     * Initialization routines                                                *
     *************************************************************************/
    Board_init(); //board configuration
    Sys_timer_init(); //start the system timer

    TRISAbits.TRISA3 = 0; /* Set pin as output. This is also LED4 on Max32 */
    TRISCbits.TRISC1 = 0; /* LED5 */

    LATCbits.LATC1 = 1; /* Set LED5 */
    LATAbits.LATA3 = 1; /* Set LED4 */

    MavSerial_Init();
    
    publisher_init(MAV_TYPE_SURFACE_BOAT);
    
    nmea_serial_init();

    RCRX_init(); //initialize the radio control system
    RC_channels_init(); //set channels to midpoint of RC system
    RC_servo_init(); // start the servo subsystem
    

    IMU_state = IMU_init(IMU_SPI_MODE);

    if (IMU_state == ERROR && IMU_retry > 0) {
        IMU_state = IMU_init(IMU_SPI_MODE);

#ifdef USB_DEBUG
        printf("IMU failed init, retrying %f \r\n", IMU_retry);
#endif
        IMU_retry--;
    }

    LATCbits.LATC1 = 0; /* Set LED5 low */
    LATAbits.LATA3 = 0; /* Set LED4 low */
    
    // MAVLink and State Machine
    uint8_t current_mode = MAV_MODE_MANUAL_ARMED;
    uint8_t last_mode = current_mode;
    
    publisher_set_mode(MAV_MODE_MANUAL_ARMED);
    publisher_set_state(MAV_STATE_ACTIVE);

    // Trajectory 
    float position_lla[DIM];
    float position_ned[DIM];
    float next[DIM];

    float cross_track_error = 0.0;
    float closest_point[DIM];
    float path_angle = 0.0;
    float yaw_rate = 0.0;
    float u = 0.0; // Resulting control effort
    uint16_t u_pulse = 0; // Pulse from converted control effort

    // Controller
    pid_controller_t trajectory_tracker;
    pid_controller_init(&trajectory_tracker,
            SAMPLE_TIME, // dt The sample time
            10.0, // kp The initial proportional gain
            0.001, // ki The initial integral gain
            1.0, // kd The initial derivative gain
            UPPER_ACT_BOUND, // The maximum rudder actuator limit in radians
            LOWER_ACT_BOUND); // The minimum rudder actuator limit in radians

#ifdef USB_DEBUG
    printf("\r\nMinimal Mavlink application %s, %s \r\n", __DATE__, __TIME__);
#endif

    unsigned int control_loop_count = 0;

    /**************************************************************************
     * Primary Loop                                                           *
     *************************************************************************/
    while (1) {
        cur_time = Sys_timer_get_msec();

        /**********************************************************************
         * Check for all events                                               *
         *********************************************************************/
        check_IMU_events(SCALED);
        check_mavlink_serial_events();
        check_RC_events(); //check incoming RC commands
        check_GPS_events(); //check and process incoming GPS messages
        
        // Check if mode switch event occurred
        current_mode = check_mavlink_mode();
        //        if (current_mode != last_mode) {
        //            last_mode = current_mode;
        //            publisher_get_gps_rmc_position(position_lla);
        //            //            lin_tra_init(position,);
        //        }
        
        /**********************************************************************
         * State Machine Logic                                                *
         *********************************************************************/
        switch (current_mode) {
            case MAV_MODE_MANUAL_ARMED: 
                break;
                
            case MAV_MODE_AUTO_ARMED: 
                break;
        }
        
        /**********************************************************************
         * CONTROL: Control and publish data                                  *
         *********************************************************************/
        if ((cur_time - control_start_time) > CONTROL_PERIOD) {
            control_start_time = cur_time; //reset control loop timer

            set_control_output(); // set actuator outputs

            IMU_state = IMU_start_data_acq(); //initiate IMU measurement with SPI

            if (IMU_state == ERROR) {
                IMU_error++;
                if (IMU_error % error_report == 0) {

#ifdef USB_DEBUG
                    printf("IMU error count %d\r\n", IMU_error);
#endif

                }
            }

            /******************************************************************
             * Control                                                        *
             *****************************************************************/


            //            cross_track_error = 
            //            
            //            u = pid_controller_update(
            //                    &trajectory_tracker,
            //                    0.0, // Commanded reference
            //                    cross_track_error,
            //                    yaw_rate); // Change in heading angle over time
            //
            //            // Scale resulting control input
            //            u_pulse = (float) (RC_RX_MID_COUNTS);
            //            u_pulse -= (((u * (float) ((((float) RC_RX_MID_COUNTS)
            //                    - ((float) (RC_RX_MIN_COUNTS + SERVO_PAD)))))
            //                    / ((float) ACTUATOR_SATURATION)));

            //            RC_servo_set_pulse(u_pulse, RC_STEERING);

            // Information from trajectory


            /******************************************************************
             * Publish data                                                   *
             *****************************************************************/
            publish_RC_signals_raw();
            publish_IMU_data(SCALED);
            control_loop_count++;
        }

        // Publish GPS
//#ifdef USING_GPS
        if (cur_time - gps_start_time >= GPS_PERIOD) {
            gps_start_time = cur_time; //reset GPS timer
            publish_GPS();
        }
//#endif
        // Publish heartbeat
        if (cur_time - heartbeat_start_time >= HEARTBEAT_PERIOD) {
            heartbeat_start_time = cur_time; //reset the timer
            publish_heartbeat(); // TODO: add argument to update the mode 

            LATAbits.LATA3 ^= 1; /* Set LED4 */

        }
    }

    return 0;
}