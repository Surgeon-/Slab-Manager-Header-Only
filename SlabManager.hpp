#pragma once

#include <vector>
#include <stdexcept>

namespace gen {
    
    class SlabManager {

        public:

            typedef size_t Index;

        private:

            static const Index NULL_INDEX = Index(-1);

            struct Elem {
                
                bool is_empty;

                Index prev;
                Index next;

                Elem()
                    : is_empty(true)
                    , prev(NULL_INDEX)
                    , next(NULL_INDEX)
                    { }

                };

            Index  empty_head;
            Index filled_head;

            size_t  empty_cnt;
            size_t filled_cnt;

            std::vector<Elem> elem_vec;

            void initialize(size_t n);

        public:

            SlabManager(const SlabManager & other) = default;
            SlabManager(SlabManager && other) = default;

            SlabManager & operator=(const SlabManager & other) = default;
            SlabManager & operator=(SlabManager && other) = default;

            /// <summary> Construct with one slot reserved. </summary>
            ///
            SlabManager();

            /// <summary> Construct with n reserved slots (min 1). </summary>
            ///
            SlabManager(size_t n);

            /// <summary> Acquire a slot (it will be marked as not empty).
            ///        Method returns the slot's index (use it to free() it later). </summary>
            ///
            Index acquire();

            /// <summary> Give a previously acquired element back to the manager for use. </summary>
            ///
            void give_back(Index ind);

            /// <summary> Checks if the slot with the given index is empty. </summary>
            ///
            bool is_slot_empty(Index ind) const;

            /// <summary> Mark all slots as empty. </summary>
            ///
            void clear();

            /// <summary> Returns the current size of the manager. </summary>
            ///
            size_t size() const;

            /// <summary> Returns the current capacity of the underlying vector. </summary>
            ///
            size_t capacity() const;

            /// <summary> Returns the number of empty slots. </summary>
            ///
            size_t empty_count() const;

            /// <summary> Returns the number of filled slots. </summary>
            ///
            size_t filled_count() const;

            /// <summary> Upsize to make more empty slots or downsize to shave off
            ///        excess empty slots. Downsizing is a non-binding request
            ///        and will never destroy non-empty slots. </summary>
            ///
            void resize(size_t newsize);

            /// <summary> Instruct the underlying vector to allocate memory large
            ///        enough to hold at least 'size' (or more) elements. </summary>
            ///
            void reserve(size_t size);

            /// <summary> Trim unused empty slots after the last non-empty slot.
            ///        Up to 4 are allowed to remain. </summary>
            ///
            void resize_to_min();

            /// \brief Deallocate reserved but unused memory. Non-binding request. </summary>
            ///
            void shrink_to_fit();

            // STUB - Iterations

            // DEBUG METHODS:
            /*
            void debug_print() const;

            void debug_lists() const;

            void debug_check_integrity() const;
            */

        };

    // *** Implementation below: *** //

    inline
    SlabManager::SlabManager()
        : elem_vec(1) {
        
        initialize(1);

        }

    inline
    SlabManager::SlabManager(size_t n)
        : elem_vec((n > 0) ? n : 1u) {

        n = ((n > 0) ? n : 1u);

        initialize(n);

        }

    inline
    void SlabManager::initialize(size_t n) {

        empty_head = 0;
        filled_head = NULL_INDEX;

        if (n > 1) {

            elem_vec[0].is_empty = true;
            elem_vec[0].prev = NULL_INDEX;
            elem_vec[0].next = Index(1);

            for (size_t i = 1; i < n - 1; i += 1) {

                elem_vec[i].is_empty = true;

                elem_vec[i].prev = Index(i - 1);
                elem_vec[i].next = Index(i + 1);

                }

            elem_vec[n - 1].is_empty = true;
            elem_vec[n - 1].prev = n - 2;
            elem_vec[n - 1].next = NULL_INDEX;

            }
        else { // Initialize with 1 slot

            elem_vec[0].is_empty = true;
            elem_vec[0].prev = NULL_INDEX;
            elem_vec[0].next = NULL_INDEX;

            empty_head  = 0;
            filled_head = NULL_INDEX;

            }

         empty_cnt = n;
        filled_cnt = 0;

        }

    inline
    SlabManager::Index SlabManager::acquire() {
        
        if (empty_head != NULL_INDEX) {
            
            auto rv = empty_head;

            // "Move" empty_head:
            empty_head = elem_vec[empty_head].next;
            if (empty_head != NULL_INDEX) {
                
                elem_vec[empty_head].prev = NULL_INDEX;

                }

            // Link acquired element with filled ones:
            if (filled_head != NULL_INDEX) {
                
                elem_vec[filled_head].prev = rv;

                }
            elem_vec[rv].next = filled_head;
            elem_vec[rv].prev = NULL_INDEX;
            filled_head = rv;

            elem_vec[rv].is_empty = false;

             empty_cnt -= 1;
            filled_cnt += 1;

            return rv;

            }
        else {
            
            size_t rv = elem_vec.size();

            elem_vec.push_back(Elem());

            // Link acquired element with filled ones:
            if (filled_head != NULL_INDEX) {

                elem_vec[filled_head].prev = rv;

                }
            elem_vec[rv].next = filled_head;
            elem_vec[rv].prev = NULL_INDEX;
            filled_head = rv;

            elem_vec[rv].is_empty = false;

            filled_cnt += 1;

            return rv;

            }

        }

    inline
    void SlabManager::give_back(Index ind) {
        
        if (is_slot_empty(ind)) throw std::logic_error("SlabManager::free - Element not acquired!");

        // Remove from list of filled elements:
        auto prev = elem_vec[ind].prev;
        auto next = elem_vec[ind].next;

        if (next != NULL_INDEX) 
            elem_vec[next].prev = prev;
        else 
            { /* Do nothing */ }

        if (prev != NULL_INDEX) 
            elem_vec[prev].next = next;
        else
            filled_head = next;

        // Link with empty elements:
        if (empty_head != NULL_INDEX) {

            elem_vec[empty_head].prev = ind;

            }
        elem_vec[ind].next = empty_head;
        elem_vec[ind].prev = NULL_INDEX;
        empty_head = ind;

        elem_vec[ind].is_empty = true;

        filled_cnt -= 1;
         empty_cnt += 1;

        }

    inline
    bool SlabManager::is_slot_empty(Index ind) const {

        if (ind >= elem_vec.size()) throw std::out_of_range("SlabManager::is_empty - Index out of bounds!");

        return elem_vec[ind].is_empty;

        }

    inline
    void SlabManager::clear() {
        
        initialize( elem_vec.size() );

        }

    inline
    size_t SlabManager::size() const {

        return elem_vec.size();

        }

    inline
    size_t SlabManager::capacity() const {

        return elem_vec.capacity();

        }

    inline
    size_t SlabManager::empty_count() const {
        
        return empty_cnt;

        }

    inline
    size_t SlabManager::filled_count() const {
        
        return filled_cnt;
        
        }

    inline
    void SlabManager::resize(size_t newsize) {
        
        size_t ss = elem_vec.size();

        if (ss == newsize) return;

        if (newsize > ss) { // Upsize
            
            elem_vec.resize(newsize);

            for (size_t i = ss; i < newsize; i += 1) {
                
                // elem_vec[i].is_empty = true; - Not needed 'cause freshly initialized

                elem_vec[i].prev = NULL_INDEX;
                elem_vec[i].next = empty_head;

                if (empty_head != NULL_INDEX) elem_vec[empty_head].prev = i;

                empty_head = i;

                }

            empty_cnt += (newsize - ss);

            }
        else { // Downsize
            
            size_t pos = NULL_INDEX;
            size_t cnt = 0;

            newsize = ((newsize > 0) ? newsize : 1l);

            for (size_t i = ss - 1; true; i -= 1) {
                
                if (elem_vec[i].is_empty == false) { pos = i; break; }

                cnt += 1;

                // ***
                if (i == 0) break;

                }

            if (pos == NULL_INDEX) {
                
                elem_vec.resize(newsize);

                initialize( elem_vec.size() );

                }
            else {

                if (pos == ss - 1) return;

                if (empty_cnt - cnt < 4u) return;
                
                if (newsize > pos + 1)
                    elem_vec.resize(newsize);
                else
                    elem_vec.resize(pos + 1);

                // Relink empties:
                empty_head = NULL_INDEX;
                empty_cnt  = 0;
                for (size_t i = elem_vec.size() - 1; true; i -= 1) {
                    
                    if (elem_vec[i].is_empty == true) {
                        
                        empty_cnt += 1;

                        if (empty_head == NULL_INDEX) {

                            elem_vec[i].prev = NULL_INDEX;
                            elem_vec[i].next = NULL_INDEX;

                            empty_head = i;

                            }
                        else {

                            elem_vec[empty_head].prev = i;

                            elem_vec[i].prev = NULL_INDEX;
                            elem_vec[i].next = empty_head;

                            empty_head = i;
                            
                            }

                        }

                    // End condition:
                    if (i == 0) break;

                    }

                }

            }
        
        }

    inline
    void SlabManager::reserve(size_t size) {

        elem_vec.reserve(size);

        }

    inline
    void SlabManager::resize_to_min() {

        resize(1u);

        }

    inline
    void SlabManager::shrink_to_fit() {
        
        elem_vec.shrink_to_fit();

        }

    // DEBUG METHODS:
    /*
    inline
    void SlabManager::debug_print() const {
        
        printf("==================================\n");

        for (size_t i = 0; i < elem_vec.size(); i += 1) {
            
            printf("%d. Element is %s.\n", (int)i, (elem_vec[i].is_empty)?("empty"):("in use"));

            }

        printf(" empty_cnt = %d\n", (int) empty_cnt);
        printf("filled_cnt = %d\n", (int)filled_cnt);

        printf("==================================\n");
        
        }

    inline
    void SlabManager::debug_check_integrity() const {
        
        size_t counter;

        counter = 0;

        for (auto i = empty_head; i != NULL_INDEX; i = elem_vec[i].next) {
            
            counter += 1;

            }

        if (counter != empty_cnt) throw std::logic_error("e");

        // ****************** //

        counter = 0;

        for (auto i = filled_head; i != NULL_INDEX; i = elem_vec[i].next) {

            counter += 1;

            }

        if (counter != filled_cnt) throw std::logic_error("f");

        }

    inline
    void SlabManager::debug_lists() const {
        
        printf("Empty elements [head = %zu].\n", empty_head);

        for (auto i = empty_head; i != NULL_INDEX; i = elem_vec[i].next) {

            printf("%d. prev = %zu; next = %zu \n", i, elem_vec[i].prev, elem_vec[i].next);

            }

        // ************************* //

        printf("Filled elements [head = %zu].\n", filled_head);

        for (auto i = filled_head; i != NULL_INDEX; i = elem_vec[i].next) {

            printf("%d. prev = %zu; next = %zu \n", i, elem_vec[i].prev, elem_vec[i].next);

            }

        }
    */

    // *** Implementation End *** //

    }