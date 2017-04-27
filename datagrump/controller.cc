#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;
#define alpha 0.3
#define beta 0.3
#define delta 0.1
#define T_low 50.0
#define T_high 100.0
double prev_rtt = 100.0;
double min_rtt = 1000.0;
double rtt_diff = 1.0;
int count = 0;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug ), cwnd_c(20)
{}

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = cwnd_c;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
    << " window size is " << the_window_size << endl;
  }
  return the_window_size;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
                                    /* of the sent datagram */
                                    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
    << " sent datagram " << sequence_number << endl;
  }
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
                               /* what sequence number was acknowledged */
                               const uint64_t send_timestamp_acked,
                               /* when the acknowledged datagram was sent (sender's clock) */
                               const uint64_t recv_timestamp_acked,
                               /* when the acknowledged datagram was received (receiver's clock)*/
                               const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  double rtt = timestamp_ack_received - send_timestamp_acked;
  double new_rtt_diff = rtt - prev_rtt;
  double normalized_grad = rtt_diff / min_rtt;
  cwnd_c = cwnd_c + 1.0 / cwnd_c;
  if (rtt < min_rtt) {
    min_rtt = rtt;
  }
  rtt_diff = (1 - alpha) * rtt_diff + alpha * new_rtt_diff;
  prev_rtt = rtt;

  if (rtt < T_low) {
     cwnd_c = cwnd_c + delta;
  } else if (rtt > T_high) {
    cwnd_c = cwnd_c *(1.0 - beta * (1.0 - T_high / rtt));
    if (cwnd_c < 1.0) {
       cwnd_c = 1.0;
    }
  } else if (normalized_grad <= 0) {
    count++;
    if (count >= 5) {
      cwnd_c = cwnd_c + 5 * delta;
    } else {
      cwnd_c = cwnd_c + delta;
    }
  } else {
    count = 0;
    cwnd_c = cwnd_c * (1.0 - beta * normalized_grad);
    if (cwnd_c < 1.0) {
        cwnd_c = 1.0;
    }
  }

  /* Default: take no action */
  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
    << " received ack for datagram " << sequence_number_acked
    << " (send @ time " << send_timestamp_acked
    << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
    << endl;
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 40;
}
