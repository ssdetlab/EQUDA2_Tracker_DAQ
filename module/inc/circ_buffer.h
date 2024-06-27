#ifndef CircBuffer_h
#define CircBuffer_h

#include <iostream>
#include <vector>

template <typename T>
class circ_buffer {
    public:
        circ_buffer(int size) : m_size(size) {
            write_p  = 0;
            read_p   = write_p;
            distance = read_p - write_p;
            buffer.resize(m_size);            
        }
        
        // write data into buffer and increment
        // r/w distance by one
        void write(T data) {
            if (distance < 0 || distance >= m_size) {
                throw std::exception();
            }
            buffer.at(write_p%m_size) = data;
            write_p++;
            distance++;
            if (write_p >= m_size) {
                write_p = 0;
            }
        }
        // write data from buffer and decrement
        // r/w distance by one
        T read() {
            if (distance <= 0 || distance > m_size) {
                throw std::exception();
            }
            T data = buffer.at(read_p%m_size);
            read_p++;
            distance--;
            if (read_p >= m_size) {
                read_p = 0;
            }
            return data;
        }
        // return vector of type T containing n elements
        // and decrease r/w distance by n 
        std::vector<T> slice(int n) {
            if (n > distance) {
                throw std::exception();
            }
            if (read_p + n < m_size) {
                std::vector<T> data(buffer.begin() + read_p, buffer.begin() + read_p + n);
                read_p   += n;
                distance -= n;
                return data;
            }
            else{
                std::vector<T> data(buffer.begin() + read_p, buffer.end());
                data.insert(data.end(), buffer.begin(), buffer.begin() + n - (m_size - read_p));
                read_p    = 0 + n - (m_size - read_p);
                distance -= n;
                return data;
            }
        }
        // return vector of type T containing n elements
        // without changing r/w distance 
        std::vector<T> const_slice(unsigned int n) {
            if (n > distance) {
                throw std::exception();
            }
            if (read_p + n < m_size) {
                std::vector<T> data(buffer.begin() + read_p, buffer.begin() + read_p + n);
                return data;
            }
            else{
                std::vector<T> data(buffer.begin() + read_p, buffer.end());
                data.insert(data.end(), buffer.begin(), buffer.begin() + n - (m_size - read_p));
                return data;
            }
        }

        // return current position of the pointer
        typename std::vector<T>::iterator ptr_position() {
            return buffer.begin() + read_p;
        }

        // increment read pointer by n
        // and decrease r/w distance by n
        void inc_read(unsigned int n) {
            if (n > distance) {
                throw std::exception();
            }
            read_p   += n;
            if (read_p >= m_size) {
                read_p %= m_size;
            }
            distance -= n;
        }

        void resize(unsigned int n) { 
            buffer.resize(buffer.size() + n); 
            this->m_size += n;
        }
        int get_distance() { return this->distance; }
        unsigned int size() { return this->m_size; }
    private:
        unsigned int m_size;
        unsigned int read_p, write_p;
        int distance;
        std::vector<T> buffer;
};

#endif
