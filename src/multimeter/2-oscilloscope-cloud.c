/*
Main Pi RX address: 0xe5e5e5
Processing Pi RX address: 0xe6e6e6
*/

#include "nrf-test.h"
#include "nrf-hw-support.h"

#define MAX_FRAGMENTS (150)
#define FLOATS_PER_FRAGMENT (7)
#define TOTAL_FLOATS (FLOATS_PER_FRAGMENT * MAX_FRAGMENTS)

// 32-byte packaet fragment
typedef struct {
  uint8_t fragment; // 1 byte
  uint8_t total_fragments; // 1 byte
  uint8_t reserved[2]; // 2 bytes
  float data[7]; // 7 * 4 bytes = 28 bytes
} fragment_t;

void fft(float data_re[], float data_im[], const unsigned int N) {
    return;
}

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
        printk("[Cloud Pi] Waiting for data from main Pi...\n");
        while (nrf_read_exact_timeout(c, &fragment, sizeof(fragment), 60 * 1000 * 1000) != sizeof(fragment)) {
            // wait for data
        }
        printk("[Cloud Pi] Got fragment #%u of %u.\n", fragment.fragment, fragment.total_fragments);
        printk("[Cloud Pi] Data: ");
        for (int i = 0; i < fragment.total_fragments; i++) {
            if (fragment.data[i] == -1) {
                break;
            }
            printk("%f ", fragment.data[i]);
        }
        printk("\n");

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
                memcpy(&received_data[next.fragment * FLOATS_PER_FRAGMENT], next.data, sizeof(next.data));
                // for (int i = 0; i < FLOATS_PER_FRAGMENT; i++) {
                //     if (i < fragment.total_fragments) {
                //         received_data[next.fragment * FLOATS_PER_FRAGMENT + i] = next.data[i];
                //     }
                // }
            }
            printk("[Cloud Pi] Got fragment #%d of %d.\n", next.fragment, next.total_fragments);
            printk("[Cloud Pi] Data: ");
            for (int i = 0; i < fragment.total_fragments; i++) {
                if (next.data[i] == -1) {
                    break;
                }
                printk("%f ", next.data[i]);
            }
            printk("\n");
        }

        // flush receive queue
        nrf_rx_flush(c);
        nrf_recvq_flush(c);

        printk("[Cloud Pi] Received data from main Pi: ");
        for (int i = 0; i < fragment.total_fragments; i++) {
            if (received_data[i] == -1) {
                break;
            }
            printk("%f ", received_data[i]);
        }
        printk("\n");
        // received_data is now filled with the data from the client
        // fft on received data
        // fft (received_data, received_data, received_data_size); // fft on received data
        delay_ms(100);
        send_data[0] = ++counter; // frequency

        // send data to main Pi
        printk("[Cloud Pi] Sending data to main Pi: %f\n", send_data[0]);
        nrf_tx_send_noack(s, 0xe5e5e5, send_data, sizeof(*send_data) * send_data_size); // send data to address 0xe5e5e5 (RX address of main Pi)
    }
}