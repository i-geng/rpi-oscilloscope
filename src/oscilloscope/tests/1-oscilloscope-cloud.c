/*
Main Pi RX address: 0xe5e5e5
Processing Pi RX address: 0xe6e6e6
*/

#include "nrf-test.h"
#include "nrf-hw-support.h"
#include "fft.h"

#define MAX_FRAGMENTS (150)
#define FLOATS_PER_FRAGMENT (7)
#define TOTAL_FLOATS (FLOATS_PER_FRAGMENT * MAX_FRAGMENTS)

// 32-byte packaet fragment
typedef struct {
  uint8_t fragment; // 1 byte
  uint8_t total_fragments; // 1 byte
  uint16_t packet_size; // 2 bytes
  float data[7]; // 7 * 4 bytes = 28 bytes
} fragment_t;

// void fft(float data_re[], float data_im[], const unsigned int N) {
//     return;
// }

void nrf_recvq_flush(nrf_t *n) {
    n->recvq.tail = n->recvq.head;
}


void notmain(void) {
    kmalloc_init();

    float received_data[TOTAL_FLOATS];
    // uint32_t received_data_size = 8; // number of samples
    // float *received_data = kmalloc(sizeof(*received_data) * received_data_size);
    nrf_t *c = client_mk_noack(0xe6e6e6, sizeof(fragment_t));

    uint32_t send_data_size = 1; // frequency
    float *send_data = kmalloc(sizeof(*send_data) * send_data_size);
    nrf_t *s = server_mk_noack(0, sizeof(*send_data) * send_data_size);

    int counter = 0;
    while (1) {
        fragment_t fragment;
        // blocking receive from client
        // printk("[Cloud Pi] Waiting for data from main Pi...\n");
        while (nrf_read_exact_timeout(c, &fragment, sizeof(fragment), 60 * 1000 * 1000) != sizeof(fragment)) {
            // flush receive queue
            nrf_rx_flush(c);
            nrf_recvq_flush(c);
            // wait for data
        }
        // printk("[Cloud Pi] Got fragment #%u of %u.\n", fragment.fragment, fragment.total_fragments);
        // printk("[Cloud Pi] Data: ");
        // for (int i = 0; i < FLOATS_PER_FRAGMENT; i++) {
        //     if (fragment.data[i] == -1) {
        //         break;
        //     }
        //     printk("%f ", fragment.data[i]);
        // }
        // printk("\n");

        // copy first segment
        memcpy(&received_data[fragment.fragment*FLOATS_PER_FRAGMENT], fragment.data, sizeof(fragment.data));

    

        // for (int i = 0; i < FLOATS_PER_FRAGMENT; i++) {
        //     if (i < fragment.total_fragments) {
        //         received_data[fragment.fragment * FLOATS_PER_FRAGMENT + i] = fragment.data[i];
        //     }
        // }

        // if there are more fragments, receive them and copy them to received_data
        for (unsigned i = 1; i < fragment.total_fragments; i++) {
            fragment_t next;
            while (nrf_read_exact_timeout(c, &next, sizeof(next), 60 * 1000 * 1000) != sizeof(next)) {
                // wait for data
            }
            // Copy to big_buf
            if (next.fragment < fragment.total_fragments) {
                // memcpy(&received_data[next.fragment * FLOATS_PER_FRAGMENT], next.data, sizeof(next.data));
                for (int j = 0; j < FLOATS_PER_FRAGMENT; j++) {
                    // printk("Assigning to index %d\n", j);
                    if (i < fragment.total_fragments) {

                        // if ((i * FLOATS_PER_FRAGMENT + j) < 128){
                        //     if(next.data[j] == -99)
                        //         panic("joe");
                        // }

                        received_data[i * FLOATS_PER_FRAGMENT + j] = next.data[j];
                    }
                }
            }
            // printk("[Cloud Pi] Got fragment #%d of %d.\n", next.fragment, next.total_fragments);
            // printk("[Cloud Pi] Data: ");
            // for (int i = 0; i < FLOATS_PER_FRAGMENT; i++) {
            //     if (next.data[i] == -1) {
            //         break;
            //     }
            //     printk("%f ", next.data[i]);
            // }
            // printk("\n");
        }

        // flush receive queue
        nrf_rx_flush(c);
        nrf_recvq_flush(c);

        // printk("[Cloud Pi] Received data from main Pi: ");
        // for (int i = 0; i < 128; i++) {
        //     // if (received_data[i] == -1) {
        //         // break;
        //     // }
        //     printk("%f ", received_data[i]);
        // }
        // printk("\n");



        // received_data is now filled with the data from the client
        // fft on received data
        // fft (received_data, received_data, received_data_size); // fft on received data
        // delay_ms(100);
        float arr[128] = {0}; //xdd
        // float 
        // for (int i =0 ; i < 128; i ++){
        //     printk("")
        // }

        int max_i = fft(received_data, arr, 128);

        // for (int k = 0 ; k < 128; k ++){
        //     printk("Magnitude = %f \n", (received_data[k]*received_data[k])+(arr[k]*arr[k]));
        // }


        // float max = 0;
        // int max_i = 0;  
        // for (int k = 0; k < 128; k++){
        //     // printk("Spectrum index %d = %f \n", k, received_data[k]);
            
        //     // Frequency is maximum amplitude
        //     if((received_data[k]*received_data[k] +arr[k]*arr[k]) > max){
        //         max = (received_data[k]*received_data[k] +arr[k]*arr[k]);
        //         max_i = k;
        //         // printk("max found? \n");
        //     };
        //     // printk("magnitud of index %d is %x \n", k, (received_data[k]*received_data[k] +arr[k]*arr[k]));
        // }

        // fft(received_data, , 128);

        send_data[0] = max_i * (860.0/(128)); // frequency
        // printk("Freq index is %d = %f Hz\n", max_i, send_data[0]);

        // send data to main Pi
        // printk("[Cloud Pi] Sending data to main Pi: %f\n", send_data[0]);
        nrf_tx_send_noack(s, 0xe5e5e5, send_data, sizeof(*send_data) * send_data_size); // send data to address 0xe5e5e5 (RX address of main Pi)
    }
}