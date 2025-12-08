/*



                    LAB 2 TASK COMMUNICATE



*/


/**
 * @file    task_communicate.c
 * @author  Eriks Zaharans
 * @date    31 Oct 2013
 *
 * @section DESCRIPTION
 *
 * Communicate Task.
 */

/* -- Includes -- */
/* system libraries */

/* project libraries */
#include "task.h"
#include <math.h>
#include "timelib.h"
#include <sys/time.h>

 /**
 * Communication (receive and send data)
 */
void task_communicate(void)
{

	        clock_t start_time = clock();
	struct timeval begin;
	timelib_timer_set(&begin);
    printf("eho\n");
    // Check if task is enabled
    if(g_task_communicate.enabled == s_TRUE)
    {
        // Local variables
        void *data; 
        int data_type; 
        char udp_packet[g_config.udp_packet_size];
        int udp_packet_len;
        protocol_t packet;
        int seq = 0; 
        int last_id = g_list_send->count;
        // Init lists
        doublylinkedlist_t *victim_list = doublylinkedlist_init();
        doublylinkedlist_t *position_list = doublylinkedlist_init();
        doublylinkedlist_t *pheromone_list = doublylinkedlist_init();
        doublylinkedlist_t *stream_list = doublylinkedlist_init();
        doublylinkedlist_t *current_list;

        // sort main list of messages into different lists based on message type
        while(g_list_send->count > 0)
        {
            doublylinkedlist_node_t *node = g_list_send->first;
            
            switch(node->data_type)
            {
                case s_DATA_STRUCT_TYPE_ROBOT:
                    data = malloc(sizeof(robot_t));
                    break;
                case s_DATA_STRUCT_TYPE_VICTIM:
                    data = malloc(sizeof(victim_t));
                    break;
                case s_DATA_STRUCT_TYPE_PHEROMONE:
                    data = malloc(sizeof(pheromone_map_sector_t));
                    break;
                case s_DATA_STRUCT_TYPE_STREAM:
                    data = malloc(sizeof(stream_t));
                    break;
                default:
                    // if unknown type, remove from list cleanly
                    data = malloc(100);
                    doublylinkedlist_remove(g_list_send, node, data, &data_type);
                    free(data);
                    continue; 
            }

            // extract data from list
            doublylinkedlist_remove(g_list_send, node, data, &data_type);

            // insert in the new list according to data type
            switch(data_type)
            {
                case s_DATA_STRUCT_TYPE_ROBOT:
                    doublylinkedlist_insert_end(position_list, data, data_type);
                    break;
                case s_DATA_STRUCT_TYPE_VICTIM:
                    doublylinkedlist_insert_end(victim_list, data, data_type);
                    break;
                case s_DATA_STRUCT_TYPE_PHEROMONE:
                    doublylinkedlist_insert_end(pheromone_list, data, data_type);
                    break;
                case s_DATA_STRUCT_TYPE_STREAM:
                    doublylinkedlist_insert_end(stream_list, data, data_type);
                    break;
            }

            free(data);
        }

        // send data
        int measure_flag = 0;
        int send_exceed_flag = 0;
		

        doublylinkedlist_t* lists[] = {victim_list, position_list, pheromone_list, stream_list};
        int types[] = {s_DATA_STRUCT_TYPE_VICTIM, s_DATA_STRUCT_TYPE_ROBOT, s_DATA_STRUCT_TYPE_PHEROMONE, s_DATA_STRUCT_TYPE_STREAM};

        for(int i = 0; i < 4; i++)
        {
            current_list = lists[i];
            // Victim/Locqtion = prioritaire 0, others = 1
			if(i>=2) measure_flag = 1;
            while(current_list->count != 0)
            {
                // Vérification du temps (seulement pour Pheromone et Stream)
                if(measure_flag == 1 && send_exceed_flag == 0) {
                     //double time_taken = ((double)(clock() - start_time) / CLOCKS_PER_SEC) * 1000.0;
					 double time_v = timelib_timer_get(begin);
					 printf("communicqte %.2f \n", time_v);
                     if(time_v > 10.0) { // limit of time for the communicate task
                         debug_printf("Time Limit Reached\n");
                         send_exceed_flag = 1;
                     }
                }

                doublylinkedlist_node_t *current = current_list->first;
                
               //data alloction with type size
                switch(types[i]) {
                    case s_DATA_STRUCT_TYPE_ROBOT: 
						data = malloc(sizeof(robot_t)); 
						break;
                    case s_DATA_STRUCT_TYPE_VICTIM: 
						data = malloc(sizeof(victim_t)); 
						break;
                    case s_DATA_STRUCT_TYPE_PHEROMONE: 
						data = malloc(sizeof(pheromone_map_sector_t)); 
						break;
                    case s_DATA_STRUCT_TYPE_STREAM: 
						data = malloc(sizeof(stream_t)); 
						break;
                    default: 
						data = malloc(1); 
						break; 
                }

                doublylinkedlist_remove(current_list, current, data, &data_type);

                if (measure_flag == 1 && send_exceed_flag == 1) {
                    free(data);
                    continue;
                }

                seq++;
                protocol_encode(udp_packet, &udp_packet_len, s_PROTOCOL_ADDR_BROADCAST,
                        g_config.robot_id, g_config.robot_team, s_PROTOCOL_TYPE_DATA,
                        seq, g_message_sequence_id, last_id, data_type, data);

                udp_broadcast(g_udps, udp_packet, udp_packet_len);

                free(data);
            }
            doublylinkedlist_destroy(current_list);
        }

        


        /* --- Receive Data --- */
        // Receive packets, decode and forward to proper process

        // Receive UDP packet
        while(udp_receive(g_udps, udp_packet, &udp_packet_len) == s_OK)
        {
            // Decode packet
            //printf("%s\n",udp_packet);
            if(protocol_decode(&packet, udp_packet, udp_packet_len, g_config.robot_id, g_config.robot_team) == s_OK)
            {
                // Now decoding depends on the type of the packet
                switch(packet.type)
                {
                // ACK
                case s_PROTOCOL_TYPE_ACK :
                    // Do nothing
                    break;

                //Massi: go_ahead packet
                case s_PROTOCOL_TYPE_GO_AHEAD :
                {
                    // Declare go ahead command 
                    command_t go_ahead;
                    go_ahead.cmd = s_CMD_GO_AHEAD;
                    // Redirect to mission by adding it to the queue
                    queue_enqueue(g_queue_mission, &go_ahead, s_DATA_STRUCT_TYPE_CMD);

                    // Debuging stuff
                    debug_printf("GO_AHEAD RECEIVED for robot %d team %d\n",packet.recv_id,packet.send_team);
                    // Calculate time from packet (ms and s)
                    int send_time_s = floor(packet.send_time / 1000);
                    int send_time_ms = packet.send_time % 1000;
                    int now = floor(((long long)timelib_unix_timestamp() % 60000) / 1000);
                    debug_printf("GO_AHEAD_TIME: %d (%d)\n",send_time_s,now);

                    break;
                }
                // Data
                case s_PROTOCOL_TYPE_DATA :
                    // Continue depending on the data type
                    switch(packet.data_type)
                    {
                    // Robot pose
                    case s_DATA_STRUCT_TYPE_ROBOT :
                        debug_printf("received robot\n");
                        // Do nothing
                        break;
                    // Victim information
                    case s_DATA_STRUCT_TYPE_VICTIM :
                        debug_printf("received victim\n");
                        // Redirect to mission by adding it to the queue
                        queue_enqueue(g_queue_mission, packet.data, s_DATA_STRUCT_TYPE_VICTIM);
                        break;
                    // Pheromone map
                    case s_DATA_STRUCT_TYPE_PHEROMONE :
                        debug_printf("received pheromone\n");
                        // Redirect to navigate by adding it to the queue
                        queue_enqueue(g_queue_navigate, packet.data, s_DATA_STRUCT_TYPE_PHEROMONE);

                        break;
                    // Command
                    case s_DATA_STRUCT_TYPE_CMD :
                        debug_printf("received CMD\n");
                        // Redirect to mission by adding it to the queue
                        queue_enqueue(g_queue_mission, packet.data, s_DATA_STRUCT_TYPE_CMD);
                        break;
                    case s_DATA_STRUCT_TYPE_STREAM :
                        debug_printf("received data stream item\n");
                        break;
                    // Other
                    default :
                        // Do nothing
                        break;
                    }
                // Other ?
                default:
                    // Do nothing
                    break;
                }

                // Free memory (only if data packet was received!)
                if(packet.type == s_PROTOCOL_TYPE_DATA)
                    free(packet.data);
            }
        }

        // Increase msg sequance id
        g_message_sequence_id++;
    }
}