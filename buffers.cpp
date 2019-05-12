#include <queue>
#include <deque>
#include <list>
#include <queue>
#include <semaphore.h>
#include <pthread.h>
#include <malloc.h>
#include "datvexpressSink.h"

using namespace std;


// tx buffer queue
static queue <uint8_t *> m_tx_q;
// spare buffer pool
static queue <uint8_t *> m_po_q;
// Mutexes
static pthread_mutex_t mutex_po;
static pthread_mutex_t mutex_tx;
//
// Allocate a new buffer
//
uint8_t *alloc_buff(void)
{
    uint8_t *b;
    pthread_mutex_lock( &mutex_po );
    if(m_po_q.size() > 0 )
    {
        b = m_po_q.front();
        m_po_q.pop();
    }
    else
    {
        // No available memory so allocate some
        b = (uint8_t *)malloc(TP_SIZE);
    }
    pthread_mutex_unlock( &mutex_po );
    return b;
}
//
// Release a buffer
//
void rel_buff(uint8_t *b)
{
    pthread_mutex_lock( &mutex_po );
    m_po_q.push(b);
    pthread_mutex_unlock( &mutex_po );
}
//
// Post a buffer to the tx queue
//
void post_buff( uint8_t *b)
{
    pthread_mutex_lock( &mutex_tx );
    if(m_tx_q.size() < MAX_Q_LEN)
        m_tx_q.push(b);
    else
        rel_buff(b);// Queue overflow
    pthread_mutex_unlock( &mutex_tx );
}
//
// Get a buffer from the tx queue
//
uint8_t *get_buff(void)
{
    uint8_t *b;
    pthread_mutex_lock( &mutex_tx );
    if(m_tx_q.size() > 0 )
    {
        b = m_tx_q.front();
        m_tx_q.pop();
    }
    else
    {
        b = NULL;
    }
    pthread_mutex_unlock( &mutex_tx );
    return b;
}
//
// Initialise the buffers
//
void buf_init(void)
{
    // Create the mutex
    pthread_mutex_init( &mutex_tx, NULL );
    pthread_mutex_init( &mutex_po, NULL );
}
