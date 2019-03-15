#ifndef SHOAL_INCLUDE_QUEUE_H_
#define SHOAL_INCLUDE_QUEUE_H_

#include <mutex>
#include <queue>

template <class queue_element_t>
class queue{
    private:
        std::queue<queue_element_t> q;
        std::mutex wr_mutex;
        std::mutex rd_mutex;
        bool use_mutex;
        unsigned int max_size;
    public:
        queue(unsigned int max_size, bool use_mutex){
            this->max_size = max_size-1;
            this->use_mutex = use_mutex;
        };
        
        queue_element_t read(){
            if(use_mutex){
                rd_mutex.lock();
            }
            queue_element_t tmp = q.front();
            q.pop();
            if(use_mutex){
                rd_mutex.unlock();
            }
            return tmp;
        };
        
        void write(queue_element_t element){
            if(use_mutex){
                wr_mutex.lock();
            }
            while(q.size() > max_size){
                // delay
            }
            q.push(element);
            if(use_mutex){
                wr_mutex.unlock();
            }
        }

        int write_nb(queue_element_t element){
            if(use_mutex){
                wr_mutex.lock();
            }
            if(q.size() > max_size){
                if(use_mutex){
                    wr_mutex.unlock();
                }
                return -1;
            }
            q.push(element);
            if(use_mutex){
                wr_mutex.unlock();
            }
            return 0;
        }

        unsigned int size(){
            return q.size();
        }

        bool empty(){
            return q.size() == 0;
        }
        // ~queue();
};

#endif // SHOAL_INCLUDE_QUEUE_H_
